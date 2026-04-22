#pragma once

extern "C"
{
    #include "main.h"
}

//C++ includes
#include "app/servo_motor.hpp"
#include "app/helpers/ring_buffer.hpp"
#include "app/stm_packet.hpp"

#include <atomic>

class App
{
public:
    void Run(TIM_HandleTypeDef& htim1, UART_HandleTypeDef& huart1, SPI_HandleTypeDef& hspi2);
    void Loop();
    void Release();

    void GPIO_EXTI_Callback(uint16_t GPIO_Pin);
    void UART_RxCpltCallback(UART_HandleTypeDef *huart);
    void SPI_RxCpltCallback(SPI_HandleTypeDef *hspi);

private:
    void ProcessPacket();

private:
    ServoMotor m_servoMotor;

    std::atomic<bool> m_buttonPressed{false};
    
    uint8_t m_rxBuffer[Packet::PACKET_SIZE_PLUS_HEADER];
    uint8_t m_txBuffer[Packet::PACKET_SIZE_PLUS_HEADER];

    RingBuffer128 m_packetBuffer;

    UART_HandleTypeDef* m_huart1;
    SPI_HandleTypeDef* m_hspi2;
};
