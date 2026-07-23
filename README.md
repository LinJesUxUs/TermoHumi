# TermoHumi

Thermo-hygrometer code, powered on ATMega, AHT10 sensor and 7 Segment Display.

## Overview

TermoHumi is a compact embedded firmware project for an ATmega8 microcontroller that reads temperature and humidity from an AHT10 sensor and displays the values on a multiplexed 4-digit 7-segment LED display. The display alternates between temperature (°C) and relative humidity (%) every few seconds.

## Features

- **AHT10 I2C Sensor** — high-accuracy digital temperature and humidity sensor
- **Dynamic 7-Segment Display** — 4-digit multiplexed display with timer interrupt for flicker-free output
- **Sign Handling** — supports negative temperature readings with a minus sign indicator
- **Automatic Sensor Initialization** — sends the required calibration command (0xE1, 0x08, 0x00) on startup
- **Software Reset** — fallback sensor reset if initialization fails
- **Written in C++** — class-based I2C driver with static members for minimal overhead

## Hardware

- **MCU:** ATmega8 (internal 2 MHz RC oscillator)
- **Sensor:** AHT10 (I2C address 0x38)
- **Display:** 4-digit 7-segment LED (common cathode/anode via PNP/NPN switching)
- **Programmer:** USBasp (ISP)

### Pin Assignment

| Port | Pin | Function                |
|------|-----|-------------------------|
| PC0  |  0  | Digit 1 select          |
| PC1  |  1  | Digit 2 select          |
| PC2  |  2  | Digit 3 select          |
| PC3  |  3  | Digit 4 select / symbol |
| PD0  |  0  | Segment A               |
| PD1  |  1  | Segment B               |
| PD2  |  2  | Segment C               |
| PD3  |  3  | Segment D               |
| PD4  |  4  | Segment E               |
| PD5  |  5  | Segment F               |
| PD6  |  6  | Segment G               |
| PD7  |  7  | Segment DP (unused)     |
| PC4  |  4  | I2C SDA                 |
| PC5  |  5  | I2C SCL                 |

> **Note:** Port B is reserved for ISP programming. Adjust pinout if using a different ATmega variant or board layout.

## Building

### Prerequisites

- `avr-g++` (AVR GCC toolchain)
- `avr-objcopy`
- `avrdude` (for flashing)
- `make`

### Compile

```bash
make
```

This produces `bin/main.hex` ready for flashing.

### Flash

```bash
make flash
```

### Verify Flash

```bash
make verification
```

### Clean

```bash
make clean      # remove object files
make fclean     # remove all build artifacts
```

## Configuration

Edit `src/includes.h` to adjust key parameters:

```c
#define F_CPU 2000000UL       // MCU clock frequency (Hz)
#define PERIOD_SEC 4          // Seconds between temperature/humidity display toggle
#define SEGMENT_NUM 4         // Number of 7-segment digits
```

The I2C bus speed is calculated in `AHT10::begin()` based on `F_CPU` (default: 100 kHz).

## Code Structure

```
.
├── src/
│   ├── main.cpp          # Application entry point, display logic, timer ISR
│   ├── AHT10.h           # AHT10 sensor driver class declaration
│   ├── AHT10.cpp         # AHT10 sensor driver implementation (I2C routines)
│   ├── structs.h         # Shared data structures (display buffer, sensor data)
│   └── includes.h        # Global definitions and MCU-specific includes
├── Makefile              # Build system
└── README.md
```

### Key Design Decisions

- **Timer1 interrupt** handles display multiplexing at a fixed refresh rate, ensuring flicker-free output regardless of sensor read delays.
- **Sensor reads** happen synchronously in the main loop with blocking `_delay_ms(80)` after triggering a measurement — this is acceptable because the display ISR runs independently.
- **Static class members** in `AHT10` avoid the overhead of multiple sensor instances (only one sensor is expected).

## Usage

1. Connect the hardware according to the pin assignment table.
2. Build and flash the firmware.
3. On power-up, the display shows temperature in °C.
4. After `PERIOD_SEC` seconds, the display switches to relative humidity (%).
5. The cycle repeats indefinitely.

### Display Output Format

- **Temperature:** `[sign] XX` (e.g., ` 24` for +24°C, `- 5` for -5°C)
- **Humidity:** ` XX` (e.g., ` 63` for 63%)

The 4th digit (rightmost) is used for the degree/humidity symbol or minus sign.

## Troubleshooting

| Symptom                     | Possible Cause                          |
|-----------------------------|-----------------------------------------|
| Display shows garbage/flicker | Check digit/segment pin connections     |
| Sensor returns error        | Verify I2C wiring (SDA/SCL), pull-up resistors (4.7kΩ) |
| AHT10 initialization fails  | Power-cycle sensor, check `_delay_ms(75)` after init |
| Compiler errors about `i`   | Ensure `sign` check is *inside* the `for` loop in `getTempHumi()` |
| avrdude "cannot find USB"   | Check USBasp connection and udev rules  |

## License

This project is released into the public domain. Feel free to use, modify, and distribute it without restriction.

---

**Author's Note:** This project was developed as an educational exercise in AVR C++ programming, I2C communication, and interrupt-driven display multiplexing. Contributions and suggestions are welcome.
