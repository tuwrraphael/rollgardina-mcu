#ifndef APP_STEPPER_DRIVER_H
#define APP_STEPPER_DRIVER_H

#include <stdint.h>
#include "stm32f3xx_hal.h"

typedef uint8_t bool;
#define true (1)
#define false (0)

#define ENDSCHALTER_TIMOUT_SECONDS (2)

#define ABROLL_UMDREHUNGEN (17.5)
#define STEPS_UMDREHUNG (200)
#define MICROSTEPS (4)
#define ABROLL_STEPS (ABROLL_UMDREHUNGEN * STEPS_UMDREHUNG * MICROSTEPS)

#define ACC_TIME_SECONDS (2)
#define ACC_STEPS (50)
#define ACC_STEP_US ((1000 * 1000 * ACC_TIME_SECONDS) / ACC_STEPS)
#define ACC_START_STEP_S (50 * MICROSTEPS)

#define ZEIT_RUNTER (45)
#define ZEIT_RAUF (45)
#define MAX_ZEIT (ZEIT_RAUF > ZEIT_RUNTER ? ZEIT_RAUF : ZEIT_RUNTER)

#define MIN_TIMEOUT (30)
#define DRV_TIMEOUT_SECONDS (MAX_ZEIT > MIN_TIMEOUT ? MAX_ZEIT : MIN_TIMEOUT)

#define ENDSCHALTER_DEBONCE_MS (300)
#define ENDSCHALTER_DEBONCE_US (ENDSCHALTER_DEBONCE_MS * 1000)

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
    uint32_t endschalter_oben_ctr;
    uint32_t endschalter_unten_ctr;

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
