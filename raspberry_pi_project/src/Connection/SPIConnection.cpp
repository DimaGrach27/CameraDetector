#include "SPIConnection.h"

#include <iostream>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>

#include <fcntl.h>
#include <iomanip>
#include <ostream>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "protocol/Packet.h"

void SPIConnection::Init()
{
    std::cout << "SPIConnection Init" << std::endl;

    if (!Open("/dev/spidev0.0", 100000, SPI_MODE_0))
    {
        std::perror("Can't open /dev/spi");
    }
}

void SPIConnection::SendPacket(const PacketStruct packetStruct)
{
    uint32_t txData = 0;
    if (!Packet::MakePacket(packetStruct, txData))
    {
        std::perror("Can't make packet");
        return;
    }

    uint8_t buffer[Packet::PACKET_SIZE_PLUS_HEADER];
    Packet::PacketToBytes(txData, buffer);

    if (!Send(buffer, Packet::PACKET_SIZE_PLUS_HEADER))
    {
        std::cout << "Can't send packet!" << std::endl;
        return;
    }

    std::cout << "Sent packadge: " << packetStruct.message_type << " CM: " << packetStruct.command_type << " Value: " << static_cast<int>(packetStruct.value) << std::endl;

    std::cout << "Packet bytes: ";
    for (int i = 0; i < Packet::PACKET_SIZE_PLUS_HEADER; ++i)
    {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[i]);
        if (i + 1 < Packet::PACKET_SIZE_PLUS_HEADER)
            std::cout << " ";
    }
    std::cout << std::dec << std::endl;

}

bool SPIConnection::Open(const char* devicePath, uint32_t speedHz, uint8_t mode)
{
    m_fd = open(devicePath, O_RDWR);
    if (m_fd < 0)
    {
        std::perror("open spi");
        return false;
    }

    m_speedHz = speedHz;
    m_mode = mode;
    m_bitsPerWord = 8;
    m_lsb = 0;

    if (ioctl(m_fd, SPI_IOC_WR_MODE, &m_mode) < 0)
    {
        std::perror("SPI_IOC_WR_MODE");
        Close();
        return false;
    }

    if (ioctl(m_fd, SPI_IOC_RD_MODE, &m_mode) < 0)
    {
        std::perror("SPI_IOC_RD_MODE");
        Close();
        return false;
    }

    if (ioctl(m_fd, SPI_IOC_WR_BITS_PER_WORD, &m_bitsPerWord) < 0)
    {
        std::perror("SPI_IOC_WR_BITS_PER_WORD");
        Close();
        return false;
    }

    if (ioctl(m_fd, SPI_IOC_RD_BITS_PER_WORD, &m_bitsPerWord) < 0)
    {
        std::perror("SPI_IOC_RD_BITS_PER_WORD");
        Close();
        return false;
    }

    if (ioctl(m_fd, SPI_IOC_WR_MAX_SPEED_HZ, &m_speedHz) < 0)
    {
        std::perror("SPI_IOC_WR_MAX_SPEED_HZ");
        Close();
        return false;
    }

    if (ioctl(m_fd, SPI_IOC_RD_MAX_SPEED_HZ, &m_speedHz) < 0)
    {
        std::perror("SPI_IOC_RD_MAX_SPEED_HZ");
        Close();
        return false;
    }

    if (ioctl(m_fd, SPI_IOC_WR_LSB_FIRST, &m_lsb) < 0)
    {
        std::perror("SPI_IOC_WR_LSB_FIRST");
        Close();
        return false;
    }

    if (ioctl(m_fd, SPI_IOC_RD_LSB_FIRST, &m_lsb) < 0)
    {
        std::perror("SPI_IOC_RD_LSB_FIRST");
        Close();
        return false;
    }

    printf("SPI CONFIG:\n");
    printf("mode: %u\n", m_mode);
    printf("bits: %u\n", m_bitsPerWord);
    printf("speed: %u\n", m_speedHz);
    printf("LSB first: %u\n", m_lsb);
    return true;
}

void SPIConnection::Close()
{
    if (m_fd >= 0)
    {
        close(m_fd);
        m_fd = -1;
    }
}

bool SPIConnection::Transfer(const uint8_t* txData, uint8_t* rxData, size_t size)
{
    if (m_fd < 0 || txData == nullptr || size == 0)
    {
        return false;
    }

    spi_ioc_transfer transfer{};
    transfer.tx_buf = reinterpret_cast<unsigned long>(txData);
    transfer.rx_buf = reinterpret_cast<unsigned long>(rxData);
    transfer.len = static_cast<uint32_t>(size);
    transfer.speed_hz = m_speedHz;
    transfer.bits_per_word = m_bitsPerWord;
    transfer.delay_usecs = 0;

    printf("Start sending\n");
    for (uint32_t i = 0; i < size; ++i)
    {
        printf("%02X", txData[i]);
    }
    printf("\n");
    if (ioctl(m_fd, SPI_IOC_MESSAGE(1), &transfer) < 0)
    {
        std::perror("SPI_IOC_MESSAGE");
        return false;
    }

    printf("End sending\n");

    return true;
}

bool SPIConnection::Send(const uint8_t* txData, size_t size)
{
    if (m_fd < 0 || txData == nullptr || size == 0)
    {
        return false;
    }

    uint8_t dummyRx[256]{};

    if (size > sizeof(dummyRx))
    {
        return false;
    }

    return Transfer(txData, dummyRx, size);
}

void SPIConnection::TestSend()
{
    uint8_t tx[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t rx[5] = {};

    Send(tx, 5);
    sleep(1);

    uint8_t tx2[5] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    Send(tx2, 5);
    sleep(1);

    uint8_t tx3[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
    Send(tx3, 5);
    sleep(1);

    uint8_t tx4[5] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
    Send(tx4, 5);
    sleep(1);

    uint8_t tx5[5] = {0x55, 0x55, 0x55, 0x55, 0x55};
    Send(tx5, 5);
    sleep(1);
}