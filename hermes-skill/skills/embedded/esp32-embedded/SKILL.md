---
name: esp32-embedded
category: embedded
description: >-
  ESP32 development in Arduino IDE — servo, LCD, sensors, pin mapping,
  library selection, and common pitfalls for ESP32, ESP32-S3, and ESP32-CAM.
  Covers the full workflow: board setup, library install, wiring, debugging,
  and integration with Django/Telegram projects.
---

# ESP32 Embedded Development

## Board Selection (Arduino IDE)

| Board | In IDE | Notes |
|-------|--------|-------|
| ESP32 Dev Module | Tools → Board → ESP32 Arduino → "ESP32 Dev Module" | Classic ESP32 |
| ESP32-S3 (USB OTG) | Tools → Board → ESP32 Arduino → "ESP32S3 Dev Module" | **ESP32-CAM**, your N16R8 |
| ESP32-WROOM | Same as ESP32 Dev Module | — |

## Must-Install Libraries

Install via **Tools → Manage Libraries**:

| Library | Search keyword | Why |
|---------|---------------|-----|
| **ESP32Servo** | `ESP32Servo` | Standard servo — **can hang on S3** with LCD |
| **LiquidCrystal I2C** | `LiquidCrystal I2C` | LCD with I2C backpack (4-wire) |
| **LiquidCrystal** | `LiquidCrystal` | LCD **parallel** (16-pin, no backpack) |
| **ESP32_ISR_Servo** | `ESP32_ISR_Servo` | Alternative servo lib for S3 (avoids `attach()` hang) |

**🔴 RECOMMENDED (S3):** Skip `ESP32Servo` on ESP32-S3. Use **`ledcAttach` / `ledcWrite`** (native ESP32 Arduino Core 3.x API) instead — no extra library needed.

## Servo on ESP32-S3 — The Correct Way

### ✅ Working approach (no extra library):

```cpp
#define SERVO_PIN 13
#define SERVO_CHANNEL 0

void setup() {
  ledcAttach(SERVO_PIN, 50, 14);  // pin, freq=50Hz, resolution=14-bit
}

void setAngle(int angle) {
  // 500µs = 0°, 2400µs = 180° → map to 14-bit duty (0–16383)
  long minDuty = (500 * 16383) / 20000;   // ~409
  long maxDuty = (2400 * 16383) / 20000;  // ~1966
  int duty = minDuty + (maxDuty - minDuty) * angle / 180;
  ledcWrite(SERVO_CHANNEL, duty);
}
```

### Key API differences (Core 3.x vs older):

| Old API | New API (Core 3.x) |
|---------|-------------------|
| `ledcSetup(channel, freq, res)` | `ledcAttach(pin, freq, res)` |
| `ledcAttachPin(pin, channel)` | *(merged into ledcAttach)* |
| `ledcWrite(channel, duty)` | `ledcWrite(channel, duty)` — same |

## LCD 16×2 Parallel (16-pin) on ESP32-S3

### ⚠️ Critical: Strapping pins

Pins **0, 3, 15, 16, 17, 18** on ESP32-S3 are used at boot (flash/PSRAM). Using them for LCD can cause **boot failure, servo hang, or erratic behaviour**.

### ✅ Safe pins for LCD data on S3:

| LCD pin | Safe GPIO | Notes |
|---------|-----------|-------|
| D4 | **4** | OK |
| D5 | **5** | OK |
| D6 | **1** | ✅ very safe |
| D7 | **2** | ✅ very safe |
| RS | **12** | OK |
| E | **14** | OK |

```cpp
LiquidCrystal lcd(12, 14, 4, 5, 1, 2);  // RS, E, D4, D5, D6, D7
```

### 🔴 Servo + LCD together (S3):

When combining servo + parallel LCD on ESP32-S3:

1. Use **`ledcAttach`** (no ESP32Servo library) for the servo
2. Assign **safe GPIOs** for LCD (see table below)
3. Assign servo to a **safe GPIO** that is NOT also assigned to LCD
4. Use **external 5V** for the servo (ESP32's 5V pin cannot supply enough current)
5. Common GND between ESP32, servo, and LCD

**🛑 GPIO 15 on ESP32-S3 is a strapping pin. DO NOT use it for LCD data lines (D6, D7).**
Using GPIO 15 for LCD D6 will cause `ledcAttach()` or `ESP32Servo.attach()` to **hang the board**. Symptoms:
- servo never moves
- code reaches "LCD OK" but hangs on "Before attach"
- no error message, just infinite hang

**✅ Safe LCD pin assignment for ESP32-S3 (empirically tested):**

| LCD Pin | GPIO | 
|---------|------|
| RS | 12 |
| E | 14 |
| D4 | 4 |
| D5 | 5 |
| D6 | **1** (NOT 15) |
| D7 | **2** (NOT 16) |

Servo Signal can go to GPIO 3, 10, 13, or 17 — all confirmed working alongside LCD on the pins above.

## Power Notes

| Component | Power | Notes |
|-----------|-------|-------|
| Servo (HPI SF10W, SG90, MG996R) | **External 5V** | ESP32 5V pin NOT sufficient for servo |
| LCD 16×2 (parallel) | 5V from ESP32 | OK, low current (~1–2mA) |
| LCD I2C backpack | 5V or 3.3V | Check module spec |
| ESP32-S3 | USB-C | 5V from USB is fine |

**Failure symptoms of under-powered servo:**
- Servo twitches but doesn't move
- Servo moves erratically when LCD updates
- ESP32 spontaneously resets when servo is under load
- `ledcAttach()` hangs the board

## Debugging Checklist for ESP32-S3

1. **Does the code compile?** → Check library compatibility (ESP32Servo vs ledcAttach)
2. **Does the LCD work?** → Run standalone LCD test (no servo)
3. **Does the servo work?** → Run standalone servo test (no LCD)
4. **Do they work together?** → Pin conflict between servo and LCD pins
5. **Still stuck?** → Try `ESP32_ISR_Servo` library OR use `ledcAttach`/`ledcWrite` (recommended)

## References

- `references/s3-pinout.md` — ESP32-S3 strapping pin table
- `references/servo-lcd-debugging.md` — Debugging transcript from a real session
- `references/nrf24-capacitor-placement.md` — One capacitor per nRF24L01+PA+LNA module
- `references/oled-i2c-pin-remap.md` — OLED I2C pin remapping on ESP32-S3
