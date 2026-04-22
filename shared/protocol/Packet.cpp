#include "Packet.h"

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

bool Packet::ValidatePacketFields(const PacketStruct packetStruct)
{
    if (packetStruct.device_id > DEVICE_ID_MASK)
    {
        return false;
    }
    
    if (packetStruct.message_type >= MessageTypes::Count)
    {
        return false;
    }
    
    if (packetStruct.command_type >= CommandTypes::Count)
    {
        return false;
    }
    
    if (packetStruct.command_type == CommandTypes::SetLED && packetStruct.value > 1)
    {
        return false;
    }
    
    if ((packetStruct.message_type == MessageTypes::Command && packetStruct.command_type == CommandTypes::Motion && packetStruct.value != 0)
        || (packetStruct.message_type == MessageTypes::Response && packetStruct.command_type == CommandTypes::Motion && packetStruct.value > 1))
    {
        return false;
    }
    
    if ((packetStruct.command_type == CommandTypes::MotorLeft || packetStruct.command_type == CommandTypes::MotorRight) && packetStruct.value > 180)
    {
        return false;
    }
    
    return true;
}

bool Packet::MakePacket(const PacketStruct packetStruct, uint32_t& out_packet)
{
    // if (out_packet == nullptr)
    // {
    //     return false; // Invalid output pointer
    // }

    if (!ValidatePacketFields(packetStruct))
    {
        return false; // Invalid packet
    }

    out_packet = 0;
    out_packet |= ((packetStruct.device_id & DEVICE_ID_MASK) << DEVICE_ID_SHIFT);
    out_packet |= ((static_cast<uint8_t>(packetStruct.message_type) & MESSAGE_TYPE_MASK) << MESSAGE_TYPE_SHIFT);
    out_packet |= ((static_cast<uint8_t>(packetStruct.command_type) & COMMAND_TYPE_MASK) << COMMAND_TYPE_SHIFT);
    out_packet |= ((packetStruct.value & VALUE_MASK) << VALUE_SHIFT);

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
    return IsChecksumValid(packet) && ValidatePacketFields(PacketStruct{GetDeviceId(packet), GetMessageType(packet), GetCommandType(packet), GetValue(packet)});
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