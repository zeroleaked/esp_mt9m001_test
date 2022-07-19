#include <esp_log.h>

#include "camera_pins.h"
#include "esp_camera.h"

char TAG[] = "main";

void app_main() {
    ESP_LOGI("APP_MAIN", "HELLO WORLD");
    esp_err_t err;
    camera_config_t camera_config;
    camera_config.ledc_channel = LEDC_CHANNEL_0;
    camera_config.ledc_timer = LEDC_TIMER_0;
    camera_config.pin_d0 = CONFIG_D0;
    camera_config.pin_d1 = CONFIG_D1;
    camera_config.pin_d2 = CONFIG_D2;
    camera_config.pin_d3 = CONFIG_D3;
    camera_config.pin_d4 = CONFIG_D4;
    camera_config.pin_d5 = CONFIG_D5;
    camera_config.pin_d6 = CONFIG_D6;
    camera_config.pin_d7 = CONFIG_D7;
    camera_config.pin_xclk = CONFIG_XCLK;
    camera_config.pin_pclk = CONFIG_PCLK;
    camera_config.pin_vsync = CONFIG_VSYNC;
    camera_config.pin_href = CONFIG_HREF;
    camera_config.pin_sscb_sda = CONFIG_SDA;
    camera_config.pin_sscb_scl = CONFIG_SCL;
    camera_config.pin_reset = CONFIG_RESET;
    camera_config.pin_pwdn = CONFIG_PWDN;
    camera_config.xclk_freq_hz = CONFIG_XCLK_FREQ;
    camera_config.fb_count = CONFIG_FB_COUNT;

    camera_model_t out_camera_model;
    err = camera_probe(&camera_config, &out_camera_model);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera probe failed with error 0x%x", err);
        return;
    }
}