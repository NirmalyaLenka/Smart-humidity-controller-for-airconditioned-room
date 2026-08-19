/*
 * Smart Humidity Controller for an air-conditioned room
 *
 * Reads room relative humidity from an SHT31 sensor and switches a
 * relay-driven ultrasonic humidifier to keep RH inside a healthy
 * band (default 40-50%). Includes a web dashboard for live readings,
 * setpoint changes, and an optional water level cut-off so the mist
 * maker never runs dry.
 *
 * Hardware: ESP8266 NodeMCU, SHT31 (I2C), 1-channel relay module,
 * ultrasonic mist maker as the humidifying element, optional SSD1306
 * OLED, optional float switch water level sensor.
 *
 * Libraries required (install via Arduino Library Manager):
 *   - Adafruit SHT31 Library
 *   - Adafruit GFX Library        (only if HAS_OLED is 1)
 *   - Adafruit SSD1306            (only if HAS_OLED is 1)
 *   - ESP8266WiFi / ESP8266WebServer (bundled with ESP8266 core)
 */

#include <Wire.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Adafruit_SHT31.h"
#include "config.h"

#if HAS_OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(128, 64, &Wire, -1);
#endif

Adafruit_SHT31 sht31 = Adafruit_SHT31();
ESP8266WebServer server(WEB_SERVER_PORT);

float currentHumidity = NAN;
float currentTemperature = NAN;
float setpointLow = DEFAULT_SETPOINT_LOW;
float setpointHigh = DEFAULT_SETPOINT_HIGH;

bool relayState = false;         // true = humidifier ON
bool waterLow = false;           // true = reservoir needs refilling
bool sensorOk = false;

unsigned long lastSensorRead = 0;
unsigned long lastRelaySwitch = 0;

// ---------------------------------------------------------------
// Relay helpers
// ---------------------------------------------------------------
void setRelay(bool on) {
  relayState = on;
  bool pinLevel = RELAY_ACTIVE_LOW ? !on : on;
  digitalWrite(PIN_RELAY, pinLevel);
}

// ---------------------------------------------------------------
// EEPROM persistence for setpoints
// ---------------------------------------------------------------
void loadSetpoints() {
  EEPROM.begin(EEPROM_SIZE);
  uint8_t magic = EEPROM.read(EEPROM_MAGIC_ADDR);
  if (magic == EEPROM_MAGIC_VAL) {
    EEPROM.get(EEPROM_ADDR_LOW, setpointLow);
    EEPROM.get(EEPROM_ADDR_HIGH, setpointHigh);
  } else {
    setpointLow = DEFAULT_SETPOINT_LOW;
    setpointHigh = DEFAULT_SETPOINT_HIGH;
  }
}

void saveSetpoints() {
  EEPROM.put(EEPROM_ADDR_LOW, setpointLow);
  EEPROM.put(EEPROM_ADDR_HIGH, setpointHigh);
  EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_VAL);
  EEPROM.commit();
}

// ---------------------------------------------------------------
// Sensor read
// ---------------------------------------------------------------
void readSensor() {
  float h = sht31.readHumidity();
  float t = sht31.readTemperature();
  if (!isnan(h) && !isnan(t)) {
    currentHumidity = h;
    currentTemperature = t;
    sensorOk = true;
  } else {
    sensorOk = false;
  }

#if HAS_WATER_LEVEL_SENSOR
  // Float switch wired to pull the pin LOW when water is low.
  waterLow = (digitalRead(PIN_WATER_LOW) == LOW);
#else
  waterLow = false;
#endif
}

// ---------------------------------------------------------------
// Control logic: hysteresis with minimum on/off dwell time
// ---------------------------------------------------------------
void runControlLogic() {
  unsigned long now = millis();
  unsigned long sinceSwitch = now - lastRelaySwitch;

  // Safety cut-offs override everything else.
  if (!sensorOk || waterLow) {
    if (relayState) {
      setRelay(false);
      lastRelaySwitch = now;
    }
    return;
  }

  if (relayState) {
    // Currently ON, allow OFF only after min on time and above high setpoint
    if (currentHumidity >= setpointHigh && sinceSwitch >= MIN_ON_TIME_MS) {
      setRelay(false);
      lastRelaySwitch = now;
    }
  } else {
    // Currently OFF, allow ON only after min off time and below low setpoint
    if (currentHumidity < setpointLow && sinceSwitch >= MIN_OFF_TIME_MS) {
      setRelay(true);
      lastRelaySwitch = now;
    }
  }
}

// ---------------------------------------------------------------
// OLED status screen
// ---------------------------------------------------------------
#if HAS_OLED
void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Room Humidity Control");
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setCursor(0, 16);
  if (sensorOk) {
    display.print("RH: ");
    display.print(currentHumidity, 1);
    display.println(" %");
    display.print("Temp: ");
    display.print(currentTemperature, 1);
    display.println(" C");
  } else {
    display.println("Sensor error");
  }

  display.setCursor(0, 40);
  display.print("Humidifier: ");
  display.println(relayState ? "ON" : "OFF");

  display.setCursor(0, 52);
  if (waterLow) {
    display.println("Refill reservoir");
  } else {
    display.print("Range: ");
    display.print(setpointLow, 0);
    display.print("-");
    display.print(setpointHigh, 0);
    display.println("%");
  }

  display.display();
}
#endif

