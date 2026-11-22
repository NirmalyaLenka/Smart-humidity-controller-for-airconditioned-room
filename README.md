# Smart Humidity Controller for an Air-Conditioned Room

An ESP8266 NodeMCU prototype that reads relative humidity and temperature from an SHT31 sensor and switches a low-voltage ultrasonic mist maker through a relay. A web page on the device displays the readings and allows the humidity on/off thresholds to be changed. An optional SSD1306 OLED and reservoir float switch are supported by compile-time flags.

> **Prototype and safety status:** The checked-in firmware is a simple hysteresis controller, not a validated indoor-climate or safety system. It does not dehumidify, and the current sketch does not implement all of the advanced options described by some constants in `config.h`. Read [Current behavior and limitations](#current-behavior-and-limitations) before wiring or relying on it.

## Implemented behavior

- Reads SHT31 humidity and temperature every 3 seconds.
- Default control band: turn the humidifier on below 40% RH and off at/above 50% RH.
- Enforces minimum relay on/off dwell times (30 seconds on and 20 seconds off by default).
- Forces the relay off when the SHT31 reading is invalid or the enabled float switch indicates low water.
- Optional SSD1306 OLED status display and optional low-water input.
- Stores the low/high setpoints in EEPROM.
- Serves a local dashboard at `/`, accepts setpoint updates at `POST /set`, and exposes readings/state at `GET /data`.

## Repository layout

All project files are at the repository root; there are no `firmware/` or `docs/` directories.

| Path | Purpose |
|---|---|
| [`smart_humidity_controller.ino`](smart_humidity_controller.ino) | Main ESP8266 sketch |
| [`config.h`](config.h) | Pin mappings, feature flags, Wi-Fi placeholders, and control constants |
| [`wiring_diagram.svg`](wiring_diagram.svg) | Wiring diagram |
| [`BOM.md`](BOM.md) | Bill of materials and approximate cost estimates |
| [`CIRCUIT_NOTES.md`](CIRCUIT_NOTES.md) | Power rails, relay wiring, water-level input, sensor placement, and safety notes |
| [`LICENSE`](LICENSE) | MIT license text |

## Hardware and pinout

The pin definitions are in `config.h` and use NodeMCU board labels as shown here.

| Function | NodeMCU label | ESP8266 GPIO / note |
|---|---|---|
| SHT31 SDA | D2 | GPIO4 |
| SHT31 SCL | D1 | GPIO5 |
| Relay module IN | D5 | GPIO14 |
| Optional float switch | D6 | GPIO12; switch to GND when water is low |
| On-board status LED | D4 | GPIO2; active-low on common boards |
| Optional SSD1306 OLED | D2 / D1 | Shared I2C bus; firmware uses address 0x3C |
| SHT31 sensor | D2 / D1 | Firmware initializes address 0x44 |

Check the exact board and module pin labels before wiring. The relay input polarity is controlled by `RELAY_ACTIVE_LOW`; the default is `true`.

### Power and load wiring

- Power the mist-maker driver from its specified **12 V supply** through the relay's COM/NO contacts.
- Power the NodeMCU from a suitable regulated **5 V DC-DC buck converter** connected to the board's VIN/5V input as appropriate for that board.
- Connect the low-voltage grounds as required by the selected relay module and circuit.
- Do not power the mist maker from the NodeMCU 3.3 V output.
- Keep water and condensation away from the controller, relay contacts, and power supply. The firmware is designed for a low-voltage 12 V DC mist-maker load; it is not a mains-wiring guide.

See [`CIRCUIT_NOTES.md`](CIRCUIT_NOTES.md) and [`wiring_diagram.svg`](wiring_diagram.svg) before assembling the circuit.

## Prerequisites

- Arduino IDE.
- ESP8266 board support installed through Boards Manager.
- Libraries installed through Library Manager:
  - **Adafruit SHT31 Library**
  - **Adafruit GFX Library** (only when `HAS_OLED` is `1`)
  - **Adafruit SSD1306** (only when `HAS_OLED` is `1`)

`Wire`, `EEPROM`, `ESP8266WiFi`, and `ESP8266WebServer` are provided by the ESP8266 Arduino core.

## Configure, build, and upload

1. Assemble and inspect the low-voltage circuit with the mist-maker load disconnected.
2. Open the root-level `smart_humidity_controller.ino` in Arduino IDE. Keep `config.h` in the same folder.
3. In `config.h`, set `WIFI_SSID` and `WIFI_PASSWORD` to the intended Wi-Fi network. Do not commit real credentials to a public repository.
4. Set `HAS_OLED` and `HAS_WATER_LEVEL_SENSOR` to `0` if the corresponding hardware is not installed.
5. Confirm the relay module's input polarity and set `RELAY_ACTIVE_LOW` accordingly. Verify the relay's default/off state before connecting the mist maker.
6. Select the matching NodeMCU / ESP8266 board and serial port, then compile and upload.
7. Open Serial Monitor at **115200 baud**. The firmware attempts Wi-Fi connection for up to 15 seconds and prints the assigned IP address if connected.
8. From a device on the same trusted network, open that IP address in a browser. The `/data` endpoint returns humidity, temperature, relay state, water-low state, and setpoints as JSON when sensor values are valid.

## Control settings

The current sketch uses these values from `config.h`:

| Setting | Default | Effect |
|---|---:|---|
| `DEFAULT_SETPOINT_LOW` | 40% RH | Humidifier may turn on below this threshold |
| `DEFAULT_SETPOINT_HIGH` | 50% RH | Humidifier may turn off at/above this threshold |
| `MIN_ON_TIME_MS` | 30,000 ms | Minimum relay on duration before normal threshold-based shutoff |
| `MIN_OFF_TIME_MS` | 20,000 ms | Minimum relay off duration before normal threshold-based startup |
| `SENSOR_READ_INTERVAL_MS` | 3,000 ms | Sensor sampling interval |
| `RELAY_ACTIVE_LOW` | `true` | Relay input polarity |
| `HAS_OLED` | `1` | Enables OLED code |
| `HAS_WATER_LEVEL_SENSOR` | `1` | Enables the low-water input |

The dashboard accepts a new pair only when the high threshold is greater than the low threshold, the low threshold is at least 20, and the high threshold is at most 80. Accepted setpoints are stored in EEPROM and restored on boot.

## Current behavior and limitations

The current `smart_humidity_controller.ino` does **not** use the following advanced constants present in `config.h`: `MOVING_AVERAGE_SAMPLES`, `HAS_STALL_DETECTION`, `STALL_CHECK_TIME_MS`, `STALL_MIN_RISE`, `AC_GATING_MODE`, `AC_TEMP_WINDOW_MS`, `AC_TEMP_DROP_THRESHOLD`, `HAS_AC_CURRENT_SENSOR`, or `AC_CURRENT_THRESHOLD`. Therefore, moving-average filtering, stall-fault detection, AC-state gating, and current-transformer sensing should not be treated as implemented features. The current relay decisions use the latest accepted SHT31 humidity reading, simple hysteresis, dwell times, sensor validity, and the enabled water-level input.

Other limitations:

- The dashboard and `/set` endpoint have no authentication or HTTPS. Keep the device on a trusted, isolated local network; do not port-forward it or expose it directly to the internet.
- The `/data` handler formats sensor values as numbers. If a reading is unavailable and represented as `nan`, the response may not be valid JSON; consumers should validate the response.
- The controller only adds moisture. It cannot reduce humidity.
- No calibration procedure, long-term logging, alarm service, or automatic recovery test is implemented.
- A float switch is optional in the firmware and cannot replace checking the reservoir and transducer physically.

## Troubleshooting

| Symptom | Checks |
|---|---|
| SHT31 not found | Check D2/D1 wiring, power, ground, and I2C address (0x44 is used by the sketch) |
| OLED blank | Confirm `HAS_OLED` is `1`, the library is installed, and the display uses the expected I2C wiring/address |
| Relay stays on or behaves inverted | Disconnect the mist maker, verify module trigger polarity, then check `RELAY_ACTIVE_LOW` |
| Water-low warning appears continuously | Check the float-switch wiring; the enabled input is expected to pull GPIO12 LOW when water is low |
| No IP address | Recheck Wi-Fi credentials and 2.4 GHz network availability; inspect Serial Monitor at 115200 baud |
| Mist maker runs when it should not | Disconnect its supply while debugging; verify sensor readings, relay polarity, and the low-water input before reconnecting the load |
| Browser cannot reach dashboard | Confirm the ESP8266 joined Wi-Fi, use the printed local IP, and keep the browser on the same reachable network |

## License and contact

MIT; see [`LICENSE`](LICENSE).

The previous README listed `carmodbhai@gmail.com` as a contact address.
