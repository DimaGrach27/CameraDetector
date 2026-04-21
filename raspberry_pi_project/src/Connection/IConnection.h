#pragma once

#include "RPiPacket.h"

class IConnection
{
public:
    virtual void Init() { }
    virtual void SendPacket(const PacketStruct packetStruct) = 0;
};