#include "NEC_Decode.h"

static uint32_t time_high = 0;
static uint32_t time_low = 0;

static void data_sampling(NEC* handle, uint32_t high, uint32_t low) {
    switch (handle->state)
    {
    case IR_IDLE:
        if (low > 8000 && low < 10000) {
            handle->state = IR_START;
            handle->decoded[0] = 0;
            handle->decoded[1] = 0;
            handle->ptr = 0;
        }
        break;
    case IR_START:
        if (high > 4000 && high < 5000) {
            handle->state = IR_DECODE;
        }
        else if (high > 1800 && high < 2800) {
            handle->NEC_RepeatCallback();
            handle->state = IR_IDLE;
        } 
        else {
            handle->state = IR_IDLE;
        }
        break;
    case IR_DECODE:
        if (time_high > 800 && time_high < 3000) {
            handle->decoded[handle->ptr > 15 ? 1 : 0] |= (time_high > 1680 ? 1 : 0) << (handle->ptr % 16);
            handle->ptr++;
            if (handle->ptr == 32)
            {
                uint8_t cmd = ((handle->decoded[1] >> 8) & 0xFF);
                uint8_t control = ~(handle->decoded[1] & 0xFF);
                if (control == cmd) {
                    handle->NEC_DecodedCallback(handle->decoded[0], cmd);
                    handle->state = IR_IDLE;
                }
            }
        }
        else {
            handle->state = IR_IDLE;
        }
        break;
    default:
        break;
    }
}

void NEC_TIM_IC_CaptureCallback(NEC* handle, TIM_HandleTypeDef *htim) {

    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        uint32_t high = time_high - time_low;
        time_low = HAL_TIM_ReadCapturedValue(htim , TIM_CHANNEL_2);
        data_sampling(handle, high, time_low);
    }
    else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
      time_high = HAL_TIM_ReadCapturedValue(htim , TIM_CHANNEL_1);     
    }
}