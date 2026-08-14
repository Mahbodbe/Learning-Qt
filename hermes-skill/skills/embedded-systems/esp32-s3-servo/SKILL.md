---
name: esp32-s3-servo
description: Control servo motors and parallel 16x2 LCD on ESP32-S3 with Arduino IDE — pin strapping pitfalls, LEDC API, and safe wiring.
category: embedded-systems
---

# ESP32-S3 Servo + LCD Control

## Overview
Use this skill when working with servo motors and 16x2 parallel LCD on **ESP32-S3 N16R8** (ESP32-CAM or dev board) in Arduino IDE. Captures pin-strapping pitfalls and the LEDC API that works reliably on S3.

## 🚫 ESP32Servo Library — Why It Hangs on S3

The `ESP32Servo` library calls `ledcSetup`/`ledcAttachPin` under the hood. On ESP32-S3 with Arduino Core 3.x:

- **Hangs** on `attach()` when LCD data lines are on strapping pins (GPIO 6–11, 15–18) — the PWM timer conflicts with internal flash/PSRAM operations.
- **Solution:** Use ESP32's **LEDC peripheral** directly (no library).

## 🔌 Safe Pin Assignment (Servo + LCD 16x2 Parallel)

### Rules
| Pin Group | ESP32-S3 Status |
|-----------|-----------------|
| GPIO **6–11** | Reserved for internal flash/PSRAM — **DO NOT USE** |
| GPIO **15–18** | Strapping pins — cause instability with PWM |
| GPIO **1–5, 12–14, 21, 42–46** | Safe for general use |

### Recommended Wiring
| Component | Pin | Notes |
|-----------|------|-------|
| **Servo Signal** | GPIO **10** | Safe on S3, clean PWM |
| **LCD RS** | GPIO 12 | |
| **LCD E** | GPIO 14 | |
| **LCD D4** | GPIO 4 | |
| **LCD D5** | GPIO 5 | |
| **LCD D6** | GPIO **1** | Safe despite being UART0 |
| **LCD D7** | GPIO **2** | |
| Servo VCC | **5V external** | ESP32's 5V pin cannot supply servo current |
| LCD VCC | 5V | |
| All GND | Common ground | Critical for stability |

### LCD Contrast
Connect V0 (pin 3) to wiper of **10K potentiometer** between 5V and GND.

## ⚙️ LEDC API (Arduino Core 3.x)

```cpp
#include <LiquidCrystal.h>

#define SERVO_PIN 10

LiquidCrystal lcd(12, 14, 4, 5, 1, 2);  // RS, E, D4, D5, D6, D7

void setServoAngle(int angle) {
  // 500µs (0°) → 2400µs (180°) at 50Hz, 16-bit
  // duty = pulse_width / (1000000 / freq) * (2^resolution - 1)
  int duty = map(angle, 0, 180, 1638, 7864);
  ledcWrite(SERVO_PIN, duty);
}

void setup() {
  ledcAttach(SERVO_PIN, 50, 16);  // pin, 50Hz, 16-bit resolution
  lcd.begin(16, 2);
  lcd.print("System Ready");
}

void loop() {
  for (int angle = 0; angle <= 180; angle++) {
    setServoAngle(angle);
    lcd.setCursor(0, 1);
    lcd.print("Angle: ");
    lcd.print(angle);
    lcd.print("   ");
    delay(15);
  }
  delay(1000);
  // repeat reverse...
}
```

## 🔄 LEDC API Versions

| Arduino Core | API to Use | Example |
|-------------|-----------|---------|
| **≤ 2.x** (old) | `ledcSetup(ch, freq, res)` + `ledcAttachPin(pin, ch)` + `ledcWrite(ch, duty)` | Uses channel number |
| **≥ 3.x** (S3) | `ledcAttach(pin, freq, res)` + `ledcWrite(pin, duty)` | **PIN is channel** — no separate channel setup |
| `servo.h` | `myservo.write(angle)` | Universal, but hangs on S3 with conflicting pins |

## 🛠️ Complete Working Code

```cpp
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 14, 4, 5, 1, 2);
const int servoPin = 10;

void setServoAngle(int angle) {
  int duty = map(angle, 0, 180, 1638, 7864);
  ledcWrite(servoPin, duty);
}

void setup() {
  Serial.begin(115200);
  ledcAttach(servoPin, 50, 16);
  lcd.begin(16, 2);
  lcd.print("Ready");
  setServoAngle(0);
  delay(1000);
}

void loop() {
  for (int a = 0; a <= 180; a++) {
    setServoAngle(a);
    lcd.setCursor(0, 1);
    lcd.print("A: ");
    lcd.print(a);
    lcd.print("   ");
    delay(10);
  }
  delay(1500);
  for (int a = 180; a >= 0; a--) {
    setServoAngle(a);
    lcd.setCursor(0, 1);
    lcd.print("A: ");
    lcd.print(a);
    lcd.print("   ");
    delay(10);
  }
  delay(1500);
}
```

## ⚠️ Pitfalls
- **Servo doesn't move but LCD works** → Check servo VCC is from **external 5V**, not ESP32 pin
- **`ledcSetup` not declared** → You're on Core 3.x; use `ledcAttach(pin, freq, res)` instead
- **ESP32-S3 board selection** → Tools → Board → ESP32 Arduino → **ESP32S3 Dev Module** (not plain ESP32 Dev Module)
- **Upload fails on COM port** → Check Tools → Port for correct COM; install CP210x/CH340 driver if no port appears
- **LCD shows blocks, no text** → Adjust contrast pot; verify RW pin is tied to GND
