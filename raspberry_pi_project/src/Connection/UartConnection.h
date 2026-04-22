#pragma once

#include "RPiPacket.h"
#include "IConnection.h"

#include <cstdint>

class UartConnection : public IConnection
{
public:
    void Init() override;
    void SendPacket(const PacketStruct packetStruct) override;

private:
    int m_port; //fd
};