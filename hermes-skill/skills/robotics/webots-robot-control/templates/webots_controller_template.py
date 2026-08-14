from controller import Robot, AnsiCodes
import math

print(AnsiCodes.CLEAR_SCREEN)

# ============================================================================
# Webots e-puck Controller Template
# ============================================================================

robot = Robot()
timestep = int(robot.getBasicTimeStep())

# --- Devices ---
gps = robot.getDevice('gps')
gps.enable(timestep)

compass = robot.getDevice('compass')
compass.enable(timestep)

leftMotor = robot.getDevice('left wheel motor')
rightMotor = robot.getDevice('right wheel motor')
leftMotor.setPosition(float('inf'))
rightMotor.setPosition(float('inf'))
leftMotor.setVelocity(0)
rightMotor.setVelocity(0)

# --- Constants ---
MAX_WHEEL_SPEED = 6.28      # rad/s
WHEEL_RADIUS = 0.0205       # m
AXLE_LENGTH = 0.052         # m
MAX_LINEAR = MAX_WHEEL_SPEED * WHEEL_RADIUS
MAX_ANGULAR = 2 * MAX_WHEEL_SPEED * WHEEL_RADIUS / AXLE_LENGTH

# --- Control Gains (tune for your robot) ---
K_R = 1.2
K_ALPHA = 2.5
K_BETA = -1.2

# --- Deadzone Handling ---
DEADBAND = 1.0
MIN_SPEED = 1.05

def boost(val):
    """Push small outputs above deadzone, preserve sign."""
    if val == 0:
        return 0
    if abs(val) < MIN_SPEED:
        return MIN_SPEED if val > 0 else -MIN_SPEED
    return val

def motor(left, right):
    """Apply motor velocities with clamping and deadzone handling."""
    left = max(min(left, 6.28), -6.28)
    right = max(min(right, 6.28), -6.28)
    
    # Deadzone: zero out tiny values to prevent jitter
    if -1.0 < left < 1.0: left = 0
    if -1.0 < right < 1.0: right = 0
    
    # Boost to overcome deadzone
    leftMotor.setVelocity(boost(left))
    rightMotor.setVelocity(boost(right))

def normalize(angle):
    """Return angle in [-pi, pi]."""
    return math.atan2(math.sin(angle), math.cos(angle))


# ============================================================================
# Main Control Loop
# ============================================================================

while robot.step(timestep) != -1:
    # --- Read Sensors ---
    pos = gps.getValues()
    x, y = pos[0], pos[1]
    
    c = compass.getValues()
    theta = math.atan2(c[0], c[1])   # heading angle from X axis
    
    # --- Error to Goal (goal at 0,0) ---
    dx = -x
    dy = -y
    r = math.hypot(dx, dy)
    
    # Stop if close enough
    if r < 0.02:
        motor(0, 0)
        print("Goal reached!")
        break
    
    # --- Polar Control Law (Alpha-Beta) ---
    lam = math.atan2(dy, dx)           # line-of-sight angle to goal
    alpha = normalize(lam - theta)     # bearing error
    beta = normalize(-lam)             # final heading correction
    
    # Control law
    v = K_R * r
    omega = K_ALPHA * alpha + K_BETA * beta
    
    # Limit velocities
    v = max(-MAX_LINEAR, min(MAX_LINEAR, v))
    omega = max(-MAX_ANGULAR, min(MAX_ANGULAR, omega))
    
    # Wheel speeds (differential drive)
    w_right = v / WHEEL_RADIUS + (AXLE_LENGTH / (2 * WHEEL_RADIUS)) * omega
    w_left  = v / WHEEL_RADIUS - (AXLE_LENGTH / (2 * WHEEL_RADIUS)) * omega
    
    motor(w_left, w_right)
    
    # Debug print
    print(f"r:{r:.3f} | alpha:{alpha*180/math.pi:.2f} | lambda:{lam*180/math.pi:.2f} | theta:{theta*180/math.pi:.2f} | v:{v:.3f} | omega:{omega:.3f}")