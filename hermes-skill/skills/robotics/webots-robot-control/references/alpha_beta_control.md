---
title: Alpha-Beta Control Law for Differential Drive Robots
category: robotics
---

# Alpha-Beta Control Law — Detailed Derivation

## Problem Setup

Differential-drive robot at pose `(x, y, θ)` in global frame. Goal at origin `(0, 0)`.

## Coordinate Definitions

| Symbol | Meaning | Formula |
|--------|---------|---------|
| `x, y` | Robot position (global frame) | from GPS |
| `θ` | Robot heading (global, from compass) | `atan2(c[0], c[1])` |
| `dx, dy` | Vector to goal | `dx = -x`, `dy = -y` |
| `r` | Distance to goal | `hypot(dx, dy)` |
| `λ` | Line-of-sight angle (global) | `atan2(dy, dx)` |
| `α` | Bearing error | `normalize(λ - θ)` |
| `β` | Auxiliary angle | `normalize(-θ)` or `normalize(-λ)` |

## Control Law

```
v = K_R * r
ω = K_α * α + K_β * β
```

Where:
- `v` — linear velocity (m/s)
- `ω` — angular velocity (rad/s)
- `K_R` — distance gain (typically 0.5-2.0)
- `K_α` — bearing gain (typically 1.0-5.0)
- `K_β` — heading correction gain (typically -0.5 to -2.0)

## Wheel Speeds for Differential Drive

```
w_left = v / R - (L / (2R)) * ω
w_right = v / R + (L / (2R)) * ω
```

Where:
- `R` — wheel radius (m)
- `L` — axle length (m)

## Stability Analysis

The closed-loop system with `v = K_R * r` and `ω = K_α * α + K_β * β` has equilibria at `r = 0`, `α = 0`, `β = 0`.

Linearization around equilibrium yields Jacobian eigenvalues that determine stability. For stability:
- `K_R > 0`
- `K_α > 0`
- `K_β < 0` (negative for correct final heading)

## Tuning Guidelines

| Scenario | K_R | K_α | K_β |
|----------|-----|-----|-----|
| Fast convergence | 1.5-2.0 | 3.0-5.0 | -1.5 to -2.0 |
| Smooth/No overshoot | 0.5-1.0 | 1.5-2.5 | -0.5 to -1.0 |
| e-puck default | 1.2 | 2.5 | -1.2 |

## Discretization Effects

Continuous controller `Ẋ = Ax` with Hurwitz `A` → discrete `z = e^(sT_s)`.

Sampling time constraint: `T_s < 2 / |max_eigenvalue|` for stability.

Large `T_s` can cause:
- Oscillations
- Limit cycles
- Instability

## References

- Siegwart, Nourbakhsh, Scaramuzza: "Introduction to Autonomous Mobile Robots", Chapter 3
- Aicardi et al. "Closed loop steering of unicycle like vehicles via Lyapunov techniques"