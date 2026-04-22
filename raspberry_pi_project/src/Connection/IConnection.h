#pragma once

#include "RPiPacket.h"

class IConnection
{
public:
    virtual ~IConnection() = default;

    virtual void Init() { }
    virtual void SendPacket(const PacketStruct packetStruct) = 0;
};