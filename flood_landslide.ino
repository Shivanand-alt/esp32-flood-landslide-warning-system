#include <WiFi.h>
#include <WebServer.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

// ================= WIFI SETTINGS =================
const char* ssid = "realme narzo 30 5G";
const char* password = "12345678";

// ================= SMS PHONE NUMBER ==============
const char* PHONE_NUMBER = "+919740999707";

// ================= PIN DEFINITIONS ===============
#define TRIG_PIN 5
#define ECHO_PIN 18

#define RAIN_DO 19
#define RAIN_AO 34

#define SOIL_PIN_1 32
#define SOIL_PIN_2 33
#define SOIL_PIN_3 35
#define SOIL_PIN_4 36

#define GPS_RX 16
#define GPS_TX 17

#define LED_PIN 2
#define BUZZER_PIN 25
#define LANDSLIDE_BUZZER 14

#define SIM800_RX 26
#define SIM800_TX 27

// ================= OBJECTS =======================
WebServer server(80);
TinyGPSPlus gps;
HardwareSerial GPS_Serial(2);
HardwareSerial SIM800_Serial(1);

// ================= WIFI VARIABLES ================
bool webServerStarted = false;
unsigned long lastWiFiAttempt = 0;
const unsigned long WIFI_RETRY_INTERVAL = 10000;

// ================= FLOOD LEVELS ==================
const float WARNING_DISTANCE = 20.0;
const float CRITICAL_DISTANCE = 10.0;

// ================= BUZZER SETTINGS ================
const unsigned long WARNING_BEEP_INTERVAL = 1500;
const unsigned long CRITICAL_BEEP_INTERVAL = 500;
const unsigned long BEEP_DURATION = 200;

unsigned long lastBeepTime = 0;
bool buzzerState = false;

// ================= SENSOR VARIABLES ===============
float distanceCM = -1;

int rainDigital = HIGH;
int rainAnalog = 0;

int soilVal1 = 0;
int soilVal2 = 0;
int soilVal3 = 0;
int soilVal4 = 0;

int soilRaw[4] = {0, 0, 0, 0};
int soilMoisture[4] = {0, 0, 0, 0};

// Replace with readings from YOUR sensors
// Dry soil readings
int soilDry[4] = {3200, 3200, 3200, 3200};

// Wet soil readings
int soilWet[4] = {1400, 1400, 1400, 1400};

// Mean moisture across all four sensors
float meanSoilMoisture = 0.0;

// Experimental threshold; calibrate for your setup
const int SOIL_WET_THRESHOLD = 70;
bool soilWarning = false;

bool waterDetected = false;
bool warning = false;
bool critical = false;

// ================= GPS VARIABLES ==================
double latitude = 0;
double longitude = 0;
bool gpsFix = false;

// ================= SMS VARIABLES ==================
bool warningSMSsent = false;
bool criticalSMSsent = false;

// ================= WIFI MANAGEMENT ================
void maintainWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!webServerStarted) {
      server.begin();
      webServerStarted = true;

      Serial.println("WiFi connected");
      Serial.print("Dashboard: http://");
      Serial.println(WiFi.localIP());
    }
  } else {
    if (webServerStarted) {
      Serial.println("WiFi disconnected");
      webServerStarted = false;
    }

    if (millis() - lastWiFiAttempt >= WIFI_RETRY_INTERVAL) {
      lastWiFiAttempt = millis();
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
  }
}

// ================= READ WATER DISTANCE =============
float readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) return -1;

  return duration * 0.0343 / 2.0;
}

// ================= READ GPS ========================
void readGPS() {
  while (GPS_Serial.available()) {
    gps.encode(GPS_Serial.read());
  }

  if (gps.location.isValid() && gps.location.age() < 5000) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    gpsFix = true;
  } else {
    gpsFix = false;
  }
}

// ================= MOISTURE CALCULATION ===========
int calculateMoisture(int raw, int dry, int wet) {
  if (dry == wet) return 0;

  int percent = (raw - dry) * 100 / (wet - dry);

  return constrain(percent, 0, 100);
}

