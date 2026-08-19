# Smart Humidity Controller for an AC Room

Automatically maintains a healthy relative humidity band in an air-conditioned room by switching an ultrasonic mist maker on and off through a relay, based on live readings from an SHT31 digital humidity/temperature sensor.

AC systems dry out indoor air, and prolonged exposure to low humidity (below roughly 30-35% RH) is associated with dry skin, irritated eyes and airways, and increased static and dust. This project targets a working band of 40-50% RH by default, adjustable from a web dashboard.

## Why not DHT11 / DHT22

The DHT11 has roughly +/-5% RH accuracy, updates slowly, and drifts noticeably over months of continuous use. The DHT22 is somewhat better but still less stable than a digital I2C sensor over long-term operation in a humid microclimate near a mist maker. This project uses the **SHT31**, which is factory-calibrated to roughly +/-2% RH, reads faster over I2C, and holds calibration much better over time. An SHT21 module is a reasonable same-price substitute if SHT31 is not locally available.

## Features

- Reads RH and temperature every few seconds from an SHT31.
- Hysteresis control (default: ON below 40% RH, OFF above 50% RH) with a minimum on/off dwell time so the relay and pump are not switched rapidly.
- Web dashboard served directly from the ESP8266: live readings, humidifier state, and an editable setpoint form.
- `/data` JSON endpoint for logging or integrating with other tools.
- Setpoints are stored in EEPROM and survive power loss.
- Optional water level cut-off so the mist maker cannot run dry.
- Optional local OLED status screen.
- Every extra part is behind a compile-time flag in `config.h`, so the build can be trimmed down to the cheapest working version.

## Repository layout

```
smart-humidity-controller/
  firmware/
    smart_humidity_controller.ino   main sketch
    config.h                        pin mapping and tunable parameters
  docs/
    wiring_diagram.svg              block wiring diagram
    BOM.md                          parts list with approximate cost
    CIRCUIT_NOTES.md                relay/transistor driving and safety notes
  README.md
  LICENSE
```

## Hardware

See `docs/BOM.md` for the full parts list and pricing, and `docs/CIRCUIT_NOTES.md` for wiring and safety detail. Summary:

- NodeMCU ESP8266
- SHT31 humidity/temperature sensor (I2C)
- 1-channel relay module
- Ultrasonic mist maker (12V) as the humidifying element
- 12V DC adapter, plus a 5V buck converter to power the NodeMCU from the same supply
- Optional: SSD1306 OLED, float switch water level sensor

Wiring overview is in `docs/wiring_diagram.svg`.

## Firmware setup

1. Install the ESP8266 board package in the Arduino IDE (Boards Manager, search "esp8266").
2. Install libraries via Library Manager:
   - Adafruit SHT31 Library
   - Adafruit GFX Library (only needed if `HAS_OLED` is 1)
   - Adafruit SSD1306 (only needed if `HAS_OLED` is 1)
3. Open `firmware/smart_humidity_controller.ino`.
4. Edit `firmware/config.h`:
   - Set `WIFI_SSID` and `WIFI_PASSWORD`.
   - Set `HAS_OLED` and `HAS_WATER_LEVEL_SENSOR` to 0 if you are not building those parts.
   - Adjust `DEFAULT_SETPOINT_LOW` / `DEFAULT_SETPOINT_HIGH` if you want a different starting range (these can also be changed later from the web dashboard).
5. Select your board (NodeMCU 1.0) and port, then upload.
6. Open the Serial Monitor at 115200 baud to see the assigned IP address once Wi-Fi connects.
7. Visit that IP address in a browser on the same network to view the dashboard.

## Calibration check

SHT31 modules ship factory-calibrated, but it is worth a sanity check after assembly: compare the reading against a known reference (a  analog hygrometer, or a saturated salt calibration if you want to be thorough) and confirm it lands within a couple of percent. If your sensor reads far off, check the I2C address (0x44 default, 0x45 on some boards with the ADDR pin pulled high) and wiring before assuming the sensor is faulty.

## Operating notes

- Refill the reservoir before it runs dry; the optional float switch will cut power to the relay and show a warning on the dashboard and OLED if it does.
- Clean the ultrasonic transducer disc periodically; mineral buildup from tap water reduces mist output over time and can eventually stop it working. Distilled or RO water extends the interval between cleanings.
- Recheck the setpoint band seasonally. What counts as comfortable shifts with outdoor climate and how long the AC runs per day.

## License

MIT, see `LICENSE`.
## contact details for any help 
my gmail is carmodbhai@gmail.com 
if you want to contact through call or whatsapp contact 9692005585
##                  Thankyou
