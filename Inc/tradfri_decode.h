#ifndef INC_NEC_DECODE_H_
#define INC_NEC_DECODE_H_

#include <stdint.h>
#include "stm32f3xx_hal.h"

#define PERIOD_INACCURACY (40)
#define TRADFRI_FREQ (600)
#define RESET_WAIT_SECONDS (3)
#define SLEEP_WAIT_SECONDS (3)


typedef struct
{
    uint16_t pulse_width;
    uint16_t period;
    uint16_t tradfri_period;
    uint16_t duty;
    uint16_t wait_values_ctr;
    uint16_t elapsed_ctr;
    uint16_t elapsed_max;

    void (*new_value_callback)(uint16_t);
} tradfri_decoder_t;

void tradfri_capture_callback(tradfri_decoder_t *handle, TIM_HandleTypeDef *htim);
void tradfri_elapsed_callback(tradfri_decoder_t *handle, TIM_HandleTypeDef *htim);
void tradfri_init(tradfri_decoder_t *handle, TIM_HandleTypeDef *htim, uint32_t tradfri_timer_clock);
uint8_t tradfri_can_sleep(tradfri_decoder_t *handle);

#endif
