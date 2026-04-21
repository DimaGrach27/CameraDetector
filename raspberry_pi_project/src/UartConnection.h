#pragma once

#include "Packet.h"

#include <cstdint>

class UartConnection
{
public:
    void Init();
    void SendPacket(const uint8_t device_id, const MessageTypes message_type, const CommandTypes command_type, const uint8_t value);

private:
    int m_port; //fd
};