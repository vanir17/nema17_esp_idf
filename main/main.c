#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/rmt_tx.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define STEP_PIN 23
#define DIR_PIN 22
#define RMT_RESOLUTION_HZ (1000000u)  
#define DURATION0 (75u)
// #define 
// #define DURATION1 5000
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
    gpio_set_level(DIR_PIN, 1);

    // Cấu hình RMT Channel
    rmt_channel_handle_t tx_channel = NULL;
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // Nguồn Clock mặc định
        .gpio_num = STEP_PIN,  // Chân bắn xung  
        .mem_block_symbols = 128, // số lượng bộ nhớ đệm
        .resolution_hz = RMT_RESOLUTION_HZ, // 1MHz, nghĩa là 1tick = 1us
        .trans_queue_depth = 4, // Hàng đợi lệnh
    };
    // Tạo channel mới
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &tx_channel));

    //Bật channel lên (Enable)
    ESP_ERROR_CHECK(rmt_enable(tx_channel));


    // Tạo dữ liệu xung
    uint32_t steps = 6400;
    size_t size = steps * sizeof(rmt_symbol_word_t);
    rmt_symbol_word_t *payload = (rmt_symbol_word_t *)malloc(size);

    if(payload)
    {
        /* Cấu hình thông số cho 01 bước, ví dụ muốn 1000 xung / giây => 1 xung = 1ms
            => High = 500us, Low = 500us
            -> duration = 500 tick vì 1 tick = 1 us
        */
       for(int i = 0; i < steps; i++)
       {
            // High level
            payload[i].level0 = 1;
            payload[i].duration0 = DURATION0;

            // Low level
            payload[i].level1 = 0;
            payload[i].duration1 = DURATION0;
       }
       printf("Bat dau quay dong co...\n");

       rmt_encoder_handle_t copy_encoder = NULL;
       rmt_copy_encoder_config_t copy_encoder_config = {};
       ESP_ERROR_CHECK(rmt_new_copy_encoder(&copy_encoder_config, &copy_encoder));
       // Tiếp theo sẽ gửi lệnh (vòng for) để phát xung, sẽ copy dữ liệu vào RMT hardware cho tự chạy
       rmt_transmit_config_t tx_config = {
            .loop_count = 0, // 0 lần lặp lại mảng
       };

       
            ESP_ERROR_CHECK(rmt_transmit(tx_channel, copy_encoder, payload, size, &tx_config));
            printf("đã gửi 200 xung \n");
            vTaskDelay(pdMS_TO_TICKS(15000));        
       

       free(payload);
    }

}