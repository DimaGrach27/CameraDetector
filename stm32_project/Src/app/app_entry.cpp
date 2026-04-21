#include "app/app_entry.h"

#include "app/app.hpp"

static App app;

extern "C" void App_Run(TIM_HandleTypeDef* htim1, UART_HandleTypeDef* huart1, SPI_HandleTypeDef* hspi2)
{
    app.Run(*htim1, *huart1, *hspi2);
}

extern "C" void App_Loop(void)
{
    app.Loop();
}

extern "C" void App_Release(void)
{
    app.Release();
}

extern "C" void App_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    app.GPIO_EXTI_Callback(GPIO_Pin);
}

extern "C" void App_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    app.UART_RxCpltCallback(huart);
}

extern "C" void App_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    // app.UARTEx_RxEventCallback(huart, Size);
}

extern "C" void App_SPI_RxHalfCpltCallback(SPI_HandleTypeDef *hspi)
{
    app.SPI_RxHalfCpltCallback(hspi);
}