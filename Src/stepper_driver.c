#include "stepper_driver.h"
#include "main.h"

#define STATE_SLEEPING (GPIO_PIN_RESET)
#define STATE_ACTIVE (GPIO_PIN_SET)
#define PIN_DIR_UNTEN (GPIO_PIN_RESET)
#define PIN_DIR_OBEN (GPIO_PIN_SET)

static void change_period(stepper_driver_t *instance, uint16_t period)
{
    __HAL_TIM_SET_AUTORELOAD(instance->updateTimerHandle, period);
    instance->update_cycle_us = (1000 * 1000 * period) / instance->update_timer_clock;
}

static void reset_acc(stepper_driver_t *instance, stepper_dir_t dir)
{
    instance->acc_timeout = ACC_STEP_US;
    instance->acc_step = ACC_STEPS;
    change_period(instance, instance->arr_speed_start);
    if (DIR_RAUF == dir)
    {
        instance->arr_speed_end = instance->arr_speed_rauf;
    }
    else if (DIR_RUNTER == dir)
    {
        instance->arr_speed_end = instance->arr_speed_runter;
    }
    else
    {
        instance->arr_speed_end = instance->arr_speed_start;
    }
}

static void motor_start(stepper_driver_t *instance)
{
    HAL_GPIO_WritePin(DRV_SLEEP_GPIO_Port, DRV_SLEEP_Pin, STATE_ACTIVE);
    HAL_TIM_Base_Start_IT(instance->updateTimerHandle);
    instance->drv_pin = false;
}

static uint32_t get_pos(stepper_driver_t *instance, uint16_t percentage)
{
    return instance->obenPos + (uint32_t)((double)percentage / (double)100 * (double)(instance->untenPos - instance->obenPos));
}

void stepper_init(stepper_driver_t *instance, TIM_HandleTypeDef *updateTimerHandle, uint32_t timer_clock)
{
    instance->currentPos = UINT32_MAX / 2;
    instance->untenPos = UINT32_MAX / 2 + (uint32_t)ABROLL_STEPS;
    instance->mode = MODE_STOPPED;
    instance->targetPos = UINT32_MAX / 2;
    instance->targetPercentage = 0;
    instance->obenPos = UINT32_MAX / 2;
    instance->updateTimerHandle = updateTimerHandle;
    instance->timeout = 0;
    instance->acc_step = 0;
    instance->acc_timeout = 0;
    instance->first_value_received = false;
    instance->last_dir = DIR_NONE;
    instance->update_timer_clock = timer_clock / (updateTimerHandle->Init.Prescaler + 1);
    instance->arr_speed_runter = ((uint32_t)(ZEIT_RUNTER * instance->update_timer_clock) / ABROLL_STEPS) / 2;
    instance->arr_speed_rauf = ((uint32_t)(ZEIT_RAUF * instance->update_timer_clock) / ABROLL_STEPS) / 2;
    instance->arr_speed_start = (1 * instance->update_timer_clock / ACC_START_STEP_S) / 2;
    instance->arr_speed_end = instance->arr_speed_start;
    instance->drv_pin = false;
    instance->endschalter_unten_ctr = 0;
    instance->endschalter_oben_ctr = 0;

    HAL_GPIO_WritePin(DRV_M0_GPIO_Port, DRV_M0_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DRV_M1_GPIO_Port, DRV_M1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DRV_M2_GPIO_Port, DRV_M2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DRV_STEP_GPIO_Port, DRV_STEP_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DRV_EN_GPIO_Port, DRV_EN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DRV_RESET_GPIO_Port, DRV_RESET_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DRV_DIR_GPIO_Port, DRV_DIR_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DRV_SLEEP_GPIO_Port, DRV_SLEEP_Pin, STATE_SLEEPING);
}

