#include <stdio.h>
#include <esp_log.h>
#include "driver/i2c_master.h"
#include "esp_timer.h"

#include "mpu6050.h"
#include "motors.h"
#include "telemetry.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "esp_spiffs.h"


#define WIFI_SSID "RobotAP"
#define WIFI_PASS "robot1234"

#define TAG "MAIN"

void wifi_init_softap(void) {
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t ap_config = {
        .ap = {
            .ssid          = WIFI_SSID,
            .password      = WIFI_PASS,
            .ssid_len      = strlen(WIFI_SSID),
            .max_connection = 2,
            .authmode      = WIFI_AUTH_WPA2_PSK,
        },
    };
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &ap_config);
    esp_wifi_start();
    // Connect to http://192.168.4.1 on your phone/laptop
}


void spiffs_init(void) {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/web",
        .partition_label = "storage",
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPIFFS mount failed: %s", esp_err_to_name(ret));
    }
}

//i2c
#define I2C_MASTER_SDA 21
#define I2C_MASTER_SCL 22
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000

#define RAD_TO_DEG 57.2957795f

#define motor_min_speed 172 // minimum speed to overcome static friction

float Kp = 1.2f;
float Ki = 0.0f;
float Kd = 0.0f;


motor_t motor_left = {
    .channel = 0,
    .timer = 0,
    .pin_a = 26,
    .pin_b = 27,
    .en_pin = 14
};

motor_t motor_right = {
    .channel = 1,
    .timer = 1,
    .pin_a = 25,
    .pin_b = 33,
    .en_pin = 32
};






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


static int64_t last_time = 0;

float target_angle = 0.0f;
float prev_err = 0.0f;

float total_error = 0.0f; // for integral term

int motor_speed;

void control_task(void *arg){

    
    while (1) {
        



        //IMU calclulations
        int64_t now = esp_timer_get_time();
        float dt = (now - last_time) / 1000000.0f; // convert to seconds
        last_time = now;

        if (mpu6050_read_converted(I2C_MASTER_NUM, &mpu6050_data, dt) != ESP_OK) {

            ESP_LOGE(TAG, "Failed to read MPU6050 data");
                vTaskDelay(pdMS_TO_TICKS(100));

        }else {
            //ESP_LOGI(TAG, "Pitch: %.2f°, Roll: %.2f°, Yaw: %.2f°", mpu6050_data.pitch, mpu6050_data.roll, mpu6050_data.yaw);
            
        }


        
        //PID control calculations
        float err = target_angle - mpu6050_data.pitch;
        total_error += err * dt;
        
        // Reset integral term if error changes sign
        if ((err > 0 && prev_err < 0) || (err < 0 && prev_err > 0)) {
            total_error = 0.0f;
        }

        float offset_p = Kp * err;
        float offset_i = Ki * total_error;
        float offset_d = Kd * (err - prev_err) / dt;

        // Anti-windup: Clamp the integral term
        if (offset_i > 5.0f) offset_i = 5.0f;
        if (offset_i < -5.0f) offset_i = -5.0f;



        float total_offset = offset_p + offset_i + offset_d;

        prev_err = err;
        //ESP_LOGI(TAG, "Offset: P=%.2f, I=%.2f, D=%.2f, Total=%.2f", offset_p, offset_i, offset_d, total_offset);



        //motor output 
        if (abs((int)err) > 70){
            total_offset = 0;
        }


        if(total_offset < 0) {
            motor_speed = -1 * (motor_min_speed) + (int)total_offset; // reverse direction for negative offset
        }else if(total_offset > 0) {
            motor_speed = motor_min_speed + (int)total_offset;
        }else{
            motor_speed = 0;
        }

        if (motor_speed > 255) motor_speed = 255;
        if (motor_speed < -255) motor_speed = -255;


        //ESP_LOGI(TAG, "Motor speed: %d", motor_speed);

        set_motor_speed(&motor_left, motor_speed);
        set_motor_speed(&motor_right, motor_speed);




        //telemetry update  
        if (xSemaphoreTake(g_telemetry_mutex, 0) == pdTRUE) {
            g_telemetry.pitch        = mpu6050_data.pitch;
            g_telemetry.err          = err;
            g_telemetry.p_term       = offset_p;
            g_telemetry.i_term       = offset_i;
            g_telemetry.d_term       = offset_d;
            g_telemetry.total_offset = total_offset;
            g_telemetry.motor_speed  = motor_speed;
            g_telemetry.dt_ms        = dt * 1000.0f;
            xSemaphoreGive(g_telemetry_mutex);
        }
    
    }
}


void telemetry_task(void *arg) {
    while (1) {
        ws_broadcast_telemetry();
        vTaskDelay(pdMS_TO_TICKS(50));  // 20 Hz broadcast
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
    
    
    motor_init(&motor_left);
    motor_init(&motor_right);

    spiffs_init();

    wifi_init_softap();
    ws_server_start();
    
    xTaskCreatePinnedToCore(control_task, "control_task", 4096, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(telemetry_task, "telemetry", 4096, NULL, 3, NULL, 1);
    
    


}
