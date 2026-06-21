#pragma once

#include "esp_err.h"


typedef struct {

    int channel; // LEDC channel for PWM control
    int timer; // LEDC timer for PWM control
    
    int pin_a;
    int pin_b;
    int en_pin;

    int speed; // current speed, -255 to 255

} motor_t;





esp_err_t motor_init(motor_t *motor);
esp_err_t set_motor_speed(motor_t *motor, int speed);
esp_err_t motor_stop(motor_t *motor);