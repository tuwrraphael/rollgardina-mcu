#ifndef APP_STEPPER_DRIVER_H
#define APP_STEPPER_DRIVER_H

#include <stdint.h>
#include "stm32f3xx_hal.h"

typedef uint8_t bool;
#define true (1)
#define false (0)

#define DRV_TIMEOUT_SECONDS (30)
#define ENDSCHALTER_TIMOUT_SECONDS (2)

#define ABROLL_UMDREHUNGEN (17.5)
#define STEPS_UMDREHUNG (200)
#define MICROSTEPS (8)
#define ABROLL_STEPS (ABROLL_UMDREHUNGEN * STEPS_UMDREHUNG * MICROSTEPS)

#define ACC_TIME_SECONDS (1)
#define ACC_STEPS (50)
#define ACC_STEP_US ((1000 * 1000 * ACC_TIME_SECONDS) / ACC_STEPS)
#define ACC_START_STEP_S (STEPS_UMDREHUNG * MICROSTEPS)

#define ZEIT_RUNTER (5)
#define ZEIT_RAUF (15)

typedef enum
{
    MODE_STOPPED,
    MODE_DRIVE_TARGET
} stepper_driving_mode_t;

typedef enum
{
    DIR_NONE,
    DIR_RUNTER,
    DIR_RAUF
} stepper_dir_t;

typedef struct
{
    uint32_t obenPos;
    uint32_t untenPos;
    uint32_t currentPos;
    uint32_t targetPos;
    uint16_t targetPercentage;
    uint32_t timeout;
    uint32_t acc_timeout;
    uint16_t acc_step;
    uint32_t endschalter_timeout;
    uint16_t update_cycle_us;
    uint32_t update_timer_clock;
    uint16_t arr_speed_runter;
    uint16_t arr_speed_rauf;
    uint16_t arr_speed_start;
    uint16_t arr_speed_end;
    bool first_value_received;
    bool drv_pin;
    stepper_driving_mode_t mode;
    stepper_dir_t last_dir;

    TIM_HandleTypeDef *updateTimerHandle;
} stepper_driver_t;

void stepper_init(stepper_driver_t *instance, TIM_HandleTypeDef *updateTimerHandle, uint32_t timer_clock);
void stepper_time_update(stepper_driver_t *instance);

/**
 * @param percentage 0-100
 */
void stepper_drive_target(stepper_driver_t *instance, uint16_t percentage);

#endif
