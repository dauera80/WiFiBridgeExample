/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI15_10_IRQn
#define ADC_ACC_Z_Pin GPIO_PIN_0
#define ADC_ACC_Z_GPIO_Port GPIOC
#define ADC_ACC_Y_Pin GPIO_PIN_1
#define ADC_ACC_Y_GPIO_Port GPIOC
#define CLCD_D0_Pin GPIO_PIN_2
#define CLCD_D0_GPIO_Port GPIOC
#define CLCD_D1_Pin GPIO_PIN_3
#define CLCD_D1_GPIO_Port GPIOC
#define ADC_VAR_Pin GPIO_PIN_0
#define ADC_VAR_GPIO_Port GPIOA
#define ADC_CDS_Pin GPIO_PIN_1
#define ADC_CDS_GPIO_Port GPIOA
#define ADC_TCA_Pin GPIO_PIN_4
#define ADC_TCA_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define LED0_Pin GPIO_PIN_6
#define LED0_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_7
#define LED1_GPIO_Port GPIOA
#define CLCD_D2_Pin GPIO_PIN_4
#define CLCD_D2_GPIO_Port GPIOC
#define CLCD_D3_Pin GPIO_PIN_5
#define CLCD_D3_GPIO_Port GPIOC
#define ADC_ACC_X_Pin GPIO_PIN_0
#define ADC_ACC_X_GPIO_Port GPIOB
#define SW0_Pin GPIO_PIN_1
#define SW0_GPIO_Port GPIOB
#define SW0_EXTI_IRQn EXTI1_IRQn
#define SW1_Pin GPIO_PIN_2
#define SW1_GPIO_Port GPIOB
#define SW1_EXTI_IRQn EXTI2_IRQn
#define SPI2_CS_Pin GPIO_PIN_12
#define SPI2_CS_GPIO_Port GPIOB
#define EN_SW_Pin GPIO_PIN_8
#define EN_SW_GPIO_Port GPIOA
#define EN_SW_EXTI_IRQn EXTI9_5_IRQn
#define LED2_Pin GPIO_PIN_11
#define LED2_GPIO_Port GPIOA
#define LED3_Pin GPIO_PIN_12
#define LED3_GPIO_Port GPIOA
#define EN_B_SIG_Pin GPIO_PIN_15
#define EN_B_SIG_GPIO_Port GPIOA
#define EN_B_SIG_EXTI_IRQn EXTI15_10_IRQn
#define CLCD_EN_Pin GPIO_PIN_10
#define CLCD_EN_GPIO_Port GPIOC
#define CLCD_RS_Pin GPIO_PIN_11
#define CLCD_RS_GPIO_Port GPIOC
#define EN_A_SIG_Pin GPIO_PIN_12
#define EN_A_SIG_GPIO_Port GPIOC
#define EN_A_SIG_EXTI_IRQn EXTI15_10_IRQn
#define WiFi_Reset_Pin GPIO_PIN_3
#define WiFi_Reset_GPIO_Port GPIOB
#define SW2_Pin GPIO_PIN_4
#define SW2_GPIO_Port GPIOB
#define SW2_EXTI_IRQn EXTI4_IRQn
#define SW3_Pin GPIO_PIN_5
#define SW3_GPIO_Port GPIOB
#define SW3_EXTI_IRQn EXTI9_5_IRQn
#define PWM_SERVO_Pin GPIO_PIN_7
#define PWM_SERVO_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
