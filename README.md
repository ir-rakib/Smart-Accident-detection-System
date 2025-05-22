# Smart Accident Detection System

This is a microcontroller-based accident detection and alert system using an **ESP32**, **MPU-6050 accelerometer/gyroscope**, **GPS module**, and **SIM800L GSM module**. When a fall/accident is detected, it captures the GPS coordinates and sends an SMS alert with a Google Maps location link to predefined phone numbers. It also hosts a local web server to show the current status.

---

## ⚙️ Hardware Used

- ESP32 Dev Board
- MPU-6050 (Accelerometer + Gyroscope)
- GPS Module (connected to Serial1)
- SIM800L GSM Module (connected to Serial2)
- WiFi for Web UI
- Optional: Battery, power management circuit

---

## 📦 Features

- Detects sudden falls or accidents using MPU-6050
- Determines orientation changes to confirm an accident
- Captures GPS location on fall detection
- Sends SMS alerts with Google Maps link to specified contacts
- Hosts a local web page to display:
  - Fall detection status
  - Latest location (if available)

---

## 📲 Web Interface

When the ESP32 is connected to WiFi, access the status page via its IP address shown in the Serial Monitor:
## How It Works

- MPU-6050 constantly monitors accelerometer and gyroscope data.
- When a sharp drop and orientation shift is detected, it considers it a fall.
- It reads GPS coordinates and sends them via SMS.
- The ESP32 also runs a web server to show real-time accident and location info.
