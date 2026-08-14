---
name: arduino-esp32-development
description: "Program ESP32 (ESP32-S3, ESP32-CAM) with Arduino IDE: servo, LCD, pin mapping, PWM, sensor integration"
version: 1.0.0
author: Hermes Agent
license: MIT
platforms: [windows, linux, macos]
metadata:
  hermes:
    tags: [esp32, arduino, embedded, servo, lcd, pwm, iot]
    related_skills: [systematic-debugging]
---

# Arduino ESP32 Development

## When to Use

- User programs ESP32 (especially ESP32-S3 N16R8 / ESP32-CAM) via Arduino IDE
- User has servo motor, LCD 16x2, sensors in a project (e.g., smart parking gate for instrumentation course)
- User asks about pinouts, libraries, PWM, code for ESP32

## Communication Style (MANDATORY — see 'persian-speaking-technical-peer' skill)

This skill carries the user's communication preferences in its own SKILL.md.
Load it for full details (ultra-terse, Persian, peer-critic, project separation, etc.).
Key rules applicable here:
- **Persian by default** (casual/friendly, NOT formal)
- **Ultra-terse format**: abbreviate, use → arrows, no filler sentences
- **After fix**: confirm with `✅ fix: [what changed].` and **stop** — do NOT re-explain
- **Never re-explain what the fix does** after they've seen it work
- **This user is first-time with Arduino IDE** — be more pedagogical and step-by-step when explaining IDE setup, but still terse for code/troubleshooting

## Essential Background

### ESP32 vs ESP32-S3 Differences

| Feature | ESP32 | ESP32-S3 |
|---------|-------|----------|
| Core | Xtensa dual-core LX6 | Xtensa dual-core LX7 |
| PSRAM | Optional | Built-in (N16R8 = 16MB Flash, 8MB PSRAM) |
| Strapping pins | GPIO 0, 2, 5, 12, 15 | GPIO 0, 3, 46 — **different set!** |
| Safe GPIO for LCD/Servo | Any except strapping | **Avoid GPIO 3, 6, 7, 8, 9, 10, 11, 15, 16, 18, 19, 20, 46** — many are PSRAM/Flash reserved on S3 |
| LEDC API (v3.x) | `ledcSetup` + `ledcAttachPin` | `ledcAttach(pin, freq, res)` + `ledcWrite(pin, duty)` (unified in v3.x) |

### Safe GPIO Pins for ESP32-S3 (proven working)

For **parallel LCD 16x2** (6 pins): use GPIO 1, 2, 4, 5, 12, 14
For **servo signal**: GPIO 13, 10, 17, 21 all work

**Strapping pins on S3 that cause boot issues if pulled high/low:**
- GPIO 0, 3, 46 — avoid for critical output; GPIO 3 = U0RX, used during boot

## Servo Motor (HPI SF10W) Without Libraries

The standard `ESP32Servo` library often **hangs** on ESP32-S3 (Arduino Core v3.x). Use **native LEDC PWM** instead — no extra library needed.

### Wiring (HPI SF10W)

```
Servo Signal (white) → GPIO 13
Servo VCC (red)      → 5V external ⚠️ (ESP32 5V pin cannot supply servo current)
Servo GND (black)    → GND (share between servo PSU and ESP32)
Servo common GND is REQUIRED — connect ESP32 GND to external PSU GND.
```

### Code Template

```cpp
#include <LiquidCrystal.h>

#define SERVO_PIN 13
LiquidCrystal lcd(12, 14, 4, 5, 1, 2);  // RS, E, D4, D5, D6, D7

// 14-bit resolution: duty range ~409 (0°) to ~1966 (180°)
int angleToDuty(int angle) {
  long minDuty = (500 * 16383) / 20000;    // ~409
  long maxDuty = (2400 * 16383) / 20000;   // ~1966
  return minDuty + ((maxDuty - minDuty) * angle / 180);
}

void setup() {
  ledcAttach(SERVO_PIN, 50, 14);   // freq=50Hz, resolution=14bit
  ledcWrite(SERVO_PIN, angleToDuty(0));

  lcd.begin(16, 2);
  lcd.print("Ready");
}

void loop() {
  // Sweep 0→180
  for (int a = 0; a <= 180; a++) {
    ledcWrite(SERVO_PIN, angleToDuty(a));
    lcd.setCursor(0, 1);
    lcd.print("Angle: ");
    lcd.print(a);
    lcd.print("   ");
    delay(15);
  }
  delay(1000);

  // Sweep 180→0
  for (int a = 180; a >= 0; a--) {
    ledcWrite(SERVO_PIN, angleToDuty(a));
    lcd.setCursor(0, 1);
    lcd.print("Angle: ");
    lcd.print(a);
    lcd.print("   ");
    delay(15);
  }
  delay(1000);
}
```

