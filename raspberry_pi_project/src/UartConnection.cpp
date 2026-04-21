#include "UartConnection.h"

#include "Packet.h"

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <cstdint>
#include <iostream>
#include <iomanip>

void UartConnection::Init()
{
    m_port = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY);

    termios tty{};
    tcgetattr(m_port, &tty);

    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | CSTOPB | CRTSCTS);

    tcsetattr(m_port, TCSANOW, &tty);
}

void UartConnection::SendPacket(const uint8_t device_id, const MessageTypes message_type, const CommandTypes command_type, const uint8_t value)
{
    uint32_t packet = 0;
    uint8_t buffer[Packet::PACKET_SIZE_PLUS_HEADER];

    Packet::MakePacket(device_id, message_type, command_type, value, packet);
    Packet::PacketToBytes(packet, buffer);

    std::cout << "Sent packadge: " << message_type << " CM: " << command_type << " Value: " << static_cast<int>(value) << std::endl;
    
    std::cout << "Packet bytes: ";
    for (int i = 0; i < Packet::PACKET_SIZE_PLUS_HEADER; ++i)
    {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[i]);
        if (i + 1 < Packet::PACKET_SIZE_PLUS_HEADER)
            std::cout << " ";
    }
    std::cout << std::dec << std::endl;

    // for (uint8_t i = 0; i < Packet::PACKET_SIZE_PLUS_HEADER; i++)
    // {
    //     write(m_port, &buffer[i], 1);
    // }
    write(m_port, &buffer, Packet::PACKET_SIZE_PLUS_HEADER);
}