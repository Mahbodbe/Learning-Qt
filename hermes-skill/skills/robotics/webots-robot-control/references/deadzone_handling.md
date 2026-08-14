---
title: Deadzone Handling for e-puck Motors
category: robotics
---

# Deadzone Handling for e-puck Motors in Webots

## The Problem

e-puck motors in Webots have a **deadzone** (region of no motion) approximately **[-1, 1] rad/s**.

```
Velocity range:    -6.28 ────────────────── 0 ────────────────── +6.28
Deadzone:                    -1 ────── 0 ────── +1
```

Any velocity command with magnitude < 1.0 rad/s produces **zero torque** → motor doesn't move.

## Why This Breaks Control

In proportional control (e.g., `ω = K_α * α`):

- As error `α → 0`, control output `ω → 0`
- Once `|ω| < 1.0`, motor stops
- Robot stops **before reaching goal** → steady-state error

## Solution: Boost Function

Push small non-zero outputs to just above deadzone:

```python
DEADBAND = 1.0
MIN_SPEED = 1.05  # slightly above deadzone

def boost(val):
    """Push small outputs above deadzone, preserve sign."""
    if val == 0:
        return 0
    if abs(val) < MIN_SPEED:
        return MIN_SPEED if val > 0 else -MIN_SPEED
    return val

# Usage in motor command:
def motor(left, right):
    left = max(min(left, 6.28), -6.28)
    right = max(min(right, 6.28), -6.28)
    leftMotor.setVelocity(boost(left))
    rightMotor.setVelocity(boost(right))
```

## How Boost Works

| Control Output | Boosted Output | Effect |
|----------------|----------------|--------|
| `0.0` | `0.0` | Stop at goal |
| `0.01` | `+1.05` | Move slowly forward |
| `-0.01` | `-1.05` | Move slowly backward |
| `2.0` | `2.0` | Normal speed |
| `-3.0` | `-3.0` | Normal speed reverse |

## Alternative Approaches

### 1. Integral Action (PID)
Add integral term to accumulate error until output > deadzone:

```python
integral += error * dt
output = Kp * error + Ki * integral
```

**Downside:** Windup, slower, more complex tuning.

### 2. Dither Signal
Add small high-frequency dither to escape deadzone:

```python
dither = 0.5 * math.sin(robot.getTime() * 50)
motor(boost(left + dither), boost(right + dither))
```

**Downside:** Vibration, wear, not deterministic.

### 3. Feedforward + Feedback
Add small feedforward to overcome static friction:

```python
base = 1.05 if direction > 0 else -1.05
output = Kp * error + base
```

## Deadzone in Motor Function (Explicit)

```python
DEADBAND = 1.0

def motor(left, right):
    # Clamp
    left = max(min(left, 6.28), -6.28)
    right = max(min(right, 6.28), -6.28)
    
    # Deadzone: zero out speeds in [-1, 1]
    if -DEADBAND < left < DEADBAND:
        left = 0
    if -DEADBAND < right < DEADBAND:
        right = 0
    
    leftMotor.setVelocity(left)
    rightMotor.setVelocity(right)
```

**Note:** This explicit deadzone in motor() + boost() is the recommended combination:
- `motor()` zeros tiny values (prevents jitter at exactly 0)
- `boost()` ensures commanded motion actually moves the robot

## Tuning MIN_SPEED

| Robot | Deadzone | MIN_SPEED | Notes |
|-------|----------|-----------|-------|
| e-puck (Webots) | [-1, 1] | 1.05 | Just above 1.0 |
| Real e-puck | ~[-0.5, 0.5] | 0.6 | Hardware dependent |
| Custom | [a, b] | \|b\| + 0.05 | 5% margin |

## Testing Deadzone

```python
# Test script: sweep velocity and log actual motion
for cmd in [i/10 for i in range(-15, 16)]:
    motor(cmd, cmd)
    delay(500)
    pos = gps.getValues()
    print(f"Cmd: {cmd:.1f} → Pos: {pos[0]:.3f}")
```

Expected: No position change for `|cmd| < 1.0`, linear motion for `|cmd| ≥ 1.05`.

## Common Pitfalls

1. **Forgetting boost()** → robot stops 2-3cm from goal
2. **MIN_SPEED too high** → robot overshoots/oscillates
3. **MIN_SPEED too low** → still in deadzone, no motion
4. **Forgetting sign preservation** → wrong direction
5. **Applying boost to 0** → robot never fully stops