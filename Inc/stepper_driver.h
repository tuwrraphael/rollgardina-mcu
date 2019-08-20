#ifndef APP_STEPPER_DRIVER_H
#define APP_STEPPER_DRIVER_H

#include <stdint.h>
#include "stm32f3xx_hal.h"

typedef uint8_t bool;
#define true (1)
#define false (0)

typedef enum {
    MODE_STOPPED,
    MODE_DRIVE
} stepper_driving_mode_t;

typedef struct
{
    int32_t startPos;
    int32_t endPos;
    int32_t currentPos;
    int32_t targetPos;
    stepper_driving_mode_t mode;
    TIM_HandleTypeDef *timerHandle;
} stepper_driver_t;

void stepper_init(stepper_driver_t *instance, TIM_HandleTypeDef *timerHandle);
void stepper_time_update(stepper_driver_t *instance);
void stepper_set_start(stepper_driver_t *instance);
void stepper_set_end(stepper_driver_t *instance);

/**
 * @param percentage 0-9
 */
void stepper_drive_target(stepper_driver_t *instance, uint8_t percentage);

void stepper_drive(stepper_driver_t *instance, int32_t steps);

#endif
