#include <SPI.h>
#include <MFRC522.h>

#define DOOR_SS_PIN   5
#define DOOR_RST_PIN  4
#define DOOR_LED_G    2
#define DOOR_LED_R    15

#define SEAT_SS_PIN   32
#define SEAT_RST_PIN  33
#define SEAT_LED_G    25
#define SEAT_LED_R    26
#define SEAT_FSR      35  

#define BUZZER        16

MFRC522 doorRFID(DOOR_SS_PIN, DOOR_RST_PIN);
MFRC522 seatRFID(SEAT_SS_PIN, SEAT_RST_PIN);

struct Passenger {
  String cardUID;
  String name;
  int assignedSeat;
  int status;        
  unsigned long lastSeen;
  bool isBanned;
};

Passenger passengers[] = {
  {"01 02 03 04", "Ahmed",    1, 0, 0, false},
  {"11 22 33 44", "Sarah",    2, 0, 0, false},
  {"55 66 77 88", "Mohammed", 3, 0, 0, false},
};
int numPassengers = 3;

#define STATUS_OUTSIDE   0
#define STATUS_ON_BOARD  1
#define STATUS_EXITED    2
#define STATUS_SEATED    3  

int passengersOnBoard = 0;

bool seatOccupied = false;        
bool seatValidated = false;       
String seatedPassengerUID = "";   
bool lastSwitchState = LOW;       

void setup() {
  Serial.begin(115200);
  SPI.begin();
  
  pinMode(DOOR_LED_G, OUTPUT);
  pinMode(DOOR_LED_R, OUTPUT);
  doorRFID.PCD_Init();
  
  pinMode(SEAT_LED_G, OUTPUT);
  pinMode(SEAT_LED_R, OUTPUT);
  pinMode(SEAT_FSR, INPUT);
  seatRFID.PCD_Init();
  
  pinMode(BUZZER, OUTPUT);
  
  resetAllOutputs();
  
  Serial.println("╔════════════════════════════════════════════════╗");
  Serial.println("║   VEHICLE ACCESS CONTROL (Final Version)       ║");
  Serial.println("╠════════════════════════════════════════════════╣");
  Serial.println("║  ✅ Seat Memory - No re-scan while seated      ║");
  Serial.println("║  ✅ Card Lock - Can't use card at door seated  ║");
  Serial.println("╚════════════════════════════════════════════════╝");
  Serial.println("");
  printDatabase();
}

void loop() {
  checkDoor();      
  checkSeat();      
  checkTimeouts();  
  
  delay(100);
}

void checkDoor() {
  if (!doorRFID.PICC_IsNewCardPresent() || !doorRFID.PICC_ReadCardSerial()) {
    return;
  }
  
  String uid = getUID(doorRFID);
  int idx = findPassenger(uid);
  
  Serial.println("🚪 [DOOR] Card Scanned: " + uid);
  
  if (idx == -1) {
    Serial.println("   ❌ Unknown Card - Access Denied");
    doorAlert();
  } 
  else if (passengers[idx].isBanned) {
    Serial.println("   🚫 Card BANNED - Access Denied");
    doorAlert();
  }
  else if (passengers[idx].status == STATUS_SEATED) {
    
    Serial.println("   ⚠️  Passenger is SEATED - Card locked at seat!");
    Serial.println("   🚫 Cannot exit while seated (stand up first)");
    doorAlert();
  }
  else if (passengers[idx].status == STATUS_ON_BOARD) {
    
    passengers[idx].status = STATUS_EXITED;
    passengersOnBoard--;
    Serial.println("   ✅ Exit Confirmed - Safe travels!");
    Serial.print("   📊 Passengers on board: ");
    Serial.println(passengersOnBoard);
    doorSuccess();
  }
  else if (passengers[idx].status == STATUS_OUTSIDE || passengers[idx].status == STATUS_EXITED) {
    // Entry
    passengers[idx].status = STATUS_ON_BOARD;
    passengers[idx].lastSeen = millis();
    passengersOnBoard++;
    Serial.println("   ✅ Entry Granted - Welcome aboard!");
    Serial.print("   🪑 Assigned Seat: ");
    Serial.println(passengers[idx].assignedSeat);
    Serial.print("   📊 Passengers on board: ");
    Serial.println(passengersOnBoard);
    doorSuccess();
  }
  
  Serial.println("");
  doorRFID.PICC_HaltA();
  doorRFID.PCD_StopCrypto1();
  delay(1000);
  resetAllOutputs();
}

