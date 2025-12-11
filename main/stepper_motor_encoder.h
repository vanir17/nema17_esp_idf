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