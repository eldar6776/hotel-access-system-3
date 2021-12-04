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
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include "stm32f7xx_hal.h"

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
#define LCD_DISP_Pin GPIO_PIN_2
#define LCD_DISP_GPIO_Port GPIOE
#define TS_INT_Pin GPIO_PIN_3
#define TS_INT_GPIO_Port GPIOE
#define LCD_DE_Pin GPIO_PIN_4
#define LCD_DE_GPIO_Port GPIOE
#define LCD_BL_CTRL_Pin GPIO_PIN_5
#define LCD_BL_CTRL_GPIO_Port GPIOE
#define LCD_RST_Pin GPIO_PIN_13
#define LCD_RST_GPIO_Port GPIOC
#define INDOOR_TEMP_Pin GPIO_PIN_1
#define INDOOR_TEMP_GPIO_Port GPIOC
#define FANCOIL_TEMP_Pin GPIO_PIN_2
#define FANCOIL_TEMP_GPIO_Port GPIOC
#define AMBIENT_LIGHT_Pin GPIO_PIN_3
#define AMBIENT_LIGHT_GPIO_Port GPIOC
#define EEPROM_NCS_Pin GPIO_PIN_2
#define EEPROM_NCS_GPIO_Port GPIOG
#define NRF24L01_CE_Pin GPIO_PIN_3
#define NRF24L01_CE_GPIO_Port GPIOG
#define FAN_TRIAC_Pin GPIO_PIN_8
#define FAN_TRIAC_GPIO_Port GPIOC
#define RS485_TX_Pin GPIO_PIN_9
#define RS485_TX_GPIO_Port GPIOA
#define RS485_RX_Pin GPIO_PIN_10
#define RS485_RX_GPIO_Port GPIOA
#define RS485_DIR_Pin GPIO_PIN_12
#define RS485_DIR_GPIO_Port GPIOA
#define FAN_LOW_Pin GPIO_PIN_10
#define FAN_LOW_GPIO_Port GPIOC
#define FAN_MIDDLE_Pin GPIO_PIN_11
#define FAN_MIDDLE_GPIO_Port GPIOC
#define FAN_HIGH_Pin GPIO_PIN_12
#define FAN_HIGH_GPIO_Port GPIOC
#define COOLING_VL_Pin GPIO_PIN_2
#define COOLING_VL_GPIO_Port GPIOD
#define HEATING_VL_Pin GPIO_PIN_4
#define HEATING_VL_GPIO_Port GPIOD
#define ONE_WIRE_TX_Pin GPIO_PIN_5
#define ONE_WIRE_TX_GPIO_Port GPIOD
#define ONE_WIRE_RX_Pin GPIO_PIN_6
#define ONE_WIRE_RX_GPIO_Port GPIOD
#define STATUS_LED_Pin GPIO_PIN_7
#define STATUS_LED_GPIO_Port GPIOD
#define FANCOIL_ZC_Pin GPIO_PIN_13
#define FANCOIL_ZC_GPIO_Port GPIOG
#define FANCOIL_RPM_Pin GPIO_PIN_14
#define FANCOIL_RPM_GPIO_Port GPIOG
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
