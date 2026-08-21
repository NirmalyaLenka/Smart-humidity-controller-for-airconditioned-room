#ifndef CONFIG_H
#define CONFIG_H

// ---------------------------------------------------------------
// Board: NodeMCU 1.0 (ESP-12E) or any ESP8266 dev board
// ---------------------------------------------------------------

// Feature toggles. Set to 0 to remove a part and cut cost further.
#define HAS_OLED              1   // 0.96" SSD1306 I2C display
#define HAS_WATER_LEVEL_SENSOR 1  // float switch / water level probe in the reservoir

// ---------------------------------------------------------------
// Pin mapping (NodeMCU silkscreen labels)
// ---------------------------------------------------------------
#define PIN_I2C_SDA   4   // D2
#define PIN_I2C_SCL   5   // D1
#define PIN_RELAY     14  // D5 - drives the relay module IN pin
#define PIN_WATER_LOW 12  // D6 - float switch / water level sensor input
#define PIN_STATUS_LED 2  // D4 - onboard LED, also used as Wi-Fi status indicator

// Relay module type. Most cheap single-channel modules are active LOW
// (output turns on when the input pin is pulled LOW). Flip this if
// your module is active HIGH.
#define RELAY_ACTIVE_LOW true

// ---------------------------------------------------------------
// Humidity control parameters
// ---------------------------------------------------------------
// Healthy indoor RH for an air-conditioned room is generally
// accepted as 40-60%. AC dries the air, so this project only adds
// moisture (does not dehumidify). Defaults below give a working
// band of 40-50% with hysteresis to stop relay chatter.

#define DEFAULT_SETPOINT_LOW   40.0f  // turn humidifier ON below this RH
#define DEFAULT_SETPOINT_HIGH  50.0f  // turn humidifier OFF above this RH

// Minimum time the relay must stay in a state before it can switch
// again. Protects the relay contacts and the ultrasonic mist maker
// from being pulsed on and off rapidly.
#define MIN_ON_TIME_MS   30000UL   // 30 s
#define MIN_OFF_TIME_MS  20000UL   // 20 s

// Sensor read interval
#define SENSOR_READ_INTERVAL_MS 3000UL

// ---------------------------------------------------------------
// Moving average filter
// ---------------------------------------------------------------
// Control decisions use the average of the last N humidity samples
// instead of the raw instantaneous reading. This smooths sensor
// noise so the relay does not toggle on single noisy samples near
// a setpoint. Raw and smoothed values are both exposed on the
// dashboard and /data endpoint.
#define MOVING_AVERAGE_SAMPLES 8

// ---------------------------------------------------------------
// Stall detection
// ---------------------------------------------------------------
// If the relay has been ON continuously for STALL_CHECK_TIME_MS and
// the smoothed humidity has risen by less than STALL_MIN_RISE over
// that window, something is wrong (empty reservoir past what the
// float switch caught, clogged transducer, disconnected pump, or a
// door/window left open). The controller latches a fault, forces the
// relay off, and shows a warning until cleared from the dashboard.
#define HAS_STALL_DETECTION 1
#define STALL_CHECK_TIME_MS (10UL * 60UL * 1000UL)  // 10 minutes
#define STALL_MIN_RISE 1.0f                          // percent RH

// ---------------------------------------------------------------
// AC-state gating
// ---------------------------------------------------------------
// When the AC is off, room humidity tends to drift back up on its
// own, so there is no point running the humidifier. Two detection
// modes are available:
//   0 = disabled, humidifier runs purely on the humidity band
//   1 = temperature trend heuristic (no extra hardware, uses the
//       SHT31 that is already on the board)
//   2 = current sensor on the AC's mains feed (extra hardware,
//       more reliable, see docs/CIRCUIT_NOTES.md for safe wiring)
#define AC_GATING_MODE 1

// Mode 1 (temperature trend) parameters. The controller tracks the
// highest temperature seen in the last AC_TEMP_WINDOW_MS and treats
// the AC as "on" once the current reading has dropped at least
// AC_TEMP_DROP_THRESHOLD below that recent high.
#define AC_TEMP_WINDOW_MS (20UL * 60UL * 1000UL)  // 20 minutes
#define AC_TEMP_DROP_THRESHOLD 1.0f                // degrees C

// Mode 2 (current sensor) parameters. Requires a non-invasive CT
// clamp (for example SCT-013-030) around one AC mains conductor,
// feeding a burden resistor and rectifier into an analog pin. Do
// not wire any sensor directly in series with mains wiring. See
// docs/CIRCUIT_NOTES.md before enabling this mode.
#define HAS_AC_CURRENT_SENSOR 0
#define PIN_AC_CURRENT A0
#define AC_CURRENT_THRESHOLD 100   // raw ADC peak, calibrate for your CT/burden setup

// EEPROM layout
#define EEPROM_SIZE       32
#define EEPROM_ADDR_LOW   0   // float, 4 bytes
#define EEPROM_ADDR_HIGH  4   // float, 4 bytes
#define EEPROM_MAGIC_ADDR 8   // 1 byte, marks that EEPROM has been written
#define EEPROM_MAGIC_VAL  0xA5

// ---------------------------------------------------------------
// Wi-Fi credentials
// ---------------------------------------------------------------
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"

// Web server port
#define WEB_SERVER_PORT 80

#endif

