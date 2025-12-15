#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "driver/gpio.h"
#include "soc/rmt_reg.h" // Cần thiết cho các API encoder cũ (nếu dùng)

// --- CẤU HÌNH PHẦN CỨNG ---
#define STEP_GPIO_PIN 23  // Chân GPIO cho xung STEP (thay đổi nếu cần)
#define DIR_GPIO_PIN 22   // Chân GPIO cho hướng DIR (thay đổi nếu cần)

// --- CẤU HÌNH RMT ---
// Độ phân giải RMT: 1 MHz (1 tick = 1 microsecond)
#define RMT_RESOLUTION_HZ 1000000 
// Tần số ban đầu: 500 Hz (Tốc độ chậm để kiểm tra)
#define INITIAL_FREQ_HZ 500 

// Tính toán duration cho 500 Hz
// T_chu_ky = 1 / 500 Hz = 2000 us (ticks)
// T_HIGH = T_LOW = 1000 us (ticks)
#define STEP_DURATION_TICKS (RMT_RESOLUTION_HZ / INITIAL_FREQ_HZ / 2) // Kết quả: 1000

static const char *TAG = "STEPPER_RMT";
rmt_channel_handle_t step_channel = NULL;
// Cần thêm RMT Encoder Handle cho v5.x
rmt_encoder_handle_t symbol_encoder = NULL; 

// Định nghĩa một symbol RMT cho một xung vuông 50% duty cycle
// Symbol: HIGH trong STEP_DURATION_TICKS, sau đó LOW trong STEP_DURATION_TICKS
static const rmt_symbol_word_t step_symbol = {
    .level0 = 1,                 // HIGH (Level 1)
    .duration0 = STEP_DURATION_TICKS, // Thời gian HIGH (1000 us)
    .level1 = 0,                 // LOW (Level 0)
    .duration1 = STEP_DURATION_TICKS  // Thời gian LOW (1000 us)
};


/**
 * @brief Khởi tạo các chân GPIO phụ trợ (DIR)
 */
void init_gpio() {
    // Cấu hình chân DIR
    gpio_reset_pin(DIR_GPIO_PIN);
    gpio_set_direction(DIR_GPIO_PIN, GPIO_MODE_OUTPUT);
    
    // Đặt hướng quay mặc định
    gpio_set_level(DIR_GPIO_PIN, 0); 

    ESP_LOGI(TAG, "GPIO pins initialized. DIR set to 0.");
}

/**
 * @brief Khởi tạo và cấu hình kênh RMT và Encoder cho v5.x
 */
void init_rmt() {
    // 1. Cấu hình và tạo kênh RMT
    rmt_tx_channel_config_t tx_config = {
        .gpio_num = STEP_GPIO_PIN, 
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = RMT_RESOLUTION_HZ, 
        .mem_block_symbols = 64,  
        .trans_queue_depth = 4,   
        .flags.with_dma = false,  
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_config, &step_channel));

    // 2. Cấu hình và tạo RMT Encoder (để mã hóa trực tiếp symbol)
    // Trong v5.x, chúng ta sử dụng rmt_new_copy_encoder để sao chép dữ liệu symbol trực tiếp
    rmt_copy_encoder_config_t copy_config = {};
    ESP_ERROR_CHECK(rmt_new_copy_encoder(&copy_config, &symbol_encoder));
    
    ESP_LOGI(TAG, "RMT Channel and Encoder initialized.");
}

/**
 * @brief Bắt đầu truyền xung STEP lặp vô hạn
 */
void start_stepper_motor() {
    // Cho phép kênh RMT
    ESP_ERROR_CHECK(rmt_enable(step_channel));
    
    // Đã sửa: Sử dụng rmt_transmit_config_t thay vì rmt_tx_config_t
    rmt_transmit_config_t tx_config = {
        // Đã sửa: .loop_count nằm trong cấu trúc này
        .loop_count = -1, // -1 nghĩa là truyền lặp vô hạn
        .flags.eot_level = 0 // Đảm bảo chân STEP về LOW sau khi truyền xong (nếu không lặp)
    };

    // Đã sửa: Truyền dữ liệu symbol bằng encoder mới
    ESP_ERROR_CHECK(rmt_transmit(
        step_channel,
        symbol_encoder,           // Sử dụng encoder đã tạo
        (void*)&step_symbol,      // Dữ liệu symbol
        sizeof(step_symbol),      // Kích thước dữ liệu
        &tx_config                // Cấu hình truyền (lặp)
    ));

    ESP_LOGI(TAG, "Stepper Motor started at %d Hz on GPIO %d.", INITIAL_FREQ_HZ, STEP_GPIO_PIN);
}

/**
 * @brief Hàm chính của ứng dụng
 */
void app_main(void) {
    // 1. Khởi tạo GPIO (cho chân DIR, ENABLE,...)
    init_gpio();

    // 2. Khởi tạo Kênh RMT và Encoder
    init_rmt();

    // 3. Bắt đầu truyền xung STEP
    start_stepper_motor();
    
    // --- Vòng lặp chính của chương trình ---
    int count = 0;
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(5000)); // Đợi 5 giây
        count++;

        // Ví dụ: Đảo chiều quay sau mỗi 10 giây
        if (count % 2 == 0) {
            gpio_set_level(DIR_GPIO_PIN, 0);
            ESP_LOGI(TAG, "Direction changed to 0 (Counter-Clockwise).");
        } else {
            gpio_set_level(DIR_GPIO_PIN, 1);
            ESP_LOGI(TAG, "Direction changed to 1 (Clockwise).");
        }
    }
}