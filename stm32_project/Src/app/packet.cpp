#include "app/packet.hpp"

#include <cstdint>

uint8_t Packet::CalculateChecksum(const uint32_t packet)
{
    uint8_t sum = 0;
    sum += ((packet >> DEVICE_ID_SHIFT) & DEVICE_ID_MASK);
    sum += ((packet >> MESSAGE_TYPE_SHIFT) & MESSAGE_TYPE_MASK);
    sum += ((packet >> COMMAND_TYPE_SHIFT) & COMMAND_TYPE_MASK);
    sum += ((packet >> VALUE_SHIFT) & VALUE_MASK);
    sum += ((packet >> RESERVED_SHIFT) & RESERVED_MASK);

    return sum;
}

bool Packet::ValidatePacketFields(const uint8_t device_id, const MessageTypes message_type, const CommandTypes command_type, const uint8_t value)
{
    if (device_id > DEVICE_ID_MASK)
    {
        return false;
    }
    
    if (message_type >= MessageTypes::Count)
    {
        return false;
    }
    
    if (command_type >= CommandTypes::Count)
    {
        return false;
    }
    
    if (command_type == CommandTypes::SetLED && value > 1)
    {
        return false;
    }
    
    if ((message_type == MessageTypes::Command && command_type == CommandTypes::Motion && value != 0)
        || (message_type == MessageTypes::Response && command_type == CommandTypes::Motion && value > 1))
    {
        return false;
    }
    
    if ((command_type == CommandTypes::MotorLeft || command_type == CommandTypes::MotorRight) && value > 180)
    {
        return false;
    }
    
    return true;
}

bool Packet::MakePacket(const uint8_t device_id, const MessageTypes message_type, const CommandTypes command_type, const uint8_t value, uint32_t& out_packet)
{
    // if (out_packet == nullptr)
    // {
    //     return false; // Invalid output pointer
    // }

    if (!ValidatePacketFields(device_id, message_type, command_type, value))
    {
        return false; // Invalid packet
    }

    out_packet = 0;
    out_packet |= ((device_id & DEVICE_ID_MASK) << DEVICE_ID_SHIFT);
    out_packet |= ((static_cast<uint8_t>(message_type) & MESSAGE_TYPE_MASK) << MESSAGE_TYPE_SHIFT);
    out_packet |= ((static_cast<uint8_t>(command_type) & COMMAND_TYPE_MASK) << COMMAND_TYPE_SHIFT);
    out_packet |= ((value & VALUE_MASK) << VALUE_SHIFT);

    uint8_t checksum_value = CalculateChecksum(out_packet);
    out_packet |= ((checksum_value & CHECKSUM_MASK) << CHECKSUM_SHIFT);

    return true; // Packet created successfully
}

bool Packet::IsChecksumValid(const uint32_t packet)
{
    uint8_t expected_checksum = GetChecksum(packet);
    uint8_t calculated_checksum = CalculateChecksum(packet);

    return expected_checksum == calculated_checksum;
}

bool Packet::ValidatePacket(const uint32_t packet)
{
    return IsChecksumValid(packet) && ValidatePacketFields(GetDeviceId(packet), GetMessageType(packet), GetCommandType(packet), GetValue(packet));
}

void Packet::PacketToBytes(const uint32_t packet, uint8_t* out_bytes)
{
    out_bytes[0] = HEADER_BYTE;
    out_bytes[1] = packet & 0xFF;
    out_bytes[2] = (packet >> 8) & 0xFF;
    out_bytes[3] = (packet >> 16) & 0xFF;
    out_bytes[4] = (packet >> 24) & 0xFF;
}

uint32_t Packet::BytesToPacket(const uint8_t* bytes)
{
    return bytes[0] 
        | (bytes[1] << 8) 
        | (bytes[2] << 16) 
        | (bytes[3] << 24);
}