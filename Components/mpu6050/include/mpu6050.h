#pragma once

#include "driver/i2c.h"
#include "esp_err.h"

#define MPU6050_ADDR       0x68
#define MPU6050_WHO_AM_I   0x75
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B



typedef struct {
    int16_t accel_x, accel_y, accel_z; // raw values
    int16_t gyro_x, gyro_y, gyro_z;    // raw values
    int16_t temp;                      // raw temperature

    // converted values
    float accel_x_g, accel_y_g, accel_z_g;      // in gs
    float accel_x_ms2, accel_y_ms2, accel_z_ms2; // in m/s²

    float gyro_x_dps, gyro_y_dps, gyro_z_dps;    // in °/s
    
    float pitch_g, roll_g, yaw_g;
    float pitch, roll, yaw;

    

    //offsets
    int32_t gyro_x_offset;
    int32_t gyro_y_offset;
    int32_t gyro_z_offset;
    int32_t accel_x_offset;
    int32_t accel_y_offset;
    int32_t accel_z_offset;

} mpu6050_data_t;

esp_err_t mpu6050_init(i2c_port_t i2c_num, mpu6050_data_t *data);
esp_err_t mpu6050_calibrate(i2c_port_t i2c_num, mpu6050_data_t *data);
esp_err_t mpu6050_who_am_i(i2c_port_t i2c_num, uint8_t *id);
esp_err_t mpu6050_read_raw(i2c_port_t i2c_num, mpu6050_data_t *data);
esp_err_t mpu6050_read_converted(i2c_port_t i2c_num, mpu6050_data_t *data, float dt);
