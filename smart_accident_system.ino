  #include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

// MPU-6050 and fall detection variables
const int MPU_addr = 0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
float ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0;
boolean fall = false;
boolean trigger1 = false, trigger2 = false, trigger3 = false;
byte trigger1count = 0, trigger2count = 0, trigger3count = 0;
int angleChange = 0;

// WiFi credentials
const char *ssid = "Homies";
const char *pass = "asdfghjkl";

// GPS and SIM800L variables
WebServer server(80);         // Web server on port 80
TinyGPSPlus gps;              // GPS instance
HardwareSerial GPSModule(1);  // Use Serial1 on ESP32 for GPS
HardwareSerial sim800l(2);    // Use Serial2 for SIM800L (adjust pins as per setup)

// GPS location variables
float latitude = 0.0, longitude = 0.0;
bool locationCaptured = false;

void setup() {
  Serial.begin(115200);
  
  // Initialize MPU-6050
  Wire.begin(22, 21);
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0);    // Wake up MPU-6050
  Wire.endTransmission(true);

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // GPS Module setup
  GPSModule.begin(9600, SERIAL_8N1, 16, 17);  // RX=16, TX=17
  
  // SIM800L setup
  // SIM800L setup
  sim800l.begin(9600, SERIAL_8N1, 4, 2); // TX = 4, RX = 2
  sendCommand("AT");          // Initialize SIM800L
  sendCommand("AT+CMGF=1");   // Set SMS to text mode

  // Start the web server
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  mpu_read();
  calculateValues();
  fallDetection();
  readGPSData();
  server.handleClient();
  delay(100);
}

// Send AT command to SIM800L
void sendCommand(const char* command) {
  sim800l.println(command);
  delay(500);
}

// Send SMS with accident details
void sendSMS(const char* phoneNumber, const char* message) {
  sim800l.print("AT+CMGS=\"");
  sim800l.print(phoneNumber);
  sim800l.println("\"");
  delay(500); // Wait for > response
  sim800l.print(message);
  sim800l.write(26); // ASCII CTRL+Z
  delay(3000); // Allow time for SMS to send
}

// Read MPU-6050 data
void mpu_read() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true);
  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();
}

// Calculate accelerometer and gyroscope values
void calculateValues() {
  ax = (AcX - 2050) / 16384.00;
  ay = (AcY - 77) / 16384.00;
  az = (AcZ - 1947) / 16384.00;
  gx = (GyX + 270) / 131.07;
  gy = (GyY - 351) / 131.07;
  gz = (GyZ + 136) / 131.07;
}

// Fall detection logic
void fallDetection() {
  float Raw_Amp = sqrt(ax * ax + ay * ay + az * az);
  int Amp = Raw_Amp * 10; // Scale acceleration to integer

  if (Amp <= 3 && !trigger2) {
    trigger1 = true;
  }
  if (trigger1 && Amp >= 7) {
    trigger2 = true;
    trigger1 = false;
  }
  if (trigger2) {
    angleChange = sqrt(gx * gx + gy * gy + gz * gz);
    if (angleChange >= 15 && angleChange <= 250) {
      trigger3 = true;
      trigger2 = false;
    }
  }
  if (trigger3) {
    angleChange = sqrt(gx * gx + gy * gy + gz * gz);
    if (angleChange >= 10 && angleChange <= 300) {
      fall = true;
      trigger3 = false;
      Serial.println("Fall detected!"); // Print fall detection message
      Serial.print("Angle Change: ");
      Serial.println(angleChange);
      sendAccidentDetails();
    }
  }
}

// Send accident details
void sendAccidentDetails() {
  locationCaptured = captureGPSLocation();
  char message[160];
  if (locationCaptured) {
    snprintf(message, sizeof(message), 
             "Accident Detected! Location: https://maps.google.com/?q=%.6f,%.6f",
             latitude, longitude);
  } else {
    snprintf(message, sizeof(message), "Accident Detected! Location unavailable.");
  }
  sendSMS("+8801885125093", message);
  sendSMS("+8801521300294", message);
}

// Handle root page for web server
void handleRoot() {
  String html = "<html><body>";
  html += "<h1>Accident Detection Status</h1>";
  html += "<p>Accident Detected: " + String(fall ? "Yes" : "No") + "</p>";
  if (locationCaptured) {
    html += "<p>Location (Lat, Lon): " + String(latitude, 6) + ", " + String(longitude, 6) + "</p>";
  } else {
    html += "<p>Location: Not available</p>";
  }
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// Read GPS data continuously
void readGPSData() {
  while (GPSModule.available() > 0) {
    gps.encode(GPSModule.read());
  }
}

// Capture GPS location when a fall is detected
bool captureGPSLocation() {
  if (gps.location.isValid()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    return true;
  }
  return false;
}

