# ESP32-S3 Strapping & Safe Pin Reference

## Strapping Pins (DO NOT use for LCD/servo)

These pins are sampled at boot for flash/PSRAM config. Driving them from LCD or servo can prevent boot or cause `ledcAttach()` to hang.

| GPIO | Boot function | Conflict risk |
|------|--------------|---------------|
| 0 | Boot mode (GPIO0) | HIGH |
| 3 | UART0 RX / flash | MEDIUM |
| 15 | Chip flash config | HIGH — caused servo attach() hang |
| 16 | PSRAM / flash | MEDIUM |
| 17 | PSRAM / flash | MEDIUM |
| 18 | PSRAM / flash | LOW |

## Safe GPIOs for LCD data (empirically tested)

| LCD pin | Safe GPIO | Verified |
|---------|-----------|----------|
| D4 | 4 | ✅ |
| D5 | 5 | ✅ |
| D6 | 1 | ✅ (NOT 15) |
| D7 | 2 | ✅ (NOT 16) |
| RS | 12 | ✅ |
| E | 14 | ✅ |

## Safe GPIOs for Servo Signal

3, 10, 13, 17 — all confirmed working alongside LCD with pins above.

## Bad pin combos that will fail

| Attempt | Result | Why |
|---------|--------|-----|
| LCD D6=15, Servo=13 | ledcAttach() hangs | GPIO 15 strapping |
| LCD D6=15, Servo=3 | Same hang | Same cause |
| LCD D7=16, any servo | Intermittent glitch | GPIO 16 strapping |
