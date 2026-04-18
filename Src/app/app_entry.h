#pragma once

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

void App_Run(TIM_HandleTypeDef* htim1, UART_HandleTypeDef* huart1);
void App_Loop(void);
void App_Release(void);
void App_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void App_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void App_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);

#ifdef __cplusplus
}
#endif