// ---------------------------------------------------------------
// Web dashboard
// ---------------------------------------------------------------
const char PAGE_HEADER[] PROGMEM =
  "<!DOCTYPE html><html><head><meta charset='utf-8'>"
  "<meta name='viewport' content='width=device-width, initial-scale=1'>"
  "<title>Room Humidity Control</title>"
  "<style>"
  "body{font-family:Arial,Helvetica,sans-serif;background:#f2f2f2;margin:0;padding:20px;}"
  ".card{background:#fff;border-radius:8px;padding:20px;max-width:420px;margin:0 auto;"
  "box-shadow:0 1px 3px rgba(0,0,0,0.15);}"
  "h1{font-size:20px;margin-top:0;}"
  ".reading{font-size:36px;font-weight:bold;margin:10px 0;}"
  ".label{color:#666;font-size:13px;}"
  ".status-on{color:#1a7f37;font-weight:bold;}"
  ".status-off{color:#999;font-weight:bold;}"
  ".warn{color:#b30000;font-weight:bold;}"
  "form{margin-top:16px;}"
  "input[type=number]{width:70px;padding:4px;}"
  "input[type=submit]{margin-top:10px;padding:8px 16px;background:#1a7f37;color:#fff;"
  "border:none;border-radius:4px;cursor:pointer;}"
  "</style></head><body><div class='card'>";

const char PAGE_FOOTER[] PROGMEM = "</div></body></html>";

void handleRoot() {
  String page;
  page.reserve(2048);
  page += FPSTR(PAGE_HEADER);
  page += "<h1>Room Humidity Control</h1>";

  if (sensorOk) {
    page += "<div class='label'>Relative Humidity</div>";
    page += "<div class='reading'>" + String(currentHumidity, 1) + " %</div>";
    page += "<div class='label'>Temperature</div>";
    page += "<div>" + String(currentTemperature, 1) + " C</div>";
  } else {
    page += "<div class='warn'>Sensor read error</div>";
  }

  page += "<p class='label'>Humidifier</p>";
  page += relayState ? "<p class='status-on'>ON</p>" : "<p class='status-off'>OFF</p>";

  if (waterLow) {
    page += "<p class='warn'>Water level low, refill reservoir</p>";
  }

  page += "<form action='/set' method='POST'>";
  page += "<div class='label'>Turn ON below (%)</div>";
  page += "<input type='number' step='0.5' name='low' value='" + String(setpointLow, 1) + "'>";
  page += "<div class='label'>Turn OFF above (%)</div>";
  page += "<input type='number' step='0.5' name='high' value='" + String(setpointHigh, 1) + "'>";
  page += "<br><input type='submit' value='Save'>";
  page += "</form>";

  page += "<p class='label'>Page refreshes every 5 seconds.</p>";
  page += "<script>setTimeout(function(){location.reload();},5000);</script>";
  page += FPSTR(PAGE_FOOTER);

  server.send(200, "text/html", page);
}

void handleSet() {
  if (server.hasArg("low") && server.hasArg("high")) {
    float low = server.arg("low").toFloat();
    float high = server.arg("high").toFloat();
    if (high > low && low >= 20.0f && high <= 80.0f) {
      setpointLow = low;
      setpointHigh = high;
      saveSetpoints();
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleData() {
  String json = "{";
  json += "\"humidity\":" + String(currentHumidity, 1) + ",";
  json += "\"temperature\":" + String(currentTemperature, 1) + ",";
  json += "\"relay\":" + String(relayState ? "true" : "false") + ",";
  json += "\"water_low\":" + String(waterLow ? "true" : "false") + ",";
  json += "\"setpoint_low\":" + String(setpointLow, 1) + ",";
  json += "\"setpoint_high\":" + String(setpointHigh, 1);
  json += "}";
  server.send(200, "application/json", json);
}

// ---------------------------------------------------------------
// Setup / loop
// ---------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  pinMode(PIN_RELAY, OUTPUT);
  setRelay(false);

#if HAS_WATER_LEVEL_SENSOR
  pinMode(PIN_WATER_LOW, INPUT_PULLUP);
#endif

  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, HIGH); // LED off (active LOW on most boards)

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  if (!sht31.begin(0x44)) {
    Serial.println("SHT31 not found, check wiring");
  }

#if HAS_OLED
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    display.setRotation(0);
    display.clearDisplay();
    display.display();
  } else {
    Serial.println("OLED not found, check wiring");
  }
#endif

  loadSetpoints();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
    digitalWrite(PIN_STATUS_LED, LOW); // LED on = Wi-Fi connected
  } else {
    Serial.println("Wi-Fi connect failed, running offline");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/set", HTTP_POST, handleSet);
  server.on("/data", HTTP_GET, handleData);
  server.begin();

  lastRelaySwitch = millis() - MIN_OFF_TIME_MS; // allow immediate first switch if needed
}

void loop() {
  server.handleClient();

  unsigned long now = millis();
  if (now - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
    lastSensorRead = now;
    readSensor();
    runControlLogic();

#if HAS_OLED
    updateDisplay();
#endif

    Serial.print("RH: ");
    Serial.print(currentHumidity);
    Serial.print(" %  Temp: ");
    Serial.print(currentTemperature);
    Serial.print(" C  Relay: ");
    Serial.print(relayState ? "ON" : "OFF");
    Serial.print("  WaterLow: ");
    Serial.println(waterLow ? "YES" : "NO");
  }
}
