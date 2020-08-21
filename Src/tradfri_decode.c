#include "tradfri_decode.h"
#include "main.h"
#include <math.h>

void tradfri_init(tradfri_decoder_t *handle, TIM_HandleTypeDef *htim, uint32_t tradfri_timer_clock)
{
    uint32_t period = tradfri_timer_clock / (htim->Init.Prescaler + 1) / 600;
    handle->tradfri_period = period;
    handle->pulse_width = 0;
    handle->period = 0;
    handle->duty = 0;
}

static void tradfri_update_value(tradfri_decoder_t *handle, uint16_t duty)
{
    if (handle->duty != duty)
    {
        double corrected = -50.9 + 32.8 * log((double)duty);
        handle->new_value_callback((uint16_t)corrected);
    }
    handle->duty = duty;
}

void tradfri_capture_callback(tradfri_decoder_t *handle, TIM_HandleTypeDef *htim)
{
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        handle->period = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        int16_t diff = handle->period - handle->tradfri_period;
        if (diff < PERIOD_INACCURACY && diff > -PERIOD_INACCURACY)
        {
            uint16_t duty = (100 * handle->pulse_width) / handle->period;
            tradfri_update_value(handle, duty);
        }
    }
    else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        handle->pulse_width = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
    }
}

void tradfri_elapsed_callback(tradfri_decoder_t *handle, TIM_HandleTypeDef *htim)
{
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
    {
        uint16_t duty = HAL_GPIO_ReadPin(TRADFRI_STATE_GPIO_Port, TRADFRI_STATE_Pin) == GPIO_PIN_SET ? 100 : 0;
        tradfri_update_value(handle, duty);
    }
}