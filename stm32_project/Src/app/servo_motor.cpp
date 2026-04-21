#include "app/servo_motor.hpp"

extern "C"
{
#include "main.h"
#include "stm32f4xx_hal.h"
}

extern UART_HandleTypeDef huart2;
#include <cstdio>
#include <cstring>
#include <cstdint>

#include "app/packet.hpp"

// int __io_putchar(int ch)
// {
//   uint8_t byte = (uint8_t)ch;
//   HAL_UART_Transmit(&huart2, &byte, 1, HAL_MAX_DELAY);
//   return ch;
// }

void ServoMotor::Init(TIM_HandleTypeDef& htim1)
{
    m_htim = &htim1;

    m_currentPulse = PulseFromAngle(90);
    m_targetPulse = m_currentPulse;
    SetPulse(m_currentPulse);

    // RunTestSequence();
}

void ServoMotor::Update(uint32_t deltaMs)
{
    if (m_currentPulse == m_targetPulse)
    {
        return;
    }

    const uint32_t maxStep = PulseFromAngle(m_speedDegPerSec) * (deltaMs / 1000.0f);

    if (m_currentPulse < m_targetPulse)
    {
        m_currentPulse += maxStep;

        if (m_currentPulse > m_targetPulse)
        {
            m_currentPulse = m_targetPulse;
        }
    }
    else
    {
        m_currentPulse -= maxStep;

        if (m_currentPulse < m_targetPulse)
        {
            m_currentPulse = m_targetPulse;
        }
    }

    SetPulse(m_currentPulse);
}

void ServoMotor::RunCommand(uint32_t command)
{
    const uint32_t currentPulse = m_targetPulse;
    // const uint32_t currentPulse = GetCurrentPulse();
    const int32_t valueAngle = Packet::GetValue(command);
    const uint32_t pulseStep = PulseStepFromAngleDelta(valueAngle);

    uint32_t nextPulse = currentPulse;

    CommandTypes commandType = Packet::GetCommandType(command);
    if (commandType == CommandTypes::MotorLeft)
    {
        if (nextPulse > SERVO_MIN_PULSE_US + pulseStep)
        {
            nextPulse -= pulseStep;
        }
        else
        {
            nextPulse = SERVO_MIN_PULSE_US;
        }
    }
    else if (commandType == CommandTypes::MotorRight)
    {
        if (nextPulse + pulseStep < SERVO_MAX_PULSE_US)
        {
            nextPulse += pulseStep;
        }
        else
        {
            nextPulse = SERVO_MAX_PULSE_US;
        }
    }

    printf("RunCommand cmd:%u currentPulse:%lu nextPulse:%lu step:%lu currentAngle:%ld nextAngle:%ld value:%u\r\n",
         (unsigned)commandType,
         (unsigned long)currentPulse,
         (unsigned long)nextPulse,
         (unsigned long)pulseStep,
         (long)AngleFromPulse(currentPulse),
         (long)AngleFromPulse(nextPulse),
         (unsigned)Packet::GetValue(command));

    SetTargetPulse(nextPulse);
}

void ServoMotor::RunTestSequence()
{
    uint32_t angle = 0;
    if (Packet::MakePacket(0, MessageTypes::Command, CommandTypes::MotorLeft, 10, angle))
    {
        RunCommand(angle);
        return;
    }
    
    if (Packet::MakePacket(0, MessageTypes::Command, CommandTypes::MotorLeft, 10, angle))
    {
        RunCommand(angle);
        HAL_Delay(1000);
    }

    if (Packet::MakePacket(0, MessageTypes::Command, CommandTypes::MotorLeft, 10, angle))
    {
        RunCommand(angle);
        HAL_Delay(1000);
    }

    if (Packet::MakePacket(0, MessageTypes::Command, CommandTypes::MotorLeft, 10, angle))
    {
        RunCommand(angle);
        HAL_Delay(1000);

    }

    if (Packet::MakePacket(0, MessageTypes::Command, CommandTypes::MotorRight, 10, angle))
    {
        RunCommand(angle);
        HAL_Delay(1000);
    }

    if (Packet::MakePacket(0, MessageTypes::Command, CommandTypes::MotorRight, 10, angle))
    {
        RunCommand(angle);
        HAL_Delay(1000);
    }

    if (Packet::MakePacket(0, MessageTypes::Command, CommandTypes::MotorRight, 10, angle))
    {
        RunCommand(angle);
        HAL_Delay(1000);
    }
}

uint32_t ServoMotor::GetCurrentPulse() const
{
    return m_currentPulse;
}

void ServoMotor::SetPulse(uint32_t pulse)
{
    if (pulse < SERVO_MIN_PULSE_US)
    {
        pulse = SERVO_MIN_PULSE_US;
    }
    else if (pulse > SERVO_MAX_PULSE_US)
    {
        pulse = SERVO_MAX_PULSE_US;
    }

    m_currentPulse = pulse;
    __HAL_TIM_SET_COMPARE(m_htim, TIM_CHANNEL_1, m_currentPulse);
}

int32_t ServoMotor::GetCurrentAngle() const
{
    return AngleFromPulse(m_currentPulse);
}

void ServoMotor::SetAngle(int32_t angle)
{
  SetPulse(PulseFromAngle(angle));
}

uint32_t ServoMotor::PulseFromAngle(int32_t angle) const
{
    if (angle < 0)
    {
        angle = 0;
    }
    else if (angle > 180)
    {
        angle = 180;
    }

    return SERVO_MIN_PULSE_US +
           (uint32_t)((SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US) * (uint32_t)angle / 180U);
}

int32_t ServoMotor::AngleFromPulse(uint32_t pulse) const
{
    if (pulse < SERVO_MIN_PULSE_US)
    {
        pulse = SERVO_MIN_PULSE_US;
    }
    else if (pulse > SERVO_MAX_PULSE_US)
    {
        pulse = SERVO_MAX_PULSE_US;
    }

    return (int32_t)(((pulse - SERVO_MIN_PULSE_US) * 180U) /
                     (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US));
}

uint32_t ServoMotor::PulseStepFromAngleDelta(int32_t angleDelta) const
{
    if (angleDelta < 0)
    {
        angleDelta = -angleDelta;
    }

    return (uint32_t)(((SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US) * (uint32_t)angleDelta) / 180U);
}

void ServoMotor::SetTargetPulse(uint32_t targetPulse)
{
    m_targetPulse = targetPulse;
}