void checkSeat() {
  
  bool currentSwitchState = digitalRead(SEAT_FSR);
  
  if (currentSwitchState == HIGH && lastSwitchState == LOW) {
    Serial.println("💺 [SEAT] Person sat down (Switch OFF→ON)");
    seatOccupied = true;
    
    requireCardScan();
  }
  
  else if (currentSwitchState == LOW && lastSwitchState == HIGH) {
    Serial.println("💺 [SEAT] Person stood up (Switch ON→OFF)");
    seatOccupied = false;
    
    if (seatValidated && seatedPassengerUID != "") {
      int idx = findPassenger(seatedPassengerUID);
      if (idx != -1) {
        passengers[idx].status = STATUS_ON_BOARD;
        Serial.println("   Passenger " + passengers[idx].name + " stood up");
      }
    }
    
    seatValidated = false;
    seatedPassengerUID = "";
    resetSeatOutputs();
  }
  
  else if (currentSwitchState == HIGH && seatValidated) {
    digitalWrite(SEAT_LED_G, HIGH);
    digitalWrite(SEAT_LED_R, LOW);
  }
  
  lastSwitchState = currentSwitchState;
}

void requireCardScan() {
  unsigned long startTime = millis();
  bool cardScanned = false;
  
  while (millis() - startTime < 3000) {
    if (seatRFID.PICC_IsNewCardPresent() && seatRFID.PICC_ReadCardSerial()) {
      cardScanned = true;
      break;
    }
    delay(50);
  }
  
  if (!cardScanned) {
    Serial.println("   ⚠️  No card scanned within 3 seconds!");
    Serial.println("   🚫 Unauthorized Sitting");
    seatAlert();
    delay(2000);
    resetSeatOutputs();
    return;
  }
  
  String uid = getUID(seatRFID);
  int idx = findPassenger(uid);
  
  Serial.println("💺 [SEAT] Card Scanned: " + uid);
  
  if (idx == -1) {
    Serial.println("   ❌ Unknown Card at Seat");
    seatAlert();
  }
  else if (passengers[idx].status != STATUS_ON_BOARD) {
    Serial.println("   ⚠️  Card not checked in at Door!");
    seatAlert();
  }
  else if (passengers[idx].assignedSeat != 1) {
    Serial.println("   ⚠️  WRONG SEAT! Assigned to Seat " + String(passengers[idx].assignedSeat));
    seatAlert();
  }
  else {
    Serial.println("   ✅ Correct Seat - Enjoy your ride!");
    passengers[idx].status = STATUS_SEATED;  
    passengers[idx].lastSeen = millis();
    seatValidated = true;
    seatedPassengerUID = uid;
    seatSuccess();
  }
  
  Serial.println("");
  seatRFID.PICC_HaltA();
  seatRFID.PCD_StopCrypto1();
}

void checkTimeouts() {
}


int findPassenger(String uid) {
  for (int i = 0; i < numPassengers; i++) {
    if (passengers[i].cardUID == uid) return i;
  }
  return -1;
}

String getUID(MFRC522 &rfid) {
  String content = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) content += "0";
    content += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) content += " ";
  }
  content.toUpperCase();
  return content;
}

void printDatabase() {
  Serial.println("📋 Passenger Database:");
  for (int i = 0; i < numPassengers; i++) {
    Serial.print("  - ");
    Serial.print(passengers[i].name);
    Serial.print(" (Seat ");
    Serial.print(passengers[i].assignedSeat);
    Serial.print("): ");
    Serial.println(passengers[i].cardUID);
  }
  Serial.println("");
}


void doorSuccess() {
  digitalWrite(DOOR_LED_G, HIGH);
  digitalWrite(DOOR_LED_R, LOW);
  noTone(BUZZER);
}

void doorAlert() {
  digitalWrite(DOOR_LED_G, LOW);
  digitalWrite(DOOR_LED_R, HIGH);
  tone(BUZZER, 1000, 500);
}

void seatSuccess() {
  digitalWrite(SEAT_LED_G, HIGH);
  digitalWrite(SEAT_LED_R, LOW);
  noTone(BUZZER);
}

void seatAlert() {
  digitalWrite(SEAT_LED_G, LOW);
  digitalWrite(SEAT_LED_R, HIGH);
  tone(BUZZER, 1000, 1000);
}

void resetAllOutputs() {
  digitalWrite(DOOR_LED_G, LOW);
  digitalWrite(DOOR_LED_R, LOW);
  
  if (!seatValidated) {
    digitalWrite(SEAT_LED_G, LOW);
    digitalWrite(SEAT_LED_R, LOW);
  }
  noTone(BUZZER);
}

void resetSeatOutputs() {
  digitalWrite(SEAT_LED_G, LOW);
  digitalWrite(SEAT_LED_R, LOW);
  noTone(BUZZER);
}
