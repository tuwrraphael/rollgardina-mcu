#ifndef INC_NEC_DECODE_H_
#define INC_NEC_DECODE_H_

#include <stdint.h>
#include "stm32f3xx_hal.h"


typedef enum {
    IR_IDLE, IR_START, IR_DECODE
} NEC_STATE;

typedef struct {
    int ptr;
    uint16_t decoded[2];

    NEC_STATE state;

    void (*NEC_DecodedCallback)(uint16_t, uint8_t);
    void (*NEC_RepeatCallback)();
} NEC;

void NEC_TIM_IC_CaptureCallback(NEC* handle, TIM_HandleTypeDef *htim);

#endif
