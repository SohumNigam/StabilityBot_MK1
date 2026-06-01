#include <stdio.h>
#include <esp_log.h>
#include "driver/i2c_master.h"
#include "esp_timer.h"

#include "mpu6050.h"

#define TAG "MAIN"


//i2c
#define I2C_MASTER_SDA 21
#define I2C_MASTER_SCL 22
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000



void i2c_master_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA,
        .scl_io_num = I2C_MASTER_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}



mpu6050_data_t mpu6050_data;
#define RAD_TO_DEG 57.2957795f


static int64_t last_time = 0;

void imu_task(void *arg){

    
    while (1) {
        
        int64_t now = esp_timer_get_time();
        float dt = (now - last_time) / 1000000.0f; // convert to seconds
        last_time = now;

        if (mpu6050_read_converted(I2C_MASTER_NUM, &mpu6050_data, dt) != ESP_OK) {

            ESP_LOGE(TAG, "Failed to read MPU6050 data");
                vTaskDelay(pdMS_TO_TICKS(100));

        }else {
            ESP_LOGI(TAG, "Pitch: %.2f°, Roll: %.2f°, Yaw: %.2f°", mpu6050_data.pitch, mpu6050_data.roll, mpu6050_data.yaw);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    
    }
}


void app_main(void)
{


    i2c_master_init();
    mpu6050_init(I2C_MASTER_NUM, &mpu6050_data);
    mpu6050_calibrate(I2C_MASTER_NUM, &mpu6050_data);

    //who am i (i2c sanity check)
    uint8_t whoami;
    mpu6050_who_am_i(I2C_MASTER_NUM, &whoami);
    ESP_LOGI(TAG, "WHO_AM_I = 0x%02X", whoami);


    xTaskCreate(imu_task, "imu_task", 4096, NULL, 5, NULL);


}
