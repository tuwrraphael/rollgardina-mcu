#include "stepper_driver.h"
#include "main.h"

static void apply_step(uint8_t step)
{
    switch (step)
    {
    case 0: // 1010
        HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_RESET);
        break;
    case 1: // 0110
        HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_RESET);
        break;
    case 2: //0101
        HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_SET);
        break;
    case 3: //1001
        HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_SET);
        break;
    }
}

static void motor_start(stepper_driver_t *instance)
{
    __HAL_TIM_SET_AUTORELOAD(instance->updateTimerHandle, instance->stepper_arr_start);
    instance->stepper_arr_current = instance->stepper_arr_start;
    instance->stepper_arr_dec = (instance->stepper_arr_start - instance->stepper_arr_max) / instance->stepper_arr_steps;
    instance->stepper_arr_dec_ctr = 50;
    // the following start start the timer
    HAL_TIM_PWM_Start(instance->pwmTimerHandler, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(instance->pwmTimerHandler, TIM_CHANNEL_4);
    HAL_TIM_Base_Start_IT(instance->updateTimerHandle);
}

void stepper_init(stepper_driver_t *instance, TIM_HandleTypeDef *timerHandle, TIM_HandleTypeDef *updateTimerHandle)
{
    instance->currentPos = UINT32_MAX / 2;
    instance->endPos = UINT32_MAX / 2 + 200;
    instance->mode = MODE_STOPPED;
    instance->targetPos = UINT32_MAX / 2;
    instance->startPos = UINT32_MAX / 2;
    instance->pwmTimerHandler = timerHandle;
    instance->updateTimerHandle = updateTimerHandle;
    instance->timeout = 0;
    instance->stepper_arr_start = 5000;
    instance->stepper_arr_steps = 3;
    HAL_GPIO_WritePin(STBY_GPIO_Port, STBY_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_SET);
}

void stepper_time_update(stepper_driver_t *instance)
{
    instance->stepper_arr_dec_ctr = instance->stepper_arr_dec_ctr > 0 ? instance->stepper_arr_dec_ctr - 1 : 0;
    if (instance->stepper_arr_dec_ctr == 0)
    {
        int32_t target = ((int32_t)instance->stepper_arr_current - (int32_t)instance->stepper_arr_dec);
        if ((int32_t)instance->stepper_arr_max < target) {
            __HAL_TIM_SET_AUTORELOAD(instance->updateTimerHandle, (uint16_t)target);
            instance->stepper_arr_current = (uint16_t)target;
        }       
        instance->stepper_arr_dec_ctr = 50;
    }
    bool goToStop = true;
    instance->timeout = instance->timeout > 0 ? instance->timeout - 1 : 0;
    switch (instance->mode)
    {
    case MODE_STOPPED:
        goToStop = true;
        break;
    case MODE_DRIVE_FORWARD:
        if (instance->timeout > 0)
        {
            instance->currentPos++;
            goToStop = false;
        }
        else
        {
            goToStop = true;
        }
        break;
    case MODE_DRIVE_BACKWARD:
        if (instance->timeout > 0)
        {
            instance->currentPos--;
            goToStop = false;
        }
        else
        {
            goToStop = true;
        }
        break;
    case MODE_DRIVE_TARGET:
        if (instance->currentPos == instance->targetPos)
        {
            goToStop = true;
        }
        else if (instance->currentPos > instance->targetPos)
        {
            instance->currentPos--;
            goToStop = false;
        }
        else if (instance->currentPos < instance->targetPos)
        {
            instance->currentPos++;
            goToStop = false;
        }
        break;
    default:
        break;
    }
    if (goToStop)
    {
        HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_SET);
        // the following should stop the timer
        HAL_TIM_PWM_Stop(instance->pwmTimerHandler, TIM_CHANNEL_1);
        HAL_TIM_PWM_Stop(instance->pwmTimerHandler, TIM_CHANNEL_4);
        HAL_TIM_Base_Stop_IT(instance->updateTimerHandle);
        instance->mode = MODE_STOPPED;
    }
    else
    {
        apply_step(instance->currentPos % 4);
    }
}

void stepper_set_start(stepper_driver_t *instance)
{
    instance->startPos = instance->currentPos;
}

void stepper_set_end(stepper_driver_t *instance)
{
    instance->endPos = instance->currentPos;
}

/**
 * @param percentage 0-9
 */
void stepper_drive_target(stepper_driver_t *instance, uint8_t percentage)
{
    if (instance->mode != MODE_STOPPED)
    {
        return;
    }
    if (instance->endPos < instance->startPos)
    {
        return;
    }
    if (instance->currentPos < instance->startPos || instance->currentPos > instance->endPos)
    {
        return;
    }
    instance->targetPos = instance->startPos + (uint32_t)((double)percentage / (double)9 * (double)(instance->endPos - instance->startPos));
    instance->mode = MODE_DRIVE_TARGET;
    if (instance->targetPos > instance->currentPos)
    {
        instance->stepper_arr_max = 1000;
    }
    else
    {
        instance->stepper_arr_max = 3840;
    }
    motor_start(instance);
}

void stepper_drive_forward(stepper_driver_t *instance)
{
    if (instance->mode != MODE_STOPPED && instance->mode != MODE_DRIVE_FORWARD)
    {
        return;
    }
    instance->timeout = 0.2/(1/(64000000.0/(127.0+1.0)/(double)(instance->stepper_arr_current + 1)));
    if (instance->mode == MODE_STOPPED)
    {
        instance->stepper_arr_max = 1000;
        instance->mode = MODE_DRIVE_FORWARD;
        motor_start(instance);
    }
}
void stepper_drive_backward(stepper_driver_t *instance)
{
    if (instance->mode != MODE_STOPPED && instance->mode != MODE_DRIVE_BACKWARD)
    {
        return;
    }
    instance->timeout = 0.2/(1/(64000000.0/(127.0+1.0)/(double)(instance->stepper_arr_current + 1)));
    if (instance->mode == MODE_STOPPED)
    {
        instance->stepper_arr_max = 3840;
        instance->mode = MODE_DRIVE_BACKWARD;
        motor_start(instance);
    }
}