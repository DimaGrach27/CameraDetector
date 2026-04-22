#include "app.hpp"
#include "app/stm_packet.hpp"
#include "protocol/Packet.h"
#include "stm32f4xx_hal_def.h"

#include <cstdint>
#include <iostream>

extern UART_HandleTypeDef huart2;
#include <cstdio>
#include <cstring>

void App::Run(TIM_HandleTypeDef& htim1, UART_HandleTypeDef& huart1, SPI_HandleTypeDef& hspi2)
{
    m_huart1 = &huart1;
    m_hspi2 = &hspi2;

    m_servoMotor.Init(htim1);

    // HAL_UART_Receive_IT(m_huart1, &m_rxByte, 1);
    HAL_StatusTypeDef spiStatus = HAL_SPI_Receive_IT(m_hspi2, m_rxBuffer, Packet::PACKET_SIZE_PLUS_HEADER);
    // HAL_StatusTypeDef spiStatus = HAL_SPI_TransmitReceive_IT(m_hspi2, m_txBuffer, m_rxBuffer, Packet::PACKET_SIZE_PLUS_HEADER);
    
    if (spiStatus != HAL_OK)
    {
        printf("SPI error receive, status = %d", spiStatus);
    }

    printf("====APP started====\n");
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
    while (m_packetBuffer.Size() > 0)
    {
        uint8_t headerByte = 0;
        if (!m_packetBuffer.Pick(headerByte))
        {
            return;
        }

        printf("Header = %02X \n", headerByte);

        if (Packet::IsHeader(headerByte))
        {
            break;
        }

        m_packetBuffer.Pop(headerByte);
    }

    if (m_packetBuffer.Size() < Packet::PACKET_SIZE_PLUS_HEADER)
    {
        return;
    }

    uint8_t headerByte = 0;
    if (!m_packetBuffer.Pop(headerByte))
    {
        return;
    }

    uint8_t bufferedPacket[Packet::PACKET_SIZE];
    for (uint8_t i = 0; i < Packet::PACKET_SIZE; ++i)
    {
        if (!m_packetBuffer.Pop(bufferedPacket[i]))
        {
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
            if (Packet::GetValue(packet) == 0)
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

    if (GPIO_Pin == PIR_Sensor_Pin)
    {
        m_motionDetected = HAL_GPIO_ReadPin(PIR_Sensor_GPIO_Port, PIR_Sensor_Pin);

        HAL_GPIO_WritePin(PIR_Sensor_LED_GPIO_Port, PIR_Sensor_LED_Pin, m_motionDetected ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

void App::UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // if (huart->Instance == USART1)
    // {
    //     m_packetBuffer.Push(m_rxByte);

    //     if (HAL_UART_Receive_IT(huart, &m_rxByte, 1) != HAL_OK)
    //     {
    //         HAL_UART_Receive_IT(huart, &m_rxByte, 1);
    //     }
    // }
}

void App::SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI2)
    {
        for (uint8_t i = 0; i < Packet::PACKET_SIZE_PLUS_HEADER; i++)
        {
            m_packetBuffer.Push(m_rxBuffer[i]);
        }
        
        // m_packetBuffer.Push(m_rxByte);
        
        if (HAL_SPI_Receive_IT(m_hspi2, m_rxBuffer, Packet::PACKET_SIZE_PLUS_HEADER) != HAL_OK)
        {
            HAL_SPI_Receive_IT(m_hspi2, m_rxBuffer, Packet::PACKET_SIZE_PLUS_HEADER);
        }
    }
}
