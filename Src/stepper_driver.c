#include "stepper_driver.h"
#include "main.h"

static void apply_step(int8_t step) {
  step = step < 0 ? step * -1 : step;
  switch (step) {
      case 0:  // 1010
      HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_RESET);
      break;
      case 1:  // 0110
          HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_RESET);
      break;
      case 2:  //0101
                HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_SET);
      break;
      case 3:  //1001
                      HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_SET);
      break;
  }
}

static void motor_start(stepper_driver_t *instance) {
    instance->mode = MODE_DRIVE;
    HAL_TIM_Base_Start_IT(instance->timerHandle);
    HAL_GPIO_WritePin(STBY_GPIO_Port, STBY_Pin, GPIO_PIN_SET);
    // the following start start the timer
    HAL_TIM_PWM_Start(instance->timerHandle, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(instance->timerHandle, TIM_CHANNEL_4);
}

void stepper_init(stepper_driver_t *instance, TIM_HandleTypeDef *timerHandle) {
    instance->currentPos = 0;
    instance->endPos = 0;
    instance->mode = MODE_STOPPED;
    instance->targetPos = 0;
    instance->startPos = 0;
    instance->timerHandle = timerHandle;
}

void stepper_time_update(stepper_driver_t *instance) {
    bool goToStop = true;
    switch (instance->mode)
    {
    case MODE_STOPPED:
        goToStop = true;
        break;
    case MODE_DRIVE:
        if (instance->currentPos == instance->targetPos) {
            goToStop = true;
        } else if (instance->currentPos > instance->targetPos) {
            instance->currentPos--;
            goToStop = false;
        } else if (instance->currentPos < instance->targetPos) {
            instance->currentPos++;
            goToStop = false;
        }
        break;
    default:
        break;
    }
    if (goToStop) {
        HAL_GPIO_WritePin(STBY_GPIO_Port, STBY_Pin, GPIO_PIN_RESET);
        // the following should stop the timer
        HAL_TIM_PWM_Stop(instance->timerHandle, TIM_CHANNEL_1);
        HAL_TIM_PWM_Stop(instance->timerHandle, TIM_CHANNEL_4);
        HAL_TIM_Base_Stop_IT(instance->timerHandle);
        instance->mode = MODE_STOPPED;
    }
    else {
        apply_step(instance->currentPos % 4);
    }
}

void stepper_set_start(stepper_driver_t *instance) {
    instance->startPos = instance->currentPos;
}

void stepper_set_end(stepper_driver_t *instance) {
    instance->endPos = instance->currentPos;
}

/**
 * @param percentage 0-9
 */
void stepper_drive_target(stepper_driver_t *instance, uint8_t percentage) {
    if (instance->mode != MODE_STOPPED) {
        return;
    }
    if (instance->endPos - instance->startPos <= 0) {
        return;
    }
    if (instance->currentPos < instance->startPos || instance->currentPos > instance->endPos)  {
        return;
    }
    instance->targetPos = (int32_t)((double)percentage/(double)9*(double)(instance->endPos - instance->startPos));
    motor_start(instance);
}

void stepper_drive(stepper_driver_t *instance, int32_t steps) {
    if (instance->mode != MODE_STOPPED) {
        int32_t remaining = instance->targetPos - instance->currentPos;
        if ((remaining > 0 && steps > 0) || (remaining < 0 && steps < 0)) {
            remaining = remaining < 0 ? remaining * -1 : remaining;
            if (remaining < 180) {
                instance->targetPos = instance->targetPos + steps;        
            }
        }
    } else {
        instance->targetPos = instance->currentPos + steps;
        motor_start(instance);
    }
}