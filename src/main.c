#include <esp_log.h>
#include <string.h>

#include "camera_pins.h"
#include "esp_camera.h"
#include "img_converters.h"

char TAG[] = "main";

static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sscb_sda = CAM_PIN_SIOD,
    .pin_sscb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    // .xclk_freq_hz = 20000000,
    .xclk_freq_hz = 5000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_GRAYSCALE, //YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_HVGA,    //QQVGA-UXGA Do not use sizes above QVGA when not JPEG

    .fb_count = 1,       //if more than one, i2s runs in continuous mode. Use only with JPEG
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    .fb_location = CAMERA_FB_IN_DRAM
};

#define SUBFRAME_WIDTH 480
#define SUBFRAME_HEIGHT 320

#include "driver/uart.h"
#include "driver/gpio.h"

#define UART_BUFFER_SIZE (1024)

esp_err_t init_uart() {
    uart_config_t uart_config = {
        // .baud_rate = 115200,
        .baud_rate = 1000000,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 17, 16, -1, -1));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, UART_BUFFER_SIZE * 2, 0, 0, NULL, 0));

    return ESP_OK;
}

esp_err_t send_frame_info(uint16_t width, uint16_t height) {
    uint8_t start_header[5] = {0};
    start_header[0] = 0x01;
    start_header[1] = width >> 8;
    start_header[2] = width & 0xFF;
    start_header[3] = height >> 8;
    start_header[4] = height & 0xFF;

    uart_write_bytes(UART_NUM_2, (const uint8_t *) &start_header, 5);

    return ESP_OK;
}

esp_err_t send_frame_buffer(uint8_t * image_buffer, uint16_t length) {
    uint8_t buffer[1024] = {0};
    buffer[0] = 0x02;
    buffer[1] = length >> 8;
    buffer[2] = length & 0xFF;
    memcpy(buffer+3, image_buffer, length);

    uart_write_bytes(UART_NUM_2, (const uint8_t *) &buffer, length+3);
    return ESP_OK;
}

esp_err_t send_whole_frame_buffer(camera_fb_t * pic) {
    send_frame_info(pic->width,pic->height);
    uint16_t pixels_per_full_batch = UART_BUFFER_SIZE-3;
    uint16_t full_batch = (pic->len)/pixels_per_full_batch;
    for (int i =  0; i < full_batch; i++) {
        send_frame_buffer(pic->buf+(i*pixels_per_full_batch), pixels_per_full_batch);
    }
    send_frame_buffer(
        pic->buf+(full_batch*pixels_per_full_batch),
        (pic->len)%pixels_per_full_batch);

    return ESP_OK;
}

void app_main() {
    ESP_LOGI("APP_MAIN", "HELLO WORLD");

    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);


    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera Init Failed");
        return;
    }
    else {
        ESP_LOGI(TAG, "Camera Init Ok");
    }

    init_uart();
    
    sensor_t *s = esp_camera_sensor_get();

    for (uint16_t rowstart=0xC; rowstart<1024; rowstart+=SUBFRAME_HEIGHT) {
        s->set_rowstart(s, rowstart);
        
        for (uint16_t colstart = 0x14; colstart<1280; colstart+=SUBFRAME_WIDTH) {
            s->set_colstart(s, colstart);


            ESP_LOGI(TAG, "Taking picture (%d, %d)...", rowstart, colstart);
            camera_fb_t *pic = esp_camera_fb_get();
            // use pic->buf to access the image
            ESP_LOGI(TAG, "Picture taken! Its size was: %zu bytes", pic->len);
            // for (int i=0; i<96; i++) {
            //     ESP_LOGI(TAG, "0x%02X", pic->buf[i]);
            // }
            send_whole_frame_buffer(pic);
            esp_camera_fb_return(pic);

        }

    }





    // ESP_LOGI(TAG, "Taking picture...");
    // pic = esp_camera_fb_get();
    // // use pic->buf to access the image
    // ESP_LOGI(TAG, "Picture taken! Its size was: %zu bytes", pic->len);
    // // for (int i=0; i<96; i++) {
    // //     ESP_LOGI(TAG, "0x%02X", pic->buf[i]);
    // // }
    // send_whole_frame_buffer(pic);
    // // send_frame_buffer()
    // esp_camera_fb_return(pic);

    
}