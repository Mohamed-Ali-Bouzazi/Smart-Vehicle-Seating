#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         4
#define SS_PIN          5

#define FSR_PIN         34

#define GREEN_LED       2
#define RED_LED         15    
#define BUZZER          16    

MFRC522 rfid(SS_PIN, RST_PIN);

String authorizedUIDs[] = {
  "01 02 03 04",
};
int numAuthorizedCards = 1;

bool seatOccupied = false;
bool cardProcessed = false;

void setup() {
  Serial.begin(115200);
  SPI.begin();
  
  pinMode(FSR_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  noTone(BUZZER);
  
  rfid.PCD_Init();
  
  Serial.println("=== System Ready ===");
}

void loop() {
  int pressureValue = analogRead(FSR_PIN);
  int pressurePercent = map(pressureValue, 0, 4095, 0, 100);
  
  if (pressurePercent > 30) {
    if (!seatOccupied) {
      Serial.println("\n--- Passenger Detected ---");
      seatOccupied = true;
      cardProcessed = false;
    }
    
    if (!cardProcessed) {
      processRFID();
      cardProcessed = true;
    }
  } else {
    if (seatOccupied) {
      Serial.println("\n--- Passenger Left ---");
      seatOccupied = false;
      cardProcessed = false;
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, LOW);
      noTone(BUZZER);
    }
  }
  delay(100);
}

void processRFID() {
  Serial.println("Scanning RFID...");
  
  if (!rfid.PICC_IsNewCardPresent()) {
    Serial.println(" No card detected");
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);  
    tone(BUZZER, 1000, 500);
    return;
  }
  
  if (!rfid.PICC_ReadCardSerial()) {
    Serial.println(" Failed to read card");
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);  
    tone(BUZZER, 1000, 500);
    return;
  }
  
  String uid = getUID();
  Serial.print(" Card UID: ");
  Serial.println(uid);
  
  if (isAuthorized(uid)) {
    Serial.println("AUTHORIZED - Access Granted");
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);   
    noTone(BUZZER);
  } else {
    Serial.println(" UNAUTHORIZED - Access Denied");
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW); 
    tone(BUZZER, 1000, 1000);
  }
  
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  
  delay(2000);
}

String getUID() {
  String content = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      content += "0";
    }
    content += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) {
      content += " ";
    }
  }
  content.toUpperCase();
  return content;
}

bool isAuthorized(String uid) {
  for (int i = 0; i < numAuthorizedCards; i++) {
    if (uid == authorizedUIDs[i]) {
      return true;
    }
  }
  return false;
}
