#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_sr.h"

static const char *TAG = "VOICE_ROVER";

// 电机引脚定义
#define MOTOR_IN1 GPIO_NUM_4
#define MOTOR_IN2 GPIO_NUM_5
#define MOTOR_IN3 GPIO_NUM_6
#define MOTOR_IN4 GPIO_NUM_7
#define MOTOR_PWM_CH1 LEDC_CHANNEL_0
#define MOTOR_PWM_CH2 LEDC_CHANNEL_1

typedef enum {
    MODE_OFFLINE_SR,
    MODE_WIFI_ASR,
    MODE_BLE_CTRL
} work_mode_t;
static work_mode_t g_mode = MODE_OFFLINE_SR;

void pwm_init(void)
{
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t ch1 = {
        .channel = MOTOR_PWM_CH1,
        .duty = 0,
        .gpio_num = MOTOR_IN1,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0,
    };
    ledc_channel_config(&ch1);

    ledc_channel_config_t ch2 = {
        .channel = MOTOR_PWM_CH2,
        .duty = 0,
        .gpio_num = MOTOR_IN3,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0,
    };
    ledc_channel_config(&ch2);
}

void motor_run(int duty)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, MOTOR_PWM_CH1, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, MOTOR_PWM_CH1);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, MOTOR_PWM_CH2, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, MOTOR_PWM_CH2);
}

void sr_callback(esp_sr_result_t *res)
{
    if(res->state == ESP_SR_WAKEUP) {
        ESP_LOGI(TAG, "唤醒成功");
    }else if(res->state == ESP_SR_CMD) {
        ESP_LOGI(TAG, "识别指令:%s", res->cmd);
        if(strcmp(res->cmd, "forward") == 0) {
            motor_run(512);
        }else if(strcmp(res->cmd, "stop") == 0) {
            motor_run(0);
        }
    }
}

void app_main(void)
{
    pwm_init();
    esp_sr_config_t sr_cfg = {
        .callback = sr_callback
    };
    esp_sr_init(&sr_cfg);
    ESP_LOGI(TAG, "ESP32-S3 语音小车启动");
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
