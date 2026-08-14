# ESP32-S3 Servo + LCD 16x2 — Working Code Reference

**Board:** ESP32-S3 N16R8 (ESP32-CAM)
**IDE:** Arduino IDE (Windows)
**Core:** ESP32 Arduino Core v3.3.10
**User:** Mahbod (مهبد)

## Critical Finding: LEDC API v3.x

Arduino Core v3.x (for ESP32-S3) uses a **unified LEDC API** that differs from both the classic ESP32 API and the v2.x transitional API.

### Working API calls

```cpp
// Attach: one call replaces setup + attachPin
ledcAttach(pin, freq, resolution);       // returns bool

// Write: uses pin, not channel
ledcWrite(pin, duty);

// OR (alternative, also works):
ledcAttachChannel(pin, freq, resolution, channel);
ledcWriteChannel(channel, duty);
```

### Broken API calls (will not compile on Core 3.x)

```cpp
ledcSetup(channel, freq, resolution);    // ERROR: not declared
ledcAttachPin(pin, channel);              // ERROR: not declared
// Use ledcAttach() instead
```

## Working Code — Servo + LCD

This code works on ESP32-S3 N16R8 with Arduino Core 3.3.10:

```cpp
#include <LiquidCrystal.h>

const int rs = 14, en = 13, d4 = 12, d5 = 11, d6 = 10, d7 = 9;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int servoPin = 17;
const int freq = 50;
const int resolution = 14;
const int ledcChannel = 0;

int angleToDuty(int angle) {
  long minDuty = (500 * 16383) / 20000;    // ~409  (0°)
  long maxDuty = (2400 * 16383) / 20000;   // ~1966 (180°)
  return minDuty + ((maxDuty - minDuty) * angle / 180);
}

void setup() {
  lcd.begin(16, 2);
  lcd.print("System Ready");
  delay(1000);

  ledcAttachChannel(servoPin, freq, resolution, ledcChannel);
  // or: ledcAttach(servoPin, freq, resolution);

  lcd.clear();
  lcd.print("Ready!");
  delay(1000);
}

void loop() {
  ledcWriteChannel(ledcChannel, angleToDuty(0));
  lcd.setCursor(0, 1);
  lcd.print("Angle: 0 deg  ");
  delay(2000);

  ledcWriteChannel(ledcChannel, angleToDuty(90));
  lcd.setCursor(0, 1);
  lcd.print("Angle: 90 deg ");
  delay(2000);

  ledcWriteChannel(ledcChannel, angleToDuty(180));
  lcd.setCursor(0, 1);
  lcd.print("Angle: 180 deg");
  delay(2000);
}
```

## Safe Pin Mapping (ESP32-S3)

### LCD 16x2 (Parallel)

| LCD Pin | Function | GPIO | Notes |
|---------|----------|------|-------|
| 4 | RS | 14 | Safe |
| 6 | E | 13 | Safe |
| 11 | D4 | 12 | Safe |
| 12 | D5 | 11 | OK, but avoid if PSRAM issues |
| 13 | D6 | 10 | OK, but avoid if PSRAM issues |
| 14 | D7 | 9 | OK, but avoid if PSRAM issues |

### Servo Signal

GPIO 17 — completely safe, no strapping, no PSRAM conflict.

### GND common

Servo MUST share GND with ESP32 even when using external 5V PSU.

## Pin Avoidance on ESP32-S3

| GPIO | Reason |
|------|--------|
| 0 | Strapping (boot mode) |
| 3 | U0RX, strapping |
| 6-11 | PSRAM/Flash (okay for LCD in some boards, avoid for servo) |
| 15, 16 | PSRAM issues |
| 18, 19, 20 | PSRAM |
| 43, 44 | USB JTAG |
| 46 | Strapping |
