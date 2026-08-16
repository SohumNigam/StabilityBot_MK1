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
![signal dropout](Assets/image-1.png)

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
Next steps will be to also finalize magnetic rotary encoder PCB and implement a cad mounting solution. To actually enable Full 4 state LQR control. I have high hpes for the stability of the final robot. Should be significantly more effective than PID.

**Total time spent: 1 hours**


# Aug 16: Putting together a tentative BOM, finalizing schematic, and going down the LQR rabbit hole

While the project is still in progress and not all parts are finalized, I still chose to put together a rough BOM to start to have an idea of what is pending and what can move forward.

![alt text](Assets/image-9.png)


Was a pain but I found the exact model of the esp32 board I used without even having it on me (Im on vacation) for the PCB. Also has been added to BOM

![alt text](Assets/image-10.png)

With this newfound knowledge, I started finalising the Schematic now that parts are confirmed (just not passive components yet). Super messy rign now will make sure to do LOTS of double checking because I rlly want this to work first try or I might not finish the project in time.

Finalized the schematic!

![alt text](Assets/image-11.png)

Still needs to be revieved and double checked but should be moving on the layout in a day or 2.

Also finished encoder PCB design.

![alt text](Assets/image-12.png)
![alt text](Assets/image-13.png)


Started working on LQR formulas derivation and simulation. Lots of juicy high level math.

Basically going to just take my notes here and hope that you guys can understand.

Consider the robot as a linear system modeled using a state space equation.

$$\dot{x} = Ax + Bu$$


This is a common method of mathematically modeling a system where...

$\dot{x}$ is the drivative of the state vector of the system. In this case $x = [\theta, \dot{\theta}, x, \dot{x}]$ being the angle, position, and their derivatives.

$A$ is the matrix defining how the system reacts based on the laws of physics. Basically how fast will it fall/roll away if we do nothing.

$B$ is the matrix representing how the system responds to control effort. What will happen to the system given motor response $u$.

We will define $A$ and $B$ using newtonian physics.

The goal of LQR is to find the optimal value for $u$ given any state $x$.

To define optimal we will need a cost function.

$$J = \int_{0}^{\infty} (x^T Q x + u^T R u) dt$$

$x^T Q x$ where Q is a symmetric positive semidefinite matrix works out to be... 
$$(Q_1x_1)^2+(Q_2x_2)^2+(Q_3x_3)^2+(Q_4x_4)^2$$

This is simply the normalized weighted sum of the deviation $x$.

Similarly,
$u^T T u$ where T is a symmetric positive definite matrix represents the the control effort (how much work the system does).

So this giant integral
$$J = \int_{0}^{\infty} (x^T Q x + u^T R u) dt$$
works out to how much work + how much error summed over time. Or in other words: If we let the system run forever, how optimal would it be. Smaller = better.

The nice part here is we get to define $Q$ and $R$ to mathmaticall define how much we care about fixing error and control effor respectivley.

So what we want is

$$min[J = \int_{0}^{\infty} (x^T Q x + u^T R u) dt]$$

Now if you did not already know.

You can find the minimum of a quadratic function $Ax^2 + Bx + C$ by finding the derivative and setting it to 0.

The same idea generalized to matricies produces the Algebraic Ricatti Equation:

$ATP+PA−PBR−1BTP+Q=0$

where the only unknown is $P$

This is basically *impossible* to derive and solve by hand but that's what computers are for.

After solving we get the optimal gain matric $K$ in the form.

$$K=R−1BTP$$

In the python script this will be

```python
scipy.linalg.solve_continuous_are(A, B, Q, R) 
K = inv(R) @ B.T @ P
```

Up until this point everything has been computed offline. Now all the MCU has to do live is...
$$u = -Kx$$

Any questions?

I hope this made some sense because writing this out definetly helped me understand how im going to implement all this in the near future.

Feel free to ask if anything is unclear and I will try to answer you. Just keep in mind I am NOT a professor, just unemployed.

This was a very description of how LQR works. If this sparked your curiosity then check out this video from someone who knows what they're talking about.

https://youtu.be/wEevt2a4SKI?si=k8IcvXmKzXowunIT

**Total time spent: 5 hours**
