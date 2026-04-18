#include "app.hpp"
#include "app/packet.hpp"

#include <cstdint>
#include <iostream>

extern UART_HandleTypeDef huart2;
#include <cstdio>
#include <cstring>

// int __io_putchar(int ch)
// {
//   uint8_t byte = (uint8_t)ch;
//   HAL_UART_Transmit(&huart2, &byte, 1, HAL_MAX_DELAY);
//   return ch;
// }

void App::Run(TIM_HandleTypeDef& htim1, UART_HandleTypeDef& huart1)
{
    m_huart1 = &huart1;

    m_servoMotor.Init(htim1);

    // HAL_UART_Receive_IT(m_huart1, &m_rxByte, 1);
    HAL_UART_Receive_IT(m_huart1, m_rxBuffer, Packet::PACKET_SIZE_PLUS_HEADER);
}

void App::Loop()
{
    if (m_buttonPressed)
    {
        m_buttonPressed = false;
        m_servoMotor.RunTestSequence();
    }

    if (m_packetBuffer.Size() >= Packet::PACKET_SIZE_PLUS_HEADER)
    {
        ProcessPacket();
    }

    static uint32_t lastTick = HAL_GetTick();
    const uint32_t now = HAL_GetTick();
    const uint32_t deltaMs = now - lastTick;
    lastTick = now;

    m_servoMotor.Update(deltaMs);
}

void App::Release()
{

}

void App::ProcessPacket()
{
    uint8_t headerByte = 0;
    if (!m_packetBuffer.Pop(headerByte))
    {
        return;
    }

    if (!Packet::IsHeader(headerByte))
    {
        printf("Not a header byte. Byte:%02X\r\n", headerByte);
        return;
    }

    uint8_t bufferedPacket[Packet::PACKET_SIZE];

    for (uint8_t i = 0; i < Packet::PACKET_SIZE; ++i)
    {
        if (!m_packetBuffer.Pop(bufferedPacket[i]))
        {
            printf("Failed to pop byte from packet buffer\r\n");
            return;
        }
    }

    uint32_t packet = Packet::BytesToPacket(bufferedPacket);

    printf("raw:%02X %02X %02X %02X packet:%08lX cmd:%u val:%u valid:%u\r\n",
         bufferedPacket[0], bufferedPacket[1], bufferedPacket[2], bufferedPacket[3],
         (unsigned long)packet,
         (unsigned)Packet::GetCommandType(packet),
         (unsigned)Packet::GetValue(packet),
         (unsigned)Packet::ValidatePacket(packet));


    if (!Packet::ValidatePacket(packet))
    {
        return;
    }

    MessageTypes messageType = Packet::GetMessageType(packet);
    if (messageType != MessageTypes::Command)
    {
        return; // Only process command messages
    }

    switch (Packet::GetCommandType(packet))
    {
        case CommandTypes::SetLED:
            if (Packet::GetValue(packet) == 1)
            {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            }
            else
            {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
            }
            break;
        case CommandTypes::MotorLeft:
        case CommandTypes::MotorRight:
            m_servoMotor.RunCommand(packet);
            break;
        default:
            break;
    
    }
}

void App::GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_13)
    {
        m_buttonPressed = true;
    }
}

void App::UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // m_packetBuffer.Push(m_rxByte);
        // HAL_UART_Receive_IT(huart, &m_rxByte, 1);

        for (int i = 0; i < Packet::PACKET_SIZE_PLUS_HEADER; ++i)
        {
            m_packetBuffer.Push(m_rxBuffer[i]);
        }

        if (HAL_UART_Receive_IT(huart, m_rxBuffer, Packet::PACKET_SIZE_PLUS_HEADER) != HAL_OK)
        {
            // Handle error: could retry, log, or set an error flag
            // Example: retry once (optional)
            HAL_UART_Receive_IT(huart, m_rxBuffer, Packet::PACKET_SIZE_PLUS_HEADER);
        }
    }
}