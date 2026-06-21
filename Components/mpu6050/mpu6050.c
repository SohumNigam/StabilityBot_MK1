#include "mpu6050.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "math.h"




static const char *TAG = "MPU6050";



#define ACCEL_SCALE 16384.0f   // LSB/g for ±2g full scale
#define G 9.80665f             // m/s²
#define GYRO_SCALE 131.0f      // LSB/(°/s) for ±250°/s full scale
#define SAMPLE_COUNT 500

#define RAD_TO_DEG 57.2957795131


esp_err_t mpu6050_write_byte(i2c_port_t i2c_num, uint8_t reg, uint8_t data) {
    uint8_t buf[2] = {reg, data};
    return i2c_master_write_to_device(i2c_num, MPU6050_ADDR, buf, 2, 1000 / portTICK_PERIOD_MS);
}

static esp_err_t mpu6050_read_bytes(i2c_port_t i2c_num, uint8_t reg, uint8_t *data, size_t len) {
    uint8_t reg_buf[1] = {reg};
    return i2c_master_write_read_device(
        i2c_num,
        MPU6050_ADDR,
        reg_buf,
        1,
        data,
        len,
        1000 / portTICK_PERIOD_MS
    );
}


esp_err_t mpu6050_init(i2c_port_t i2c_num, mpu6050_data_t *data) {
    esp_err_t ret;

    // Wake the MPU6050
    ret = mpu6050_write_byte(i2c_num, 0x6B, 0x00); // PWR_MGMT_1 = 0
    if (ret != ESP_OK) return ret;

    // Small delay to stabilize
    vTaskDelay(pdMS_TO_TICKS(50));

    // Set sample rate
    ret = mpu6050_write_byte(i2c_num, 0x19, 0x07); // SMPLRT_DIV = 7
    if (ret != ESP_OK) return ret;

    // Configure DLPF
    ret = mpu6050_write_byte(i2c_num, 0x1A, 0x03); // CONFIG = 3
    if (ret != ESP_OK) return ret;

    // Set gyro full-scale ±250 °/s
    ret = mpu6050_write_byte(i2c_num, 0x1B, 0x00); // GYRO_CONFIG
    if (ret != ESP_OK) return ret;

    // Set accel full-scale ±2g
    ret = mpu6050_write_byte(i2c_num, 0x1C, 0x00); // ACCEL_CONFIG
    if (ret != ESP_OK) return ret;

    data->pitch_g = 0;
    data->roll_g = 0;
    data->yaw_g = 0;

    ESP_LOGI(TAG, "Initialization complete");

    return ESP_OK;
}

esp_err_t mpu6050_read_raw(i2c_port_t i2c_num, mpu6050_data_t *data) {
    uint8_t buf[14];
    esp_err_t ret = mpu6050_read_bytes(i2c_num, MPU6050_ACCEL_XOUT_H, buf, 14);

    if (ret != ESP_OK) return ret;

    data->accel_x = (buf[0] << 8) | buf[1];
    data->accel_y = (buf[2] << 8) | buf[3];
    data->accel_z = (buf[4] << 8) | buf[5];
    data->temp    = (buf[6] << 8) | buf[7];
    data->gyro_x  = (buf[8] << 8) | buf[9];
    data->gyro_y  = (buf[10] << 8) | buf[11];
    data->gyro_z  = (buf[12] << 8) | buf[13];

    return ESP_OK;
}


