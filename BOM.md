# Bill of Materials

Prices are approximate street/online prices in India (INR) at the time of writing. They will vary by seller.

| Part | Purpose | Approx. price (INR) |
|---|---|---|
| NodeMCU ESP8266 (ESP-12E) | Controller, runs the web dashboard | 200 - 280 |
| SHT31 breakout module | Humidity and temperature sensing, I2C | 250 - 400 |
| 1-channel 5V relay module | Switches the mist maker on/off | 50 - 90 |
| Ultrasonic mist maker (12V, 20mm disc + driver board) | Adds moisture to the air | 150 - 300 |
| 12V 2A DC adapter | Powers the mist maker driver board | 200 - 300 |
| 5V buck converter (or AMS1117 module) | Steps 12V down to power the NodeMCU from the same supply | 40 - 80 |
| Small plastic reservoir (0.5 - 1 L) | Holds water for the mist maker | 50 - 100 (or reused container) |
| Float switch / water level probe | Optional, stops the pump running dry | 30 - 80 |
| 0.96" SSD1306 OLED (I2C) | Optional, local status display | 120 - 180 |
| Perfboard, wires, screw terminals, heat shrink | Assembly | 100 - 150 |

**Estimated total, full build with OLED and water level sensor: roughly 1200 - 1900 INR.**
**Minimum viable build (no OLED, no water level sensor): roughly 850 - 1350 INR.**

## Cost-cutting notes

- The relay module can be replaced with a single NPN transistor (2N2222 or BC547), a 1N4007 flyback diode, and a bare relay for a saving of roughly 30-40 INR. The firmware logic is identical either way.
- A small aquarium air pump plus a wick/sponge humidifier is a lower-power, mechanically simpler alternative to the ultrasonic mist maker if you cannot source one locally, though it humidifies more slowly.
- The OLED and water level sensor are both optional in firmware (`HAS_OLED`, `HAS_WATER_LEVEL_SENSOR` in `config.h`). Skipping them does not change the core control logic.
- SHT31 is the accuracy/price sweet spot recommended here. If unavailable, an HTU21D/SHT21 module is a similar-priced alternative with comparable ±2-3% RH accuracy. Avoid DHT11 (±5% RH, slow, drifts over time) and DHT22 (better than DHT11 but still noticeably less stable than an SHT-series digital sensor over long-term use in a humid environment).