### Servo Not Moving? Checklist

1. ⚡ **External power**: ESP32's 5V pin cannot drive a servo — use external 5V PSU, share GND
2. 📌 **Wrong pin**: On S3, avoid strapping pins (3, 46) and PSRAM pins (6-11, 15-20)
3. 🔄 **LEDC API**: On Arduino Core 3.x, use `ledcAttach(pin, 50, 14)` + `ledcWrite(pin, duty)` — NOT the old `ledcSetup`/`ledcAttachPin`
4. ⏱️ **Duty range**: 500–2400 µs = duty 409–1966 (at 14-bit, 50Hz). If out of range, servo won't move
5. 🔌 **Signal wire**: White wire is signal, red is VCC, black is GND — verify

## LCD 16x2 Parallel (16-pin)

### Safe Pin Mapping for ESP32-S3

| LCD pin | Function | GPIO |
|---------|----------|------|
| 4 | RS | 12 |
| 6 | E | 14 |
| 11 | D4 | 4 |
| 12 | D5 | 5 |
| 13 | D6 | 1 |
| 14 | D7 | 2 |

**Initialize:**
```cpp
LiquidCrystal lcd(12, 14, 4, 5, 1, 2);
```

### Contrast Potentiometer

10K pot: one leg → 5V, wiper → V0 (pin 3), other leg → GND. Without this, LCD stays blank.

## Common Errors & Fixes

### `ledcSetup was not declared in this scope`
**Cause:** Arduino Core v3.x renamed the LEDC API.
**Fix:** Replace `ledcSetup(ch, freq, res)` + `ledcAttachPin(pin, ch)` with `ledcAttach(pin, freq, res)`. Replace `ledcWrite(ch, duty)` with `ledcWrite(pin, duty)`.

### `myServo.attach()` hangs / never returns
**Cause:** `ESP32Servo` library has compatibility issues with ESP32-S3 on Core 3.x. The timer driver conflict stalls the chip.
**Fix:** Use native LEDC PWM (see code template above) — no library needed.

### Servo jitters or buzzes but doesn't move
1. Signal wire wrong pin (check strapping pin list)
2. Duty cycle out of valid range (try 500–2400 µs mapped to PWM resolution)
3. Insufficient power supply (servo needs peak 500mA–1A)

### CPU crashes / watchdog reset when LCD + servo both active
**Probable cause:** GPIO conflict — servo signal and LCD data both using PSRAM-reserved pins (6-11, 15-20 on S3).
**Fix:** Map LCD data to GPIO 1, 2, 4, 5. Map servo signal to GPIO 13 or 21.

## Pitfalls

- **Servo VCC from ESP32 5V pin**: The ESP32-CAM/S3 5V pin can only supply ~100mA — not enough for any servo larger than 9g micro servo. Always use external 5V PSU (phone charger / power bank).
- **Missing common GND**: If servo has its own PSU, you MUST connect GND of both PSU and ESP32. Otherwise the signal has no reference.
- **Parallel LCD without potentiometer**: The LCD contrast pin (V0, pin 3) needs a voltage divider. Floating = blank screen.
- **GPIO 3 on S3**: Do NOT use for servo — it's U0RX, used during boot. Holding it low prevents flashing.
- **Arduino IDE vs PlatformIO**: This user uses Arduino IDE on Windows. Board = `ESP32S3 Dev Module`. Port = COM port from Device Manager (CP210x or CH340 driver needed).
- **OOM on server when testing**: The ESP32 development is on a different machine — server RAM has no effect on Arduino IDE compilation.