// ================= READ ALL SENSORS ===============
void readSensors() {
  distanceCM = readDistance();

  rainDigital = digitalRead(RAIN_DO);
  rainAnalog = analogRead(RAIN_AO);

  soilRaw[0] = analogRead(SOIL_PIN_1);
  soilRaw[1] = analogRead(SOIL_PIN_2);
  soilRaw[2] = analogRead(SOIL_PIN_3);
  soilRaw[3] = analogRead(SOIL_PIN_4);

  soilVal1 = soilRaw[0];
  soilVal2 = soilRaw[1];
  soilVal3 = soilRaw[2];
  soilVal4 = soilRaw[3];

  // Calculate each sensor's approximate moisture percentage
  int totalMoisture = 0;

  for (int i = 0; i < 4; i++) {
    soilMoisture[i] = calculateMoisture(
      soilRaw[i], soilDry[i], soilWet[i]
    );

    totalMoisture += soilMoisture[i];
  }

  // MEAN = (S1 + S2 + S3 + S4) / 4
  meanSoilMoisture = totalMoisture / 4.0;

  // Soil warning uses the MEAN, not individual sensors
  soilWarning = (meanSoilMoisture >= SOIL_WET_THRESHOLD);

  // Rain sensor digital output is usually LOW when triggered
  waterDetected = (rainDigital == LOW);

  warning = false;
  critical = false;

  if (distanceCM > 0 && distanceCM <= CRITICAL_DISTANCE) {
    critical = true;
    warning = true;
  } else if (distanceCM > 0 &&
             distanceCM <= WARNING_DISTANCE) {
    warning = true;
  }

  if (waterDetected) {
    warning = true;
  }
}

// ================= SEND SMS ========================
void sendSMS(String message) {
  Serial.println("Sending SMS...");

  SIM800_Serial.println("AT+CMGF=1");
  delay(500);

  SIM800_Serial.print("AT+CMGS=\"");
  SIM800_Serial.print(PHONE_NUMBER);
  SIM800_Serial.println("\"");

  unsigned long start = millis();
  bool promptReceived = false;

  while (millis() - start < 5000) {
    if (SIM800_Serial.available()) {
      char c = SIM800_Serial.read();
      Serial.write(c);

      if (c == '>') {
        promptReceived = true;
        break;
      }
    }
    delay(10);
  }

  if (!promptReceived) {
    Serial.println("SMS prompt not received");
    return;
  }

  SIM800_Serial.print(message);
  SIM800_Serial.write(26);

  start = millis();

  while (millis() - start < 15000) {
    if (SIM800_Serial.available()) {
      String response = SIM800_Serial.readString();
      Serial.println(response);

      if (response.indexOf("+CMGS:") >= 0) {
        Serial.println("SMS sent successfully");
        return;
      }

      if (response.indexOf("ERROR") >= 0) {
        Serial.println("SMS failed");
        return;
      }
    }
    delay(10);
  }

  Serial.println("SMS confirmation timed out");
}

// ================= WARNING SMS =====================
void sendWarningSMS() {
  String message = "FLOOD WARNING!\n";

  message += "Water distance: ";
  message += String(distanceCM, 1);
  message += " cm\n";

  message += "Rain/water: ";
  message += waterDetected ? "DETECTED\n" : "NOT DETECTED\n";

  message += "Mean soil moisture: ";
  message += String(meanSoilMoisture, 1);
  message += "%\n";

  if (gpsFix) {
    message += "Latitude: " + String(latitude, 6);
    message += "\nLongitude: " + String(longitude, 6);
  } else {
    message += "GPS: NO FIX";
  }

  sendSMS(message);
}

// ================= CRITICAL SMS ====================
void sendCriticalSMS() {
  String message = "CRITICAL FLOOD ALERT!\n";

  message += "Water distance: ";
  message += String(distanceCM, 1);
  message += " cm\n";

  message += "Immediate attention required.\n";

  if (gpsFix) {
    message += "Latitude: " + String(latitude, 6);
    message += "\nLongitude: " + String(longitude, 6);
  } else {
    message += "GPS: NO FIX";
  }

  sendSMS(message);
}

// ================= FLOOD ALARM =====================
void updateAlarm() {
  unsigned long now = millis();

  if (!warning) {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    buzzerState = false;
    return;
  }

  digitalWrite(LED_PIN, HIGH);

  unsigned long interval =
      critical ? CRITICAL_BEEP_INTERVAL
               : WARNING_BEEP_INTERVAL;

  if (!buzzerState && now - lastBeepTime >= interval) {
    digitalWrite(BUZZER_PIN, HIGH);
    buzzerState = true;
    lastBeepTime = now;
  }

  if (buzzerState && now - lastBeepTime >= BEEP_DURATION) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerState = false;
  }
}

// ================= SOIL WARNING ALARM ==============
void updateSoilAlarm() {
  digitalWrite(
    LANDSLIDE_BUZZER,
    soilWarning ? HIGH : LOW
  );
}

