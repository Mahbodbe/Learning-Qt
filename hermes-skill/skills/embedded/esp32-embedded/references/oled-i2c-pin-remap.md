# OLED I2C Pin Remapping on ESP32-S3

## Default vs Custom I2C Pins

The ESP32-S3 I2C driver (`Wire` library) supports **pin remapping** via `Wire.begin(SDA, SCL)`. The default pins (21=SDA, 22=SCL) can be changed to any GPIO.

## For nRF24_jammer Project (128x32_Flexible)

### Hardware
- OLED SSD1306 128x32
- ESP32-S3
- Default I2C: SDA=21, SCL=22

### Custom Pins (for shorter wiring)
| Signal | GPIO | Notes |
|--------|------|-------|
| SDA | **4** | Safe, I2C capable |
| SCL | **5** | Safe, I2C capable |

### Code Changes

**File: `src/128x32_Flexible/nRF24_jammer.cpp` in `setup()`**
```cpp
// Find:
Wire.begin(21, 22);

// Change to:
Wire.begin(4, 5);  // SDA=4, SCL=5
```

**No changes needed in `config.h`** - OLED is initialized with `&Wire` which uses the remapped pins automatically.

### ESP32-S3 I2C Capability
All GPIOs 0-21 and 26-47 support I2C (input/output capable). GPIO 4 and 5 are both I2C-capable and not strapping pins.

### Verification
```cpp
Wire.begin(4, 5);
// Check:
if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed!");
} else {
    Serial.println("OLED OK on GPIO 4/5");
}
```

## Why Custom Pins?
- Shorter wiring when OLED is physically close to GPIO 4/5
- Avoids long wires that act as antennas (noise pickup)
- Cleaner PCB layout if designing custom board