esp_err_t mpu6050_calibrate(i2c_port_t i2c_num, mpu6050_data_t *data){

    ESP_LOGI(TAG, "Calibrating...");
    
    int32_t gyro_x_offset = 0, gyro_y_offset = 0, gyro_z_offset = 0;
    int32_t accel_x_offset = 0, accel_y_offset = 0, accel_z_offset = 0;

    for (int i = 0; i <= SAMPLE_COUNT; i++){
        mpu6050_read_raw(i2c_num, data);

        gyro_x_offset += data->gyro_x;
        gyro_y_offset += data->gyro_y;
        gyro_z_offset += data->gyro_z;

        accel_x_offset += data->accel_x;
        accel_y_offset += data->accel_y;
        accel_z_offset += data->accel_z;

        vTaskDelay(pdMS_TO_TICKS(5));

    }

    data->accel_x_offset = accel_x_offset / SAMPLE_COUNT;
    data->accel_y_offset = accel_y_offset / SAMPLE_COUNT;
    data->accel_z_offset = (accel_z_offset / SAMPLE_COUNT) - 16384;

    data->gyro_x_offset = gyro_x_offset / SAMPLE_COUNT;
    data->gyro_y_offset = gyro_y_offset / SAMPLE_COUNT;
    data->gyro_z_offset = gyro_z_offset / SAMPLE_COUNT;

    ESP_LOGI(TAG, "Calibrated succesfully with %d samples", SAMPLE_COUNT);

    return ESP_OK;

}




esp_err_t mpu6050_who_am_i(i2c_port_t i2c_num, uint8_t *id) {
    return mpu6050_read_bytes(i2c_num, MPU6050_WHO_AM_I, id, 1);
}


esp_err_t mpu6050_read_converted(i2c_port_t i2c_num, mpu6050_data_t *data, float dt) {
    uint8_t reg = MPU6050_ACCEL_XOUT_H; // starting register
    uint8_t raw[14];
    esp_err_t ret = i2c_master_write_read_device(
        i2c_num,
        MPU6050_ADDR,
        &reg,
        1,
        raw,
        14,
        1000 / portTICK_PERIOD_MS
    );
    if (ret != ESP_OK) return ret;


    // Combine high/low bytes
    data->accel_x = (int16_t)((raw[0] << 8) | raw[1]);
    data->accel_y = (int16_t)((raw[2] << 8) | raw[3]);
    data->accel_z = (int16_t)((raw[4] << 8) | raw[5]);
    data->temp    = (int16_t)((raw[6] << 8) | raw[7]);
    data->gyro_x  = (int16_t)((raw[8] << 8) | raw[9]);
    data->gyro_y  = (int16_t)((raw[10] << 8) | raw[11]);
    data->gyro_z  = (int16_t)((raw[12] << 8) | raw[13]);

    // Apply calibration offsets
    data->accel_x -= data->accel_x_offset;
    data->accel_y -= data->accel_y_offset;
    data->accel_z -= data->accel_z_offset;
    data->gyro_x  -= data->gyro_x_offset;
    data->gyro_y  -= data->gyro_y_offset;
    data->gyro_z  -= data->gyro_z_offset;

    // Convert to physical units
    data->accel_x_g = data->accel_x / ACCEL_SCALE;
    data->accel_y_g = data->accel_y / ACCEL_SCALE;
    data->accel_z_g = data->accel_z / ACCEL_SCALE;

    data->accel_x_ms2 = data->accel_x_g * G;
    data->accel_y_ms2 = data->accel_y_g * G;
    data->accel_z_ms2 = data->accel_z_g * G;

    data->gyro_x_dps = data->gyro_x / GYRO_SCALE;
    data->gyro_y_dps = data->gyro_y / GYRO_SCALE;
    data->gyro_z_dps = data->gyro_z / GYRO_SCALE;

    data->temp = (data->temp / 340.0f) + 36.53f;

    // Calculate accelerometer angles
    float roll_a  = atan2f(data->accel_y_g, data->accel_z_g) * RAD_TO_DEG;
    float pitch_a = atan2f(-data->accel_x_g, sqrtf(data->accel_y_g * data->accel_y_g + data->accel_z_g * data->accel_z_g)) * RAD_TO_DEG;

    // Complementary filter fusion
    const float alpha = 0.92f; // gyro weight
    data->pitch_g += data->gyro_y_dps * dt;
    data->roll_g  += data->gyro_x_dps * dt;
    data->yaw_g   += data->gyro_z_dps * dt;

    // fuse with accel
    data->pitch = alpha * data->pitch_g + (1 - alpha) * pitch_a;
    data->roll  = alpha * data->roll_g  + (1 - alpha) * roll_a;
    data->yaw   = data->yaw_g;

    // store fused angle for next iteration
    data->pitch_g = data->pitch;
    data->roll_g  = data->roll;



    return ESP_OK;
}
