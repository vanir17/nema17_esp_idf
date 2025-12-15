#ifndef stepper_motor_encoder_h
#define stepper_motor_encoder_h

#include <stdint.h>
#include <driver/rmt_encoder.h>

#ifdef __cplusplus
extern "C"{
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/**
 * @brief Stepper motor curve encoder configuration
 */
 typedef struct 
 {
    uint32_t resolution; //Encoder resolution in Hz
    uint32_t sample_points; // Samples points used for deceleration phase, abs(end_fre_hz - start_fre_hz >= samplepoints)
    uint32_t start_freq_hz;
    uint32_t end_freq_hz;
}stepper_motor_curve_encoder_config_t;

/**
 * @brief Stepper motor uniform encoder configuration
 */
typedef struct 
{
    uint32_t resolution; // Encoder resolution, in Hz
}stepper_motor_uniform_encoder_config_t;

/**
 * @brief Type of RMT encoder handle
 */
typedef struct rmt_encoder_t *rmt_encoder_handle_t;

/**
 * @brief Create stepper motor curve encoder
 *
 * @param[in] config Encoder configuration
 * @param[out] ret_encoder Returned encoder handle
 * @return
 *      - ESP_ERR_INVALID_ARG for any invalid arguments
 *      - ESP_ERR_NO_MEM out of memory when creating step motor encoder
 *      - ESP_OK if creating encoder successfully
 */
esp_err_t rmt_new_stepper_motor_curve_encoder(const stepper_motor_curve_encoder_config_t *config, rmt_encoder_handle_t *ret_encoder);

/**
 * @brief Create RMT encoder for encoding step motor uniform phase into RMT symbols
 *
 * @param[in] config Encoder configuration
 * @param[out] ret_encoder Returned encoder handle
 * @return
 *      - ESP_ERR_INVALID_ARG for any invalid arguments
 *      - ESP_ERR_NO_MEM out of memory when creating step motor encoder
 *      - ESP_OK if creating encoder successfully
 */
esp_err_t rmt_new_stepper_motor_uniform_encoder(const stepper_motor_uniform_encoder_config_t *config, rmt_encoder_handle_t *ret_encoder);

#ifdef __cplusplus
}
#endif

#endif