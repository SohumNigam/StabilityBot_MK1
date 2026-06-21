#include "motors.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#define PWM_FREQ       20000
#define PWM_RESOLUTION LEDC_TIMER_8_BIT

esp_err_t motor_init(motor_t *motor)
{
    if (motor == NULL)
        return ESP_ERR_INVALID_ARG;
        
    gpio_reset_pin(motor->pin_a);
    gpio_reset_pin(motor->pin_b);
    gpio_reset_pin(motor->en_pin);
        
    gpio_set_direction(motor->pin_a, GPIO_MODE_OUTPUT);
    gpio_set_direction(motor->pin_b, GPIO_MODE_OUTPUT);

    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = motor->timer,
        .duty_resolution  = LEDC_TIMER_8_BIT,
        .freq_hz          = 20000,
        .clk_cfg          = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .gpio_num   = motor->en_pin,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = motor->channel,
        .timer_sel  = motor->timer,
        .duty       = 0,
        .hpoint     = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));




    motor->speed = 0;

    return ESP_OK;
}
esp_err_t set_motor_speed(motor_t *motor, int speed)
{
    if (!motor)
        return ESP_ERR_INVALID_ARG;

    if (speed > 255) speed = 255;
    if (speed < -255) speed = -255;

    int dir_a = 0;
    int dir_b = 0;

    if (speed > 0)
    {
        dir_a = 1;
        dir_b = 0;
    }
    else if (speed < 0)
    {
        dir_a = 0;
        dir_b = 1;
        speed = -speed;
    }
    else
    {
        dir_a = 0;
        dir_b = 0;
    }

    gpio_set_level(motor->pin_a, dir_a);
    gpio_set_level(motor->pin_b, dir_b);
    

    ledc_set_duty(LEDC_LOW_SPEED_MODE, motor->channel, speed);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, motor->channel);

    motor->speed = (dir_a ? speed : -speed);

    return ESP_OK;
}

esp_err_t motor_stop(motor_t *motor)
{
    return set_motor_speed(motor, 0);
}