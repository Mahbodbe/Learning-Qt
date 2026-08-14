# ESP32-S3 Servo + LCD Debugging Transcript

## The Problem

Combining a **parallel LCD 16×2** (16 pins) with a **servo motor** on **ESP32-S3 N16R8** (ESP32-CAM) caused the servo to hang.

## Root Causes Found

### 1. Library conflict

`ESP32Servo.attach()` hangs on ESP32-S3 when certain pins are used by the LCD. The library uses the legacy MCPWM driver which conflicts with PSRAM/Flash pins on S3.

**Fix:** Use native `ledcAttach(pin, freq, resolution)` instead — no extra library.

### 2. Strapping pins on ESP32-S3\n\nOn S3, GPIO pins 0, 3, 15, 16, 17, 18 are used at boot time for flash/PSRAM. Using them for LCD data lines causes:\n- Boot failure\n- Servo hang on `attach()`\n- Random resets\n\n**GPIO 15 is the most dangerous one.** Using GPIO 15 for LCD D6 will cause `ledcAttach()` to hang immediately — no error message, no crash, just infinite hang.\n\n**Fix:** Use safe pins: GPIO 1, 2, 4, 5, 12, 14 for LCD. Never use 15 or 16.

### 3. `next` parameter lost in POST login form

The `hidden input` field `name="next"` in `login.html` was sending `next` via POST body, but the Django view was reading from `request.GET.get('next')` instead of `request.POST.get('next')`.

**Fix:** 
```python
next_url = request.POST.get('next') or request.GET.get('next')
```

### 4. `update.callback_query` can be None

In Telegram bot handlers, `update.callback_query` is `None` when the update is a plain message, not a button callback. Calling `.answer()` on `None` crashes the bot.

**Fix:**
```python
query = update.callback_query
if not query:
    return
await query.answer()
```

## Pin Table (Safe for ESP32-S3)

| Component | GPIO | Notes |
|-----------|------|-------|
| LCD RS | 12 | OK |
| LCD E | 14 | OK |
| LCD D4 | 4 | OK |
| LCD D5 | 5 | OK |
| LCD D6 | 1 | ✅ safe |
| LCD D7 | 2 | ✅ safe |
| Servo Signal | 3, 10, 17 | Any safe GPIO not used by LCD |

## Working Code Pattern

```cpp
#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 14, 4, 5, 1, 2);
#define SERVO_PIN 17

void setup() {
    ledcAttach(SERVO_PIN, 50, 14);
    lcd.begin(16, 2);
}

void setAngle(int angle) {
    long minDuty = (500 * 16383) / 20000;
    long maxDuty = (2400 * 16383) / 20000;
    int duty = minDuty + (maxDuty - minDuty) * angle / 180;
    ledcWrite(0, duty);
}
```
