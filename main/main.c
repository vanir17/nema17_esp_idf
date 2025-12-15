#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/rmt_tx.h"

// ESP32_DEVKIT_V1 + TB6600 Stepper driver motor: 6400 PULSES/REV, 1.5A RATED, 1.7A PEAK
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define STEP_PIN 23
#define DIR_PIN 22
#define RMT_RESOLUTION_HZ (1000000u)  
#define DURATION1 (500u)
#define DURATION0 (50u)
#define DIR_CW 1
#define DIR_CCW !DIR_CW

/*******************************************************************************
 * Main
 ******************************************************************************/
void app_main(void)
{
    // Cấu hình chân DIR
    gpio_config_t dir_conf = {
        .pin_bit_mask = (1ULL << DIR_PIN), //1: giá trị 01, ULL: unsigned long long(8byte - 64bit), <<: dịch trái
        .mode = GPIO_MODE_OUTPUT, // OUTPUT
        .pull_up_en = GPIO_PULLUP_DISABLE, // tắt điện trở kéo lên
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // tắt điện trở kéo xuống
        .intr_type = GPIO_INTR_DISABLE, // // tắt interupt (không cần)
    };
    gpio_config(&dir_conf);
    

    // Cấu hình RMT Channel
    rmt_channel_handle_t tx_channel = NULL;
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // Nguồn Clock mặc định
        .gpio_num = STEP_PIN,  // Chân bắn xung  
        .mem_block_symbols = 128, // số lượng bộ nhớ đệm
        .resolution_hz = RMT_RESOLUTION_HZ, // 1MHz, nghĩa là 1tick = 1us
        .trans_queue_depth = 4, // Hàng đợi lệnh
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &tx_channel));
    ESP_ERROR_CHECK(rmt_enable(tx_channel));

/*******************************************************************************
 * 
 ******************************************************************************/

    // ------- Cấu hình encoder ------
    rmt_encoder_handle_t copy_encoder = NULL;
    rmt_copy_encoder_config_t copy_encoder_config = {};
    ESP_ERROR_CHECK(rmt_new_copy_encoder(&copy_encoder_config, &copy_encoder));
    rmt_transmit_config_t tx_config = {
        .loop_count = 0,
    };

    // ------- Tạo dữ liệu xung ------
    uint32_t steps = 6400;
    size_t size = steps * sizeof(rmt_symbol_word_t);
    rmt_symbol_word_t *payload_cw = (rmt_symbol_word_t *)malloc(size);
    rmt_symbol_word_t *payload_ccw = (rmt_symbol_word_t *)malloc(size);
    if(payload_cw == NULL || payload_ccw == NULL)
    {
        printf("Tràn Ram!\n");
        return;
    }

    for(int i = 0; i < steps; i++)
    {
        // High level
        payload_cw[i].level0 = 1;
        payload_cw[i].duration0 = DURATION0;
        // Low level
        payload_cw[i].level1 = 0;
        payload_cw[i].duration1 = DURATION0;
    }
    for(int i = 0; i < steps; i++)
    {
        payload_ccw[i].level0 = 1;
        payload_ccw[i].duration0 = DURATION1; 
        payload_ccw[i].level1 = 0;
        payload_ccw[i].duration1 = DURATION1; 
    }
    while(1)
    {
        // CW
        gpio_set_level(DIR_PIN, DIR_CW);
        ESP_ERROR_CHECK(rmt_transmit(tx_channel, copy_encoder, payload_cw, size, &tx_config));
        ESP_ERROR_CHECK(rmt_tx_wait_all_done(tx_channel, -1));
        printf("Xong CW\n");
        vTaskDelay(pdMS_TO_TICKS(200));

        gpio_set_level(DIR_PIN, DIR_CCW);
        ESP_ERROR_CHECK(rmt_transmit(tx_channel, copy_encoder, payload_ccw, size, &tx_config));
        ESP_ERROR_CHECK(rmt_tx_wait_all_done(tx_channel, -1));
        printf("Xong CCW\n");
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    //--- Giải phóng bộ nhớ ---
    free(payload_cw);
    free(payload_ccw);
}