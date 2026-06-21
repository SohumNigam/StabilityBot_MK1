#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

typedef struct {
    float pitch;
    float err;
    float p_term;
    float i_term;
    float d_term;
    float total_offset;
    int   motor_speed;
    float dt_ms;
} telemetry_t;

extern telemetry_t g_telemetry;
extern SemaphoreHandle_t g_telemetry_mutex;

void ws_server_start(void);
void ws_broadcast_telemetry(void);
