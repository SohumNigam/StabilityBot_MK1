---
title: "StabilityBot MK1"
author: "sohum"
description: "Self-balancing inverted pendulum robot on ESP32-S3, working toward full 4-state LQR control with a custom PCB"
created_at: "2026-06-10"
---

# Before logging began: Initial concept and first assembly

Built the initial concept with basic IMU-to-motor response firmware
(no PID, no testing chassis yet). Hit the first major bug: the ESP32
boot-looped due to noise on boot GPIO pin 14 — solved by migrating
motor control to pin 25. Iterated quickly on a testing chassis design
in Onshape, then completed the first full assembly of the robot.

![logic diagram](Assets/image.png)

**Total time spent: 8 hours**

# June 10: First balancing test, MPU6050 signal dropouts

Goal: gather initial test results before upgrading to more reliable hardware.

Ran the first balancing test and tried to tune PID. The MPU6050
sensor signal repeatedly cut out, causing laggy, intermittent motor
response. Impossible to tune or balance as a result.

![signal dropout](Assets/image-1.png)

Suspected cause: the current breadboard prototype and jumper wire
connections are introducing mechanical and electrical noise, causing
signal drops.

Next step: design and order a BMI320 breakout board and solder
connections to the ESP32 to eliminate testing noise.

**Total time spent: 2 hours**

# June 14: Noise mitigation on the current setup

Goal: investigate what can be done to minimize noise with the
current setup, with intent to carry the findings into the future
design even after the hardware changes.

- Twisted signal wires with GND to reduce noise
- Tuned the complementary filter to smooth the angle estimate

Both changes lowered high-frequency noise and smoothed robot motion.
Signal dropouts still persist, further supporting the need for a
custom PCB.

before:
![noisy imu](Assets/image-2.png)

after:
![smooth imu](Assets/image-3.png)


**Total time spent: 1 hour**

# June 20: Chasing a software cause for signal dropout

Goal: investigate potential software causes for signal dropout.

- Disabled the motors
- Printed IMU readings to the serial console for debugging
- Discovered occasional watchdog timer triggers
- Added a delay to the control loop

The added delay *significantly* reduced signal dropouts during
balancing, making tuning noticeably easier. Dropouts still occur
sparsely but are much more under control now, allowing balancing
attempts to come much closer to succeeding.

**Total time spent: 2 hours**

# July 19: Custom BMI320 driver and board complete, PCB subsystems in design

![alt text](Assets/image-4.png)

![alt text](Assets/image-5.png)

BMI320 board debugging is complete and fully functioning. Root cause
was a disconnected VDDIO pin, which prevented the chip from powering
up properly. BMI320 driver firmware is nearing completion and cleanup.

PCB subsystem test boards are now in design (MCU, motor driver,
encoders). Encoders will be used to implement full 4-state LQR control.


Next: reach a fully functioning, fully integrated PCB and begin LQR
implementation.

LQR is short for Linear Quadratic Regulator and is the industry standard for optimal control in state space systems.

Credit to this paper from MIT that helped me learn it:
https://underactuated.mit.edu/lqr.html

**Total time spent: 4 hours**

# Aug 13: PCB subsystems combined into one schematic, Final PCB in design

Combined PCB subsystems into one schematic. Had to resolve a few pin conflicts

BMI320 IMU

![alt text](Assets/image-0.png)

Motor driver
![alt text](Assets/image-6.png)


Will move on to finalizing and reviweing connections. The start routing the final PCB.
Next steps will be to also finalize magnetic rotary encoder PCB and implement a cad mounting solution. To actually enable Full 4 state LQR control.

**Total time spent: 1 hours**

