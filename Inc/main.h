/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DRV_M1_Pin GPIO_PIN_0
#define DRV_M1_GPIO_Port GPIOF
#define DRV_M0_Pin GPIO_PIN_1
#define DRV_M0_GPIO_Port GPIOF
#define TRADFRI_STATE_Pin GPIO_PIN_1
#define TRADFRI_STATE_GPIO_Port GPIOA
#define DRV_STEP_Pin GPIO_PIN_0
#define DRV_STEP_GPIO_Port GPIOB
#define DRV_M2_Pin GPIO_PIN_1
#define DRV_M2_GPIO_Port GPIOB
#define DRV_EN_Pin GPIO_PIN_8
#define DRV_EN_GPIO_Port GPIOA
#define DRV_DIR_Pin GPIO_PIN_12
#define DRV_DIR_GPIO_Port GPIOA
#define END_UNTEN_Pin GPIO_PIN_4
#define END_UNTEN_GPIO_Port GPIOB
#define END_OBEN_Pin GPIO_PIN_5
#define END_OBEN_GPIO_Port GPIOB
#define DRV_RESET_Pin GPIO_PIN_6
#define DRV_RESET_GPIO_Port GPIOB
#define DRV_SLEEP_Pin GPIO_PIN_7
#define DRV_SLEEP_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define TIMER2_CLOCK (8000000)
#define TIMER3_CLOCK (8000000)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
