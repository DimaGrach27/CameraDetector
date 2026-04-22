#pragma once
#include "IConnection.h"

#include <cstdint>
#include <linux/spi/spi.h>

#include "RPiPacket.h"

class SPIConnection : public IConnection
{
public:
    void Init() override;
    void SendPacket(const PacketStruct packetStruct) override;
    void TestSend();

private:
    bool Open(const char* devicePath, uint32_t speedHz, uint8_t mode = SPI_MODE_0);
    void Close();

    bool Transfer(const uint8_t* txData, uint8_t* rxData, size_t size);
    bool Send(const uint8_t* txData, size_t size);

private:
    int m_fd = -1;
    uint32_t m_speedHz = 0;
    uint8_t m_mode = SPI_MODE_0;
    uint8_t m_bitsPerWord = 8;
    uint8_t m_lsb = 0;
};
