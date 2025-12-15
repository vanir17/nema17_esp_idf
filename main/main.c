#include <stdio.h>
#include <math.h>
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
#define PULSES_PER_REV (6400u)
#define RMT_RESOLUTION_HZ (1000000u)  
#define DURATION1 (500u)
#define DURATION0 (50u)
#define DIR_CW 1
#define DIR_CCW !DIR_CW
/*******************************************************************************
 * Variables
 ******************************************************************************/
rmt_channel_handle_t g_tx_channel = NULL;
rmt_encoder_handle_t g_copy_encoder = NULL;
rmt_symbol_word_t *g_payload = NULL;
float g_current_angle = 0.0; // Vị trí hiện tại của khớp (Mặc định khi bật điện là 0)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void setup_RMT();
void move_motor(float angle, int dir, int duration);
void goto_angle(float target_angle, int duration);
void run_round_trip (float target, int duration);
/*******************************************************************************
 * Main
 ******************************************************************************/
void app_main(void)
{

    setup_RMT();
    while(1)
    {
        printf("=== BẮT ĐẦU CHU TRÌNH MỚI ===\n");

            // --- BƯỚC 1: SANG PHẢI 90 ĐỘ (Góc +90) ---
            printf("Step 1: Phai 90\n");
            run_round_trip(90.0, 200); // Chậm
            run_round_trip(90.0, 40);  // Nhanh

            // --- BƯỚC 2: SANG TRÁI 90 ĐỘ (Góc -90) ---
            printf("Step 2: Trai 90\n");
            run_round_trip(-90.0, 200); // Chậm
            run_round_trip(-90.0, 40);  // Nhanh

            // c--- BƯỚC 3: SANG PHẢI 180 ĐỘ (Góc +180) ---
            printf("Step 3: Phai 180\n");
            // run_round_trip(180.0, 200); // Vừa (Theo yêu cầu của bạn)
            run_round_trip(180.0, 50);  // Nhanh

            // --- BƯỚC 4: SANG TRÁI 180 ĐỘ (Góc -180) ---
            printf("Step 4: Trai 180\n");
            // run_round_trip(-180.0, 200); // Vừa
            run_round_trip(-180.0, 50);  // Nhanh

            // --- BƯỚC 5: QUAY 1 VÒNG (360 độ) ---
            // Vòng Phải
            printf("Step 5: Vong tron Phai\n");
            // run_round_trip(360.0, 200);
            run_round_trip(360.0, 40);

            // Vòng Trái
            printf("Step 5: Vong tron Trai\n");
            // run_round_trip(-360.0, 200);
            run_round_trip(-360.0, 40);

            printf("=== XONG, NGHỈ 2 GIÂY ===\n");
            vTaskDelay(pdMS_TO_TICKS(2000));




    }
}


/*******************************************************************************
 * Functions
 ******************************************************************************/

void setup_RMT()
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
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // Nguồn Clock mặc định
        .gpio_num = STEP_PIN,  // Chân bắn xung  
        .mem_block_symbols = 128, // số lượng bộ nhớ đệm
        .resolution_hz = RMT_RESOLUTION_HZ, // 1MHz, nghĩa là 1tick = 1us
        .trans_queue_depth = 4, // Hàng đợi lệnh
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &g_tx_channel));
    ESP_ERROR_CHECK(rmt_enable(g_tx_channel));

    rmt_copy_encoder_config_t copy_encoder_config = {};
    ESP_ERROR_CHECK(rmt_new_copy_encoder(&copy_encoder_config, &g_copy_encoder));
    // rmt_transmit_config_t tx_config = {
    //     .loop_count = 0,
    // };
    size_t size = PULSES_PER_REV * sizeof(rmt_symbol_word_t);
    g_payload = (rmt_symbol_word_t *)malloc(size);

}
void move_motor(float angle, int dir, int duration)
{

    if(g_payload == NULL)
    {
        return;
    }
    for(int i = 0; i < PULSES_PER_REV; i++)
    {
        g_payload[i].level0 = DIR_CW;
        g_payload[i].duration0 = duration;
        g_payload[i].level1 = DIR_CCW;
        g_payload[i].duration1 = duration;
    }


    // --- angle -> pulses
    uint32_t steps_needed = (uint32_t)((angle / 360.0) * PULSES_PER_REV);
    if(steps_needed > PULSES_PER_REV)
    {
        printf("Goc quay qua lon so voi bo nho dem!\n");
        steps_needed = PULSES_PER_REV; // Cắt bớt nếu quá lớn
    }
    else if(steps_needed == 0)
    {
        return;
    }
 
    gpio_set_level(DIR_PIN, dir);
    size_t size_sent = steps_needed * sizeof(rmt_symbol_word_t);
    
    rmt_transmit_config_t tx_config = { .loop_count = 0};
    
    printf("Đang quay %.2f độ, (%ld bước), Hướng: %d\n", angle, steps_needed, dir);

    // ---  Bắn xung --- 
    ESP_ERROR_CHECK(rmt_transmit(g_tx_channel, g_copy_encoder, 
                                g_payload, size_sent, &tx_config));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(g_tx_channel, -1));

}


void goto_angle (float target_angle, int duration)
{
    float delta_angle = target_angle - g_current_angle;
    if(fabs(delta_angle) < 0.1) return;
    int dir = (delta_angle > 0) ? 1 : 0;
    float move_amount = fabs(delta_angle);
    move_motor(move_amount, dir, duration);

    g_current_angle = target_angle;
    printf("Đã di chuyển đến vị trí: %.2f độ\n", g_current_angle);
}
void run_round_trip (float target, int duration)
{
    goto_angle(target, duration);
    vTaskDelay(pdMS_TO_TICKS(200)); 
    
    goto_angle(0.0, duration);
    vTaskDelay(pdMS_TO_TICKS(200)); 
}
