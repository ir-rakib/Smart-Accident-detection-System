#include <Servo.h>               // Include Servo library
#include <SoftwareSerial.h>
#include "VoiceRecognitionV3.h"

/**
  Connection:
  Arduino    VoiceRecognitionModule
   2   ------->     RX
   3   ------->     TX
*/
VR myVR(3, 2); // 2:RX, 3:TX

uint8_t onRecords[7] = {0, 2, 4, 6};  // "ON" command records
uint8_t offRecords[7] = {1, 3, 5, 7}; // "OFF" command records
uint8_t buf[64];

Servo myServo; // Servo object

#define INITIAL_POS 70 // Initial position of the servo
#define ON_POS 120      // Position when "ON" command is recognized

void printSignature(uint8_t *buf, int len) {
  for (int i = 0; i < len; i++) {
    if (buf[i] > 0x19 && buf[i] < 0x7F) {
      Serial.write(buf[i]);
    } else {
      Serial.print("[");
      Serial.print(buf[i], HEX);
      Serial.print("]");
    }
  }
}

void printVR(uint8_t *buf) {
  Serial.println("VR Index\tGroup\tRecordNum\tSignature");
  Serial.print(buf[2], DEC);
  Serial.print("\t\t");

  if (buf[0] == 0xFF) {
    Serial.print("NONE");
  } else if (buf[0] & 0x80) {
    Serial.print("UG ");
    Serial.print(buf[0] & (~0x80), DEC);
  } else {
    Serial.print("SG ");
    Serial.print(buf[0], DEC);
  }
  Serial.print("\t");

  Serial.print(buf[1], DEC);
  Serial.print("\t\t");
  if (buf[3] > 0) {
    printSignature(buf + 4, buf[3]);
  } else {
    Serial.print("NONE");
  }
  Serial.println("\r\n");
}

void setup() {
  /** Initialize */
  myVR.begin(9600);
  Serial.begin(115200);
  Serial.println("Elechouse Voice Recognition V3 Module\r\nControl Servo sample");

  myServo.attach(9);   // Attach the servo to pin 9
  myServo.write(INITIAL_POS); // Set initial position to 70°

  if (myVR.clear() == 0) {
    Serial.println("Recognizer cleared.");
  } else {
    Serial.println("VoiceRecognitionModule not found. Check connection.");
    while (1);
  }

  // Load "ON" records
  for (int i = 0; i < 7; i++) {
    if (myVR.load(onRecords[i]) >= 0) {
      Serial.print("ON record ");
      Serial.print(onRecords[i]);
      Serial.println(" loaded");
    }
  }

  // Load "OFF" records
  for (int i = 0; i < 7; i++) {
    if (myVR.load(offRecords[i]) >= 0) {
      Serial.print("OFF record ");
      Serial.print(offRecords[i]);
      Serial.println(" loaded");
    }
  }
}

void loop() {
  int ret = myVR.recognize(buf, 50);
  if (ret > 0) {
    // Handle recognized voice commands
    for (int i = 0; i < 7; i++) {
      if (buf[1] == onRecords[i]) {
        /** Rotate servo to 90° */
        myServo.write(ON_POS);
        Serial.println("Servo moved to 90 degrees (ON command)");
        printVR(buf);
        return;
      }
      if (buf[1] == offRecords[i]) {
        /** Rotate servo back to initial position (70°) */
        myServo.write(INITIAL_POS);
        Serial.println("Servo moved to initial position (70 degrees) (OFF command)");
        printVR(buf);
        return;
      }
    }
    Serial.println("Command not recognized.");
  }
}
