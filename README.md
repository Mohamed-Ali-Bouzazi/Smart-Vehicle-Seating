# 🚗 Smart Vehicle Seat Authentication System

[![Status](https://img.shields.io/badge/status-complete-success)]()
[![License](https://img.shields.io/badge/license-MIT-blue)]()

## 📋 Project Overview
This project implements an embedded system for vehicle passenger management. It solves three key risks:
1. **Missing Passengers:** Detects presence via pressure sensors.
2. **Overloading:** Counts passengers vs. vehicle capacity.
3. **Placement Conflicts:** Authenticates identity via RFID to ensure authorized seating.

## 🎥 Demo
Try the simulation live on Wokwi:
[🔗 Open Wokwi Simulation](https://wokwi.com/projects/459300331729644545)

## 🛠️ Hardware Requirements
| Component | Quantity | Notes |
| :--- | :---: | :--- |
| ESP32 DevKit V1 | 1 | Main Controller |
| MFRC522 RFID | 1 | 13.56MHz Reader |
| FSR-402 Sensor | 1 | Pressure Sensor (Simulated by Potentiometer) |
| LED Green/Red | 2 | Status Indicators |
| Active Buzzer | 1 | Audio Alert |

## ⚡ Features
- ✅ **Real-time Authentication:** <100ms response time.
- ✅ **Visual & Audio Feedback:** LEDs and Buzzer alerts.
- ✅ **Low Power:** Deep sleep mode when seat is empty.
- ✅ **Cost Effective:** Total BOM cost ~$21.10 per seat.

## 📂 Repository Structure
- `/firmware`: Arduino source code for ESP32.
- `/docs`: Technical reports (Analysis, Implementation, Tests).
- `/schemas`: UML diagrams and circuit schematics.

## 🚀 Installation
1. Install [Arduino IDE](https://www.arduino.cc/en/software).
2. Add ESP32 Board Manager URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Install Library: `MFRC522` by miguelbalboa.
4. Upload `firmware/sketch.ino` to ESP32.

## 📊 Test Results
| Test Scenario | Expected Result | Status |
| :--- | :--- | :---: |
| Authorized Card | Green LED | ✅ Pass |
| Unauthorized Card | Red LED + Buzzer | ✅ Pass |
| No Card | Red LED + Alert | ✅ Pass |
| Empty Seat | System Sleep | ✅ Pass |

## 👥 Team
- Mohamed Ali

## 📄 License
This project is licensed under the MIT License.