// ================= WEB DASHBOARD ===================
void handleRoot() {
  String page = "";

  page += "<!DOCTYPE html><html><head>";
  page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  page += "<meta http-equiv='refresh' content='2'>";
  page += "<title>Flood and Landslide Monitor</title>";
  page += "<style>";
  page += "body{font-family:Arial;text-align:center;background:#eef2f7;padding:15px;}";
  page += ".box{background:white;padding:20px;margin:auto;max-width:550px;border-radius:12px;}";
  page += ".normal{color:green}.warning{color:orange}.critical{color:red}";
  page += "</style></head><body><div class='box'>";

  page += "<h1>Flood and Landslide Monitor</h1>";

  if (critical) {
    page += "<h2 class='critical'>CRITICAL FLOOD LEVEL</h2>";
  } else if (warning) {
    page += "<h2 class='warning'>FLOOD WARNING</h2>";
  } else {
    page += "<h2 class='normal'>NORMAL</h2>";
  }

  page += "<p><b>Water distance:</b> ";
  page += distanceCM > 0 ? String(distanceCM, 1) + " cm"
                         : "No reading";
  page += "</p>";

  page += "<p><b>Rain/water:</b> ";
  page += waterDetected ? "DETECTED" : "NOT DETECTED";
  page += "</p>";

  page += "<p><b>Rain analog:</b> ";
  page += String(rainAnalog);
  page += "</p><hr><h3>Soil Moisture</h3>";

  for (int i = 0; i < 4; i++) {
    page += "<p>Sensor ";
    page += String(i + 1);
    page += ": ";
    page += String(soilMoisture[i]);
    page += "% (raw ";
    page += String(soilRaw[i]);
    page += ")</p>";
  }

  page += "<h2>Mean Soil Moisture: ";
  page += String(meanSoilMoisture, 1);
  page += "%</h2>";

  page += "<p><b>Soil moisture status:</b> ";
  page += soilWarning ? "HIGH MOISTURE WARNING"
                      : "NORMAL";
  page += "</p>";

  page += "<p><b>GPS:</b> ";
  page += gpsFix ? "FIXED" : "WAITING FOR SATELLITE";
  page += "</p>";

  if (gpsFix) {
    page += "<p>Latitude: " + String(latitude, 6) + "</p>";
    page += "<p>Longitude: " + String(longitude, 6) + "</p>";

    page += "<p><a href='https://maps.google.com/?q=";
    page += String(latitude, 6) + "," + String(longitude, 6);
    page += "'>View Location on Map</a></p>";
  }

  page += "<p>Wi-Fi: CONNECTED</p>";
  page += "<p>GSM Module: SIM800L</p>";
  page += "</div></body></html>";

  server.send(200, "text/html", page);
}

// ================= SETUP ===========================
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  pinMode(RAIN_DO, INPUT);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LANDSLIDE_BUZZER, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LANDSLIDE_BUZZER, LOW);

  analogReadResolution(12);

  GPS_Serial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  SIM800_Serial.begin(9600, SERIAL_8N1, SIM800_RX, SIM800_TX);

  server.on("/", handleRoot);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi");

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    maintainWiFi();
  } else {
    Serial.println("Wi-Fi not connected; retrying automatically.");
  }

  Serial.println("Checking SIM800L...");
  SIM800_Serial.println("AT");
  delay(500);

  while (SIM800_Serial.available()) {
    Serial.write(SIM800_Serial.read());
  }

  SIM800_Serial.println("AT+CMGF=1");
  delay(500);

  while (SIM800_Serial.available()) {
    Serial.write(SIM800_Serial.read());
  }

  Serial.println("\nSystem ready");
}

// ================= MAIN LOOP ======================
void loop() {
  static unsigned long lastIPPrint = 0;

  maintainWiFi();

  if (WiFi.status() == WL_CONNECTED && webServerStarted) {
    server.handleClient();
  }

  if (WiFi.status() == WL_CONNECTED &&
      millis() - lastIPPrint >= 5000) {
    lastIPPrint = millis();
    Serial.print("Dashboard: http://");
    Serial.println(WiFi.localIP());
  }

  readGPS();
  readSensors();

  updateAlarm();
  updateSoilAlarm();

  if (critical && !criticalSMSsent) {
    sendCriticalSMS();
    criticalSMSsent = true;
    warningSMSsent = true;
  } else if (warning && !warningSMSsent) {
    sendWarningSMS();
    warningSMSsent = true;
  }

  if (!warning) {
    warningSMSsent = false;
    criticalSMSsent = false;
  }

  Serial.print("Water distance: ");
  Serial.print(distanceCM);
  Serial.print(" cm | Rain: ");
  Serial.print(waterDetected ? "YES" : "NO");

  Serial.print(" | Soil raw: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(soilRaw[i]);
    if (i < 3) Serial.print(", ");
  }

  Serial.print(" | Moisture %: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(soilMoisture[i]);
    Serial.print("% ");
  }

  Serial.print("| MEAN: ");
  Serial.print(meanSoilMoisture, 1);
  Serial.print("% | Soil warning: ");
  Serial.print(soilWarning ? "HIGH" : "NORMAL");

  Serial.print(" | GPS: ");
  Serial.print(gpsFix ? "FIXED" : "WAITING");

  Serial.print(" | Wi-Fi: ");
  Serial.println(
    WiFi.status() == WL_CONNECTED ? "CONNECTED"
                                  : "DISCONNECTED"
  );

  delay(500);
}
