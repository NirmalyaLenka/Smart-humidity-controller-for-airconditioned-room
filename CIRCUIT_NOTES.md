# Circuit Notes

## Power domains

Two separate power rails are used:

1. **12V rail** from the DC adapter, feeding the ultrasonic mist maker's driver board through the relay's COM/NO contacts.
2. **5V rail**, stepped down from the same 12V adapter through a small buck converter, feeding the NodeMCU VIN pin.

Keeping both on one adapter avoids needing two wall sockets. Do not tap 5V or 3V3 from the NodeMCU to power the mist maker driver board; it draws more current than the onboard regulator can safely supply.

## Relay wiring

- Relay module `IN` connects to NodeMCU `D5`.
- Relay module `VCC` connects to NodeMCU `3V3` or `5V` depending on the module (check the module's trigger voltage; most opto-isolated modules work fine from 3V3).
- Relay module `GND` connects to NodeMCU `GND`.
- The mist maker's 12V supply is broken and routed through the relay's `COM` and `NO` terminals, so the humidifier is only powered while the relay is energized.
- `RELAY_ACTIVE_LOW` in `config.h` should match your module. Most cheap single-channel modules pull their output relay in when the `IN` pin is LOW.

## Discrete transistor alternative (no relay module)

If building the switching stage from scratch instead of buying a module:

- 2N2222 or BC547 NPN transistor, base driven from the ESP8266 GPIO through a 1k resistor.
- Bare 5V or 12V relay coil as the collector load, emitter to GND.
- A 1N4007 (or similar) flyback diode across the relay coil, cathode to the supply side, to protect the transistor from the coil's back-EMF when it switches off.
- The relay's mechanical contacts then switch the mist maker exactly as described above.

## Water level safety cut-off

The ultrasonic transducer in a mist maker can overheat and fail if it runs without water. The optional float switch is wired between the water level pin and GND, with the ESP8266 internal pull-up enabled in firmware. When the reservoir drops below the float switch, the pin reads LOW, the firmware forces the relay off regardless of humidity reading, and the dashboard shows a refill warning.

## Sensor placement

- Mount the SHT31 away from the direct mist plume and away from the AC's cold air discharge; both will give a local reading that does not represent the room average.
- A height of around 1 - 1.5 m above the floor, on an interior wall away from windows and doors, gives a reading closer to the room's general condition.

## Grounding and mains safety

This project switches a low-voltage (12V) DC load only; it does not switch mains voltage. If you choose to control a mains-powered humidifier appliance instead of a bare mist maker module, use a relay module rated for mains voltage and current, keep all mains wiring physically separated from the low-voltage side, and add fusing on the mains side. Mains wiring should only be done by someone competent to do so safely.
