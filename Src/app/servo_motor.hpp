#pragma once

extern "C"
{
#include "main.h"
}

//C++ includes
#include <cstdint>

class ServoMotor
{
public:
    void Init(TIM_HandleTypeDef& htim1);
    void Update(uint32_t deltaMs);
    void RunCommand(uint32_t command);

    void RunTestSequence();

    uint32_t GetCurrentPulse() const;
    void SetPulse(uint32_t pulse);

    int32_t GetCurrentAngle() const;
    void SetAngle(int32_t angle);

private:
    uint32_t PulseFromAngle(int32_t angle) const;
    int32_t AngleFromPulse(uint32_t pulse) const;
    uint32_t PulseStepFromAngleDelta(int32_t angleDelta) const;
    void SetTargetPulse(uint32_t targetPulse);

private:
    static constexpr uint32_t SERVO_MIN_PULSE_US = 500U;
    static constexpr uint32_t SERVO_MAX_PULSE_US = 2500U;

    TIM_HandleTypeDef* m_htim;
    uint32_t m_currentPulse = 1500; // центр
    uint32_t m_targetPulse = 1500;

    uint32_t m_speedDegPerSec = 60;
};