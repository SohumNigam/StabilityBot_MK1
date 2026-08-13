# StabilityBot MK1

*Currently in prototype phase*

A self-balancing two-wheeled robot — an inverted pendulum stabilized
by IMU feedback and PID motor control, being built up toward full
4-state LQR control on a custom PCB.

**Full write-up / showcase:**
[sohumnigam.github.io/projects/stability_bot_MK1.html](https://sohumnigam.github.io/projects/stability_bot_MK1.html)

## Why I built this

I wanted a project that went past the usual "balance a robot with a
PID loop" hobby build — something that let me go deep on control
theory (state-space modeling, LQR, Kalman filtering), embedded
firmware, and PCB design all at once, and end up with a portfolio
piece that shows that depth rather than just a demo video. The
long-term goal is a robot that balances using a full 4-state LQR
controller (θ, θ̇, x, ẋ) on hardware I designed myself, with a
classical-vs-learned comparison against a reinforcement learning
controller down the line.

## Photos

<!-- TODO: add these before submission -->
- [ ] Full assembly photo(s)
- [ ] PCB photo (once fabbed)
- [ ] Schematic screenshot
- [ ] Wiring diagram for current breadboard/soldered prototype

## Current Features

- Active motor balancing response
- IMU sensor fusion (complementary filter, Kalman filter in progress)
- PID control loop
- Live web dashboard (WebSocket telemetry: pitch, motor output, PID
  tuning sliders) served from the ESP32 in SoftAP mode

## Hardware Overview

| Component | Current | Planned Upgrade |
|---|---|---|
| Compute | ESP32 Devboard | Custom ESP32-S3 PCB |
| IMU | MPU6050 | Bosch BMI320 (SPI) |
| Motor driver | L293D | TB6612FNG |
| Motors | Standard yellow DC geared motors | — |
| Battery | ELEGOO battery pack | High-discharge 2S LiPo |
| Chassis | 3D printed | — |
| Encoders | None | AS5600 / AS5600L (for 4-state LQR) |

## Software Overview

**FreeRTOS control task**
- IMU readings
- PID calculations (LQR planned once encoders are integrated)
- Motor PWM control

**FreeRTOS web interface task**
- Digital logging
- Mobile control
- Remote PID tuning

## How to Assemble

<!-- TODO: fill in with real steps once chassis/wiring is finalized -->
1. 3D print the chassis (files: `/cad`)
2. Mount motors and wheels
3. Wire IMU, motor driver, and battery to the ESP32 per the wiring
   diagram above
4. Secure battery pack and IMU to minimize vibration noise

## How to Flash

1. Clone this repo into your ESP-IDF project directory
2. Confirm your wiring matches the pin assignments in `main/`
3. Build and flash with `idf.py build flash monitor`
4. Connect to the robot's WebSocket dashboard to tune PID live

## Bill of Materials

<!-- TODO: fill in real part numbers, sources, and costs -->
| Part | Qty | Source | Cost |
|---|---|---|---|
| ESP32 Devboard | 1 | | |
| MPU6050 | 1 | | |
| L293D | 1 | | |
| DC geared motors | 2 | | |
| Battery pack | 1 | | |
| 3D printed chassis | 1 | | |

## Known Issues

- IMU signal occasionally drops out on the current breadboard
  prototype — traced to noise from jumper wire connections; largely
  mitigated with twisted GND pairs and a control-loop delay, but not
  fully eliminated. Root fix is the move to a soldered BMI320 board
  and eventually the custom PCB.
- 4-state LQR not yet running on hardware — pending encoder
  integration.

## Credits

<!-- TODO: list any open-source firmware, libraries, or references used -->
- Built on [ESP-IDF](https://github.com/espressif/esp-idf) (Espressif)