void stepper_time_update(stepper_driver_t *instance)
{
    bool obenGedrueckt = HAL_GPIO_ReadPin(END_OBEN_GPIO_Port, END_OBEN_Pin) == GPIO_PIN_RESET;
    bool untenGedrueckt = HAL_GPIO_ReadPin(END_UNTEN_GPIO_Port, END_UNTEN_Pin) == GPIO_PIN_RESET;

    if (obenGedrueckt) {
        instance->endschalter_oben_ctr = instance->endschalter_oben_ctr >= ENDSCHALTER_DEBONCE_US ? instance->endschalter_oben_ctr : instance->endschalter_oben_ctr + instance->update_cycle_us;
    } else {
        instance->endschalter_oben_ctr = 0;
    }
    if (untenGedrueckt) {
        instance->endschalter_unten_ctr = instance->endschalter_unten_ctr >= ENDSCHALTER_DEBONCE_US ? instance->endschalter_unten_ctr : instance->endschalter_unten_ctr + instance->update_cycle_us;
    } else {
        instance->endschalter_unten_ctr = 0;
    }

    bool obenEndReached = instance->endschalter_oben_ctr >= ENDSCHALTER_DEBONCE_US;
    bool untenEndReached = instance->endschalter_unten_ctr >= ENDSCHALTER_DEBONCE_US;

    if (obenEndReached)
    {
        instance->currentPos = instance->obenPos;
        instance->targetPos = get_pos(instance, instance->targetPercentage);
    }
    else if (untenEndReached)
    {
        instance->currentPos = instance->untenPos;
        instance->targetPos = get_pos(instance, instance->targetPercentage);
    }

    instance->timeout = instance->timeout > instance->update_cycle_us ? instance->timeout - instance->update_cycle_us : 0;
    if (obenGedrueckt || untenGedrueckt) {
        instance->endschalter_timeout += instance->update_cycle_us;
    }
    int16_t makeStep = 0;
    bool endschalterAbwarten = false;
    switch (instance->mode)
    {
    case MODE_STOPPED:
        makeStep = 0;
        break;
    case MODE_DRIVE_TARGET:
        if (instance->timeout == 0 || instance->endschalter_timeout > (ENDSCHALTER_TIMOUT_SECONDS * 1000 * 1000))
        {
            makeStep = 0;
        }
        else if (instance->currentPos == instance->targetPos)
        {
            makeStep = 0;
        }
        else if (instance->currentPos > instance->targetPos && !obenEndReached)
        {
            makeStep = -1;
            endschalterAbwarten = obenGedrueckt;
            HAL_GPIO_WritePin(DRV_DIR_GPIO_Port, DRV_DIR_Pin, PIN_DIR_OBEN);
        }
        else if (instance->currentPos < instance->targetPos && !untenEndReached)
        {
            makeStep = 1;
            endschalterAbwarten = untenGedrueckt;
            HAL_GPIO_WritePin(DRV_DIR_GPIO_Port, DRV_DIR_Pin, PIN_DIR_UNTEN);
        }
        break;
    default:
        makeStep = 0;
        break;
    }
    stepper_dir_t dir = makeStep < 0 ? DIR_RAUF : makeStep > 0 ? DIR_RUNTER : DIR_NONE;
    if (makeStep == 0)
    {
        HAL_GPIO_WritePin(DRV_SLEEP_GPIO_Port, DRV_SLEEP_Pin, STATE_SLEEPING);
        HAL_GPIO_WritePin(DRV_STEP_GPIO_Port, DRV_STEP_Pin, GPIO_PIN_RESET);
        HAL_TIM_Base_Stop_IT(instance->updateTimerHandle);
        instance->mode = MODE_STOPPED;
        instance->last_dir = DIR_NONE;
        instance->drv_pin = false;
    }
    else
    {
        if (dir != instance->last_dir)
        {
            instance->last_dir = dir;
            reset_acc(instance, dir);
        }
        if (!endschalterAbwarten) {
            instance->acc_timeout = instance->acc_timeout > instance->update_cycle_us ? instance->acc_timeout - instance->update_cycle_us : 0;
            if (instance->acc_timeout == 0)
            {
                if (instance->acc_step > 0)
                {
                    instance->acc_step--;
                    double arr = instance->arr_speed_start + (double)(instance->arr_speed_end - instance->arr_speed_start) * (1.0 - ((double)instance->acc_step / (double)ACC_STEPS));
                    change_period(instance, (uint16_t)arr);
                    instance->acc_timeout = ACC_STEP_US;
                }
            }
            HAL_GPIO_WritePin(DRV_STEP_GPIO_Port, DRV_STEP_Pin, instance->drv_pin ? GPIO_PIN_RESET : GPIO_PIN_SET);
            if (instance->drv_pin)
            {
                instance->currentPos += makeStep;
            }
            instance->drv_pin = !instance->drv_pin;
        }
    }
}

void stepper_drive_target(stepper_driver_t *instance, uint16_t percentage)
{
    if (!instance->first_value_received)
    {
        instance->first_value_received = true;
        instance->currentPos = get_pos(instance, percentage);
        return;
    }
    instance->targetPercentage = percentage;
    bool startedBefore = instance->mode != MODE_STOPPED;
    if (instance->untenPos < instance->obenPos)
    {
        return;
    }
    // if (instance->currentPos < instance->obenPos || instance->currentPos > instance->untenPos)
    // {
    //     return;
    // }
    instance->targetPos = get_pos(instance, percentage);
    instance->mode = MODE_DRIVE_TARGET;
    instance->timeout = DRV_TIMEOUT_SECONDS * 1000 * 1000;
    instance->endschalter_timeout = 0;
    if (!startedBefore)
    {
        motor_start(instance);
    }
}