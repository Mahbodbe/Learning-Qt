#!/usr/bin/env python3
"""
Deadzone behavior test script for e-puck motors in Webots.
Run this to characterize the deadzone of your specific motor model.
"""

DEADBAND = 1.0
MIN_SPEED = 1.05

def boost(val):
    if val == 0:
        return 0
    if abs(val) < MIN_SPEED:
        return MIN_SPEED if val > 0 else -MIN_SPEED
    return val

def motor(left, right):
    left = max(min(left, 6.28), -6.28)
    right = max(min(right, 6.28), -6.28)
    # Deadzone
    if -1.0 < left < 1.0: left = 0
    if -1.0 < right < 1.0: right = 0
    left = boost(left)
    right = boost(right)
    return left, right

def test_deadzone():
    """Sweep command velocities and predict actual motor output."""
    print("Deadzone Test: Command → Actual Motor Output")
    print("=" * 50)
    print(f"Deadzone: [-1.0, 1.0]")
    print(f"Min Speed (boost): {MIN_SPEED}")
    print()
    
    test_values = [i/10 for i in range(-15, 16)]  # -1.5 to 1.5
    
    for cmd in test_values:
        actual_left, actual_right = motor(cmd, cmd)
        if actual_left == 0:
            status = "NO MOTION (deadzone)"
        else:
            status = "MOTION"
        print(f"Cmd: {cmd:>5.1f} → Actual: {actual_left:>5.2f} | {status}")

def test_boost_edge_cases():
    """Test boost function edge cases."""
    print("\nBoost Function Edge Cases:")
    print("=" * 50)
    
    cases = [0, 0.01, -0.01, 0.5, -0.5, 1.0, -1.0, 1.05, -1.05, 2.0, -2.0]
    for val in cases:
        boosted = boost(val)
        print(f"boost({val:>5.2f}) = {boosted:>5.2f}")

if __name__ == "__main__":
    test_deadzone()
    test_boost_edge_cases()
    
    print("\n" + "=" * 50)
    print("INTERPRETATION:")
    print("- Values in [-1.0, 1.0] → NO MOTION (deadzone)")
    print("- Values in (1.0, 1.05) → BOOSTED to ±1.05")
    print("- Values ≥ 1.05 → PASSED THROUGH unchanged")
    print("\nIf your robot stops before goal, check:")
    print("1. Is boost() applied to motor commands?")
    print("2. Is MIN_SPEED > 1.0 (e.g., 1.05)?")
    print("3. Is deadzone correctly identified for your robot?")