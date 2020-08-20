#ifndef APP_STEPPER_DRIVER_H
#define APP_STEPPER_DRIVER_H

#include <stdint.h>
#include "stm32f3xx_hal.h"

typedef uint8_t bool;
#define true (1)
#define false (0)

typedef enum {
    MODE_STOPPED,
    MODE_DRIVE_TARGET,
    MODE_DRIVE_FORWARD,
    MODE_DRIVE_BACKWARD
} stepper_driving_mode_t;

typedef struct
{
    uint32_t startPos;
    uint32_t endPos;
    uint32_t currentPos;
    uint32_t targetPos;
    uint32_t timeout;
    uint16_t stepper_arr_current;
    uint16_t stepper_arr_start;
    uint16_t stepper_arr_max;
    uint16_t stepper_arr_steps;
    uint16_t stepper_arr_dec;
    uint16_t stepper_arr_dec_ctr;
    stepper_driving_mode_t mode;
    TIM_HandleTypeDef *pwmTimerHandler;
    TIM_HandleTypeDef *updateTimerHandle;
} stepper_driver_t;

void stepper_init(stepper_driver_t *instance, TIM_HandleTypeDef *timerHandle, TIM_HandleTypeDef *updateTimerHandle);
void stepper_time_update(stepper_driver_t *instance);
void stepper_set_start(stepper_driver_t *instance);
void stepper_set_end(stepper_driver_t *instance);

/**
 * @param percentage 0-9
 */
void stepper_drive_target(stepper_driver_t *instance, uint8_t percentage);

void stepper_drive_forward(stepper_driver_t *instance);
void stepper_drive_backward(stepper_driver_t *instance);

#endif
