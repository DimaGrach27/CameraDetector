#pragma once

extern "C"
{
    #include "main.h"
}

//C++ includes
#include "app/servo_motor.hpp"
#include "app/helpers/ring_buffer.hpp"
#include "app/packet.hpp"

#include <atomic>

class App
{
public:
    void Run(TIM_HandleTypeDef& htim1, UART_HandleTypeDef& huart1);
    void Loop();
    void Release();
    void GPIO_EXTI_Callback(uint16_t GPIO_Pin);
    void UART_RxCpltCallback(UART_HandleTypeDef *huart);

    void Uart_StartDmaReception();
    void Uart_FetchFromDmaToRingBuffer();

private:
    void ProcessPacket();
private:
    ServoMotor m_servoMotor;

    std::atomic<bool> m_buttonPressed{false};
    std::atomic<bool> m_packetReceived{false};
    
    static constexpr size_t UartDmaRxBufferSize = 256;

    uint8_t g_uartDmaRxBuffer[UartDmaRxBufferSize];
    volatile size_t g_uartDmaLastPos = 0;
    
    uint8_t m_rxBuffer[Packet::PACKET_SIZE_PLUS_HEADER];
    uint8_t m_rxByte;

    RingBuffer128 m_packetBuffer;

    UART_HandleTypeDef* m_huart1;

};