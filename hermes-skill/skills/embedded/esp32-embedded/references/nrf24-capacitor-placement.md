# nRF24L01+PA+LNA Capacitor Placement on ESP32-S3

## Critical: One Capacitor Per Module

Each nRF24L01+PA+LNA module **MUST** have its own 100µF capacitor placed directly between its VCC and GND pins.

## Why One Capacitor Per Module?

The +PA+LNA module draws **100-120mA peak current** in short pulses during TX. The ESP32's 3.3V regulator cannot supply this fast transient current. Without a local capacitor:

- Voltage drops (brownout) during TX
- Packet corruption
- Range reduction
- Module reset
- Potential module damage

## Correct Placement

```
nRF24 #1:  100µF between VCC₁ ── GND₁  (directly on module pins)
nRF24 #2:  100µF between VCC₂ ── GND₂  (directly on module pins)
nRF24 #3:  100µF between VCC₃ ── GND₃  (directly on module pins)
...
```

**NOT**: One capacitor on ESP32's 3.3V rail shared by all modules.

## Technical Details

- **Capacitor**: 100µF electrolytic or tantalum + 0.1µF ceramic in parallel (for HF noise)
- **Placement**: As close as possible to module's VCC/GND pins (minimize trace inductance)
- **Why not shared**: Trace inductance prevents instantaneous current delivery during TX pulse

## Power Notes

| Component | Power | Notes |
|-----------|-------|-------|
| nRF24+PA+LNA | 3.3V from ESP32 | Each module needs local 100µF |
| ESP32-S3 3.3V regulator | Limited current | Cannot handle simultaneous TX pulses |
| External 5V for servo | Required | ESP32 5V pin insufficient |

## Common Mistakes

| Mistake | Consequence |
|---------|-------------|
| One capacitor for all modules | Brownout during simultaneous TX |
| Capacitor far from module | Inductance prevents fast current delivery |
| No capacitor | Module reset / packet loss / range loss |
| Using 10µF instead of 100µF | Insufficient charge for TX pulse |