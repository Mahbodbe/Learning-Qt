# ESP32-S3 Servo Pitfalls

- **MCPWM Deprecation Warning:** The `ESP32Servo` library might use legacy MCPWM drivers, showing a `#warning "legacy MCPWM driver is deprecated..."`. This is harmless and won't break functionality. If you want to remove it, you'd need to use a newer ESP32 Arduino Core version or migrate to the `driver/mcpwm_prelude.h` API directly (which is more complex).

- **Servo Attach Failure (`ESP32_ISR_Servo`):** `myServo.attach()` might fail (hang) on ESP32-S3 with certain GPIO pins due to conflicts with strapping pins or internal flash/PSRAM interfaces.
  - **Cause:** Using pins like GPIO 6, 7, 15, 16 for Servo Signal. These pins have specific roles during ESP32 boot.
  - **Safe Pins for Servo Signal:** GPIO 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13, 14, 17, 18, 19, 20, 21.
  - **Recommended Fix:** Use GPIO pins like 1, 2, 3, 10, 13, 14, 17 etc. for the servo signal. Avoid the strapping pins if possible.
  - **Test:** Try a different GPIO pin for the servo signal (e.g., GPIO 10 instead of 13) if `attach()` fails.

- **LCD + Servo Conflict:** Using specific LCD data pins (like D6=GPIO 15, D7=GPIO 16) can interfere with the servo signal, especially if those pins are also used by the servo or have other internal functions. This causes the servo to stop moving even if the LCD works.
  - **Safe LCD Pins:** GPIO 1, 2, 4, 5, 12, 14 are generally safe for LCD parallel interface.
  - **Fix:** Use safe GPIO pins for LCD data lines D4-D7 as recommended (e.g., 4, 5, 1, 2).

- **Power Supply Issues:** Servos require significant current, especially during movement. Trying to power them directly from ESP32's 5V pin often leads to insufficient voltage/current, causing the servo to stall or the ESP32 to brownout/reset.
  - **Fix:** Always use a separate, adequate external 5V power supply for the servo. Connect the external supply's GND to the ESP32's GND.

- **Incorrect `ledcWrite` Usage (ESP32 Arduino Core v3+):** The `ledcWrite()` function signature changed. It now expects `ledcWrite(uint8_t channel, uint32_t duty)` instead of `ledcWrite(uint8_t pin, uint32_t duty)`. Also, `ledcSetup()` and `ledcAttachPin()` are replaced by `ledcAttach(pin, freq, resolution)`.
  - **Fix:** Use the new `ledcAttach(pin, freq, resolution)` for initialization and `ledcWrite(channel, duty)` for controlling the servo pulse.

## Important Libraries:

- **ESP32Servo:** For basic servo control (might have issues on S3).
- **ESP32_ISR_Servo:** A more robust alternative for ESP32s, especially S3, using interrupt timers.
- **LiquidCrystal:** Standard Arduino library for parallel LCDs.

## Testing Strategy:

1.  **Test LCD alone:** Verify LCD initialization and display without servo connected.
2.  **Test Servo alone:** Verify servo movement with a simple sweep code (e.g., `myServo.write(angle)` directly, no LCD code).
3.  **Combine:** Integrate LCD and Servo code, ensuring correct pin assignments and reliable power supply.
4.  **Troubleshoot Servo:** If servo doesn't move but LCD works:
    *   Check power supply (external 5V, common GND).
    *   Try different GPIO pins for servo signal (avoid strapping pins).
    *   Consider switching to `ESP32_ISR_Servo` library if basic `ESP32Servo` fails.
    *   Verify `ledcAttach`/`ledcWrite` signatures if using direct LEDC control.

Remember to check the ESP32-S3 datasheet for specific GPIO restrictions and strapping pin behavior during boot.
