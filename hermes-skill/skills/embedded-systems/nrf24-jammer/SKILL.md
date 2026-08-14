---
name: nrf24-jammer
description: ESP32/ESP32-S3 firmware for nRF24L01+PA+LNA based jammer with web interface, OLED, serial interface, and button controls.
category: embedded-systems
version: "3.0.0"
author: gitati16 (adapted for ESP32-S3)
---

# nRF24 Jammer Firmware

## Overview
ESP32/ESP32-S3 firmware for nRF24L01+PA+LNA based jammer with web interface, OLED display, serial interface, and button controls. Covers Bluetooth, BLE, Wi-Fi, Drone, and Zigbee jamming via constant carrier on 2.4GHz ISM band.

## Critical Platform Pitfalls (ESP32-standard → ESP32-S3)

### 1. Board Selection
- **WRONG**: `board = esp32dev` → produces "Invalid image block" on S3
- **RIGHT**: `board = esp32-s3-devkitc-1` in `platformio.ini`

### 2. HSPI Pins Must Be Explicit — ABSOLUTELY CRITICAL
- On ESP32, `hp->begin()` works with default HSPI pin mapping.
- On **ESP32-S3**, you MUST specify pins explicitly: `hp->begin(14, 12, 13)` (SCK, MISO, MOSI)
- Without this, `RF24.begin()` returns false → module not found → jammer does nothing.
- Symptom: `printDetails()` shows all zeros for registers, Model=nRF24L01+ but not transmitting.

### 3. OLED I2C Pins
- ESP32 default: SDA=21, SCL=22
- ESP32-S3 custom: Add `Wire.begin(4, 5)` before `display.begin(SSD1306_SWITCHCAPVCC, 0x3C)`
- If OLED stays dark, check that `Wire.begin()` is called AND pins exist on your board.

### 4. Button Pins (Input-only GPIOs on ESP32-S3)
- ESP32-S3 GPIOs 25, 26, 27 may not work as INPUT_PULLUP on all boards.
- Use GPIO 35, 36, 37 instead (these are input-capable and common on S3 boards).
- Change in `config.h`:
  - `btnOK(35)`, `btnNext(36)`, `btnPrevious(37)`
- **GyverButton constructor pitfall**: The version in PlatformIO only supports 3-parameter constructors: `GButton(int8_t pin, bool type, bool dir)`. Do NOT pass a 4th debounce argument. Set debounce separately with `btnOK.setDebounce(100)` in `setup()`.
- **Button noise from boost regulator**: When powering via TP4056 + boost converter, switching noise can cause false triggers. Add 100nF ceramic capacitor between each button GPIO pin and GND (at the ESP pin).

### 5. Capacitor 100µF is NOT optional
- Each nRF24L01+PA+LNA draws 100-120mA pulses during TX.
- Without 100µF across VCC/GND of each module, voltage droops → brownout → module resets mid-transmit.
- Simple RC explanation: the ESP32's 3.3V regulator provides voltage regulation but NOT instantaneous current. The capacitor acts as a charge buffer.

## Hardware Connections

### nRF24 Modules (SPI shared, CE/CSN separate)
| Module | CE | CSN | SCK | MOSI | MISO |
|--------|----|-----|-----|------|------|
| 1 | GPIO 16 | GPIO 15 | 14 | 13 | 12 |
| 2 | GPIO 18 | GPIO 17 | 14 | 13 | 12 |
| 3 | GPIO 23 | GPIO 19 | 14 | 13 | 12 |
| 4 | GPIO 5 | GPIO 4 | 14 | 13 | 12 |
| 5 | GPIO 33 | GPIO 32 | 14 | 13 | 12 |

- SPI is SHARED across all modules (SCK, MOSI, MISO are common)
- Only CE/CSN pins differ per module
- Each module MUST have its own 100µF capacitor between VCC and GND

### Power
- VCC: 3.3V with 100µF + 0.1µF caps per module
- GND: Common ground to all modules

### Buttons (ESP32-S3 safe GPIOs)
| Button | GPIO |
|--------|------|
| OK | 35 |
| NEXT | 36 |
| PREV | 37 |

### OLED (I2C — add explicit Wire.begin)
| Pin | GPIO |
|-----|------|
| SDA | 4 |
| SCL | 5 |

## Software Architecture

### Source Files (src/128x64_Flexible/ or src/128x32_Flexible/)
| File | Purpose |
|------|---------|
| `nRF24_jammer.cpp` | Main entry, web server routes, loop, OLED display |
| `jam.cpp` | nRF24 init/deinit, HSPI config, jammer logic (const carrier) |
| `deauth.cpp` | WiFi deauth attacks via promiscuous sniffing |
| `scan.cpp` | WiFi AP scanning with channel occupancy analysis |
| `serial.cpp` | Serial CLI interface with command handling |

### Include Files (include/128x64_Flexible/)
| File | Purpose |
|------|---------|
| `config.h` | Pin definitions, EEPROM config, global vars |
| `jam.h` | Jammer function declarations |
| `deauth.h` | Deauth function declarations |
| `scan.h` | Scan function declarations |
| `serial.h` | Serial interface declarations |
| `html.h` | Web interface HTML (embedded string) |
| `bitmap.h` | OLED bitmap images for menu icons |

### Jammer Core (jam.cpp) — Key Functions

```cpp
// Initialize RF24 objects with CE/CSN pins from EEPROM
void InitRadios()

// Configure HSPI with explicit pins + RF24 settings
void HSPI_init() {
  hp = new SPIClass(HSPI);
  hp->begin(14, 12, 13);      // SCK=14, MISO=12, MOSI=13 ← REQUIRED on S3
  hp->setFrequency(16000000);
  for (int i = 0; i < nrf24_count; i++) {
    radios[i]->begin(hp);
    radios[i]->setAutoAck(false);        // No ACK for jamming
    radios[i]->stopListening();
    radios[i]->setRetries(0, 0);
    radios[i]->setPALevel(RF24_PA_MAX, true);
    radios[i]->setDataRate(RF24_2MBPS);  // Max speed
    radios[i]->disableCRC();              // No CRC overhead
  }
}

// Per-protocol jammer loops — bluetooth_jam(), drone_jam(), wifi_jam(), etc.
void bluetooth_jam() {
  InitRadios();
  HSPI_init();
  for (int j = 0; j < nrf24_count; j++)
    radios[j]->startConstCarrier(PA_MAX, 45);  // Constant carrier wave
  while (!btnOK.isSingle()) {
    // Distribute Bluetooth channels (21 or 80) across available modules
    for (int j = 0; j < nrf24_count; j++)
      radios[j]->setChannel(bluetooth_channels[ch++]);
  }
  DeinitRadios(true);  // Stop carrier, power down
}
```

### Common Pitfalls During Development

| Symptom | Root Cause | Fix |
|---------|-----------|-----|
| `Invalid image block` on flash | Wrong board target in platformio.ini | Change to `esp32-s3-devkitc-1`, erase_flash, reflash |
| RF24 NOT FOUND / all registers zero | SPI pins not explicitly mapped | Add `hp->begin(14, 12, 13)` to HSPI_init |
| OLED stays dark | I2C pins not initialized | Add `Wire.begin(4, 5)` before display.begin() |
| OLED stays dark (address) | Wrong I2C address | Try 0x3C vs 0x3D |
| Buttons not working | GPIOs 25/26/27 not input-capable on S3 | Move to 35/36/37 |
| Module resets during TX | No 100µF capacitor across VCC/GND | Add 100µF per module directly on pins |
| EEPROM ssid/password NOT_FOUND | First boot, NVS empty | Normal. Device auto-APs with SSID "jammer". Set via web UI. |
| Web interface unreachable | EEPROM pins not set | CE/CSN default to 0 in EEPROM. Set via web UI (192.168.4.1). |
| `Invalid channel` or no transmission | Power supply droop | Add capacitor + check 3.3V stability under load |

### First Boot Sequence
1. ESP boots → EEPROM/NVS empty → `ssid NOT_FOUND` warnings (normal)
2. Device creates AP: SSID "jammer" / password "W0rthlessS0ul"
3. Connect to this WiFi from phone/laptop
4. Open `192.168.4.1` in browser → web UI
5. Go to nRF24 settings → set CE=16, CSN=15 (for one module)
6. Select jammer mode (Bluetooth/Drone/Wi-Fi/etc.) and enable

## Build & Flash (ESP32-S3)

### PlatformIO
```ini
[env:128x64_Flexible]
board = esp32-s3-devkitc-1       # NOT esp32dev!
build_src_filter = -<*> +<128x64_Flexible/>
build_flags = -std=gnu++17 -Wl,-z,muldefs -Iinclude/128x64_Flexible
```
```bash
git clone https://github.com/gitati16/nRF24_jammer.git
cd nRF24_jammer
# Edit platformio.ini board line first!
# Edit include/128x64_Flexible/config.h for pin changes
# Edit src/128x64_Flexible/jam.cpp for HSPI pins
pio run -e 128x64_Flexible
```

### Flashing
- PlatformIO Upload button → automatic
- ESPWebTool (https://esp.huhn.me/): bootloader.bin@0x1000, partitions.bin@0x8000, firmware.bin@0x10000
- If first flash fails: run `esptool.py --chip esp32s3 --port COM3 erase_flash` then retry

### Power
- USB power while testing
- For portable: TP4056 + Li-Ion 3.7V via buck converter/3.3V LDO to ESP32 5V pin
- DO NOT connect battery directly to 3.3V (3.7-4.2V > 3.3V tolerance)

## Document Output (for university reports)
The skill `persian-academic-report` covers full Persian DOCX generation. When producing a report for this project, include:
- Hardware block diagram (ESP32-S3 + nRF24 modules + OLED + buttons)
- Pin mapping table
- Explanation of 100µF capacitor necessity (voltage droop at TX pulse)
- Platform adaptation steps (esp32dev → esp32-s3-devkitc-1)
- Test results (Bluetooth jammer confirmed working)
