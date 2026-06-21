# Stability Bot MK1
*Currently in prototype phase*

Classic 2-wheeled robot stabilised with PID IMU feedback.

## Showcase
Webpage coming soon at:
https://sohumnigam.github.io


# Current features
- Reactive motor response
- IMU sensor fusion
- PID


## Hardware Overview & Potential Future Upgrades
- ESP32 Devboard --> Custom PCB
- MPU6050 IMU module --> BOSCH BMI320
- L293D dual H-Bridge motor driver
- Standard yellow DC geared motors
- ELEGOO Battery Pack --> High discharge lipo + Boost converter
- 3D printed chassis

## Software Overview

### FreeRTOS control task
- IMU readings
- PID calculations
- Motor PWM control

### FreeRTOS Web interface handler (Not implemented yet)
- Digital logging
- Mobile control
- Remote tuning

## Quick Start
- Clone the project to yout ESP-IDF project directory
- Wire the ESP32 and other components to match the code
- Flash and potentially tune PID if needed






# Dev Log

## Short summary of development before logging began:

- Initial concept with basic IMU --> motor response firmware (No PID, no testing body)
- First major bug: ESP32 boot-loop due to noise on boot GPIO pin #14 --> solved by migrating motor control to pin 25
- Rapid iterative design of initial testing chassis in onshape
- Initial full assembly of the robot


## 06/10/2026:
Goal: Gather initial test results before upgrading to more reliable harware

- conducted initial balancing test and attempted to tune PID
- MPU6050 sensor signal repeatedly cut out
- caused laggy intermitent motor response
- impossible to tune and balance

Suspected cause: Current breadboard connection prototype and jumper wire connections introduce mechanical and electrical noise causing signal drops

Next step: Design and order BMI320 breakout board and solder connections to ESP32 to eliminate testing noise

## 06/14/2026
Goal: Investigate what can be done to minimze noise with the current setup with intent to optimize the future design even after harware changes.

- Twisted signal wires with GND to reduce noise
- Tuned complementary filter to smoothe angle estimate

Both changes lowered high frequency noise and smoothed robot motion. Signal dropouts still persist further suppourting the need for a custom PCB.