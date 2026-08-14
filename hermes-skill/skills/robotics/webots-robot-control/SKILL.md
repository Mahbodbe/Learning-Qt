---
name: webots-robot-control
description: Webots robot control for e-puck and differential drive robots. Covers polar coordinate control (Alpha-Beta law), deadzone handling, state machine patterns, and Webots controller structure for Iranian university robotics courses.
category: robotics
tags: [webots, e-puck, polar-coordinates, alpha-beta-control, deadzone, differential-drive, control-systems]
version: 1.0.0
author: Hermes
created: 2026-07-31
---

# Webots Robot Control — e-puck & Differential Drive

Skill for controlling e-puck and differential-drive robots in Webots simulator. Covers polar coordinate control (Alpha-Beta law), deadzone handling, state machine patterns, and Webots controller structure — tailored for Iranian university robotics/control courses.

## Trigger

- User asks for Webots controller code for e-puck or differential drive
- User mentions "polar coordinate control", "Alpha-Beta control", "Alpha-Beta control law"
- User asks about deadzone handling for e-puck motors ([-1, 1] deadzone)
- User asks for 4-phase control state machine (align → rotate → forward → stop)
- User mentions Webots, e-puck, GPS, Compass, GPS+Compass pose estimation

## Core Concepts

### Polar Coordinate Control (Alpha-Beta Law)

Classic control law for differential-drive robots to reach a goal pose:

```
v = K_R * r
ω = K_alpha * alpha + K_beta * beta
```

**Variables (robot frame → goal at origin):**
- `r = sqrt(x² + y²)` — distance to goal (0,0)
- `λ = atan2(-y, -x)` — line-of-sight angle to goal (global frame)
- `θ` — robot heading (yaw) from compass, global frame
- `α = normalize(λ - θ)` — bearing error (heading error to goal)
- `β = normalize(-θ)` — heading error to align with goal direction at arrival

**Gains (tuned for e-puck):**
- `K_R = 1.2` — linear velocity gain
- `K_alpha = 2.5` — bearing error gain
- `K_beta = -1.2` — final heading correction gain

### Normalize Angle to [-π, π]

```python
def normalize(angle):
    return math.atan2(math.sin(angle), math.cos(angle))
```

### Deadzone Handling for e-puck Motors

e-puck motors have a deadzone `[-1, 1]` rad/s — velocities in this range produce no motion.

**Solution: Boost function to push output above deadzone**

```python
DEADBAND = 1.0
MIN_SPEED = 1.05  # slightly above deadzone

def boost(val):
    if val == 0:
        return 0
    if abs(val) < MIN_SPEED:
        return MIN_SPEED if val > 0 else -MIN_SPEED
    return val

# Usage:
motor(boost(v + omega), boost(v - omega))
```

### 4-Phase State Machine Pattern

Common pattern for "go to origin" tasks:

```python
MODE_ALIGN = 1      # Align to 90° reference
MODE_ROTATE = 2     # Rotate to face goal (theta control)
MODE_FORWARD = 3    # Move straight to goal (r control)
MODE_STOP = 4       # Goal reached

mode = MODE_ALIGN

# In main loop:
if mode == MODE_ALIGN:
    # Align to 90° reference
elif mode == MODE_ROTATE:
    # Rotate to face goal (theta control)
elif mode == MODE_FORWARD:
    # Move to goal (radius control)
elif mode == MODE_STOP:
    motor(0, 0)
```

### Webots e-puck Sensor Reading

```python
# GPS (ground plane X-Y)
pos = gps.getValues()
x, y = pos[0], pos[1]

# Compass (heading vector)
c = compass.getValues()
theta = math.atan2(c[0], c[1])  # heading from X axis
```

### Wheel Speed from v, ω

```python
WHEEL_RADIUS = 0.0205
AXLE_LENGTH = 0.052

w_left = v / WHEEL_RADIUS - (AXLE_LENGTH / (2 * WHEEL_RADIUS)) * omega
w_right = v / WHEEL_RADIUS + (AXLE_LENGTH / (2 * WHEEL_RADIUS)) * omega
```

## Common Control Theory Questions (Iranian University Courses)

### Q: Why add -θ and -α to β-dot in state equations?
**A:** In the Alpha-Beta formulation, `β = -λ` (or `-θ` for final alignment). The term `β-dot` captures how the goal direction changes. Adding `-θ` and `-α` ensures the controller corrects both the current heading error and the bearing error simultaneously for better convergence.

### Q: Discretization effects on continuous controller gains?
**A:** When continuous gains (designed for `Ẋ = Ax`, A Hurwitz) are discretized with sampling time `T_s`, the discrete poles shift: `z = e^(sT_s)`. Large `T_s` can push poles outside unit circle → instability. Must check: `T_s < 2/|max_eigenvalue|` for stability.

### Q: Can gain interval guarantee performance beyond stability?
**A:** Stability (poles inside unit circle) ≠ performance. Gain interval ensuring stability may still give poor settling time, overshoot, or steady-state error. Need robust performance analysis (H∞, μ-synthesis) or at least check step response across interval.

### Q: Can controller achieve r→0 and θ→θd?
**A:** Yes if: (1) system is controllable, (2) gains tuned so `v ∝ r`, `ω ∝ α + β`, (3) no deadzone/saturation prevents small corrections. If deadzone exists, need `boost()` or integrator to overcome.

## Webots Controller Template

```python
from controller import Robot, AnsiCodes
import math

print(AnsiCodes.CLEAR_SCREEN)

robot = Robot()
timestep = int(robot.getBasicTimeStep())

# Devices
gps = robot.getDevice('gps'); gps.enable(timestep)
compass = robot.getDevice('compass'); compass.enable(timestep)
leftMotor = robot.getDevice('left wheel motor')
rightMotor = robot.getDevice('right wheel motor')
leftMotor.setPosition(float('inf')); rightMotor.setPosition(float('inf'))

# Constants
MAX_WHEEL_SPEED = 6.28
WHEEL_RADIUS = 0.0205
AXLE_LENGTH = 0.052

# Gains
K_R = 1.2
K_ALPHA = 2.5
K_BETA = -1.2

def motor(left, right):
    left = max(min(left, 6.28), -6.28)
    right = max(min(right, 6.28), -6.28)
    # Deadzone handling (commented for now)
    # if 0 < left < 1: left = 0
    # if -1 < left < 0: left = 0
    # if 0 < right < 1: right = 0
    # if -1 < right < 0: right = 0
    leftMotor.setVelocity(left)
    rightMotor.setVelocity(right)

def normalize(angle):
    return math.atan2(math.sin(angle), math.cos(angle))

# Main loop
while robot.step(timestep) != -1:
    # Read sensors
    # Compute control
    # Apply motor commands
    pass
```

## Persian Academic Report Generation

For university reports (آزمایشگاه کنترل دیجیتال):

1. Write markdown with `<div dir="rtl">...</div>` wrapper
2. Use pandoc: `pandoc report.md -o report.docx --from markdown`
3. Apply RTL fix: wrap entire content in `<div dir="rtl">...</div>`
4. Publish: `chmod 644 /var/www/resin-media/name.docx`

## Related Skills

- `communication/mahboob-persona` — warm Persian tone for this user
- `productivity/persian-academic-report` — report generation workflow
- `embedded/esp32-embedded` — for ESP32 hardware integration

## References

- `references/alpha_beta_control.md` — detailed Alpha-Beta control derivation
- `references/deadzone_handling.md` — deadzone analysis and boost function variants
- `references/state_machine_patterns.md` — 4-phase and other state machine patterns
- `templates/webots_controller_template.py` — starter template
- `scripts/deadzone_test.py` — test deadzone behavior