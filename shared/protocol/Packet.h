#pragma once

#include <cstdint>

enum class MessageTypes : uint8_t
{
    Command = 0x00,
    Response = 0x01,
    MessageError = 0x02,
    Count = 0x03
};

enum class CommandTypes : uint8_t
{
    SetLED = 0x00, //value 0 - off, 1 - on
    Motion = 0x01, //request value = 0, response 0 or 1
    MotorLeft = 0x02, //value 0...180
    MotorRight = 0x03, //value 0...180
    Count = 0x04
};

struct PacketStruct
{
    const uint8_t device_id; 
    const MessageTypes message_type; 
    const CommandTypes command_type;
    const uint8_t value;
};

class Packet
{
public:
    static constexpr uint8_t HEADER_BYTE = 0XAA;
    static constexpr uint8_t PACKET_SIZE = 4;
    static constexpr uint8_t PACKET_SIZE_PLUS_HEADER = 5;

public:
static inline bool IsHeader(const uint8_t byte)
{
    return byte == HEADER_BYTE;
}

static inline uint8_t GetChecksum(const uint32_t packet)
{
    return (packet >> CHECKSUM_SHIFT) & CHECKSUM_MASK;
}

static inline uint8_t GetDeviceId(const uint32_t packet)
{
    return (packet >> DEVICE_ID_SHIFT) & DEVICE_ID_MASK;
}

static inline MessageTypes GetMessageType(const uint32_t packet)
{
    return static_cast<MessageTypes>((packet >> MESSAGE_TYPE_SHIFT) & MESSAGE_TYPE_MASK);
}

static inline CommandTypes GetCommandType(const uint32_t packet)
{
    return static_cast<CommandTypes>((packet >> COMMAND_TYPE_SHIFT) & COMMAND_TYPE_MASK);
}

static inline uint8_t GetValue(const uint32_t packet)
{
    return (packet >> VALUE_SHIFT) & VALUE_MASK;
}

static uint8_t CalculateChecksum(const uint32_t packet);

static bool ValidatePacketFields(const PacketStruct packetStruct);
static bool MakePacket(const PacketStruct packetStruct, uint32_t& out_packet);
static bool IsChecksumValid(const uint32_t packet);
static bool ValidatePacket(const uint32_t packet);

static void PacketToBytes(const uint32_t packet, uint8_t* out_bytes);
static uint32_t BytesToPacket(const uint8_t* bytes);

private:
    static constexpr uint8_t DEVICE_ID_MASK = 0x03;
    static constexpr uint8_t MESSAGE_TYPE_MASK = 0x03;
    static constexpr uint8_t COMMAND_TYPE_MASK = 0x0F;
    static constexpr uint8_t VALUE_MASK = 0xFF;
    static constexpr uint8_t RESERVED_MASK = 0xFF;
    static constexpr uint8_t CHECKSUM_MASK = 0xFF;

    static constexpr uint8_t DEVICE_ID_SHIFT = 0;
    static constexpr uint8_t MESSAGE_TYPE_SHIFT = 2;
    static constexpr uint8_t COMMAND_TYPE_SHIFT = 4;
    static constexpr uint8_t VALUE_SHIFT = 8;
    static constexpr uint8_t RESERVED_SHIFT = 16;
    static constexpr uint8_t CHECKSUM_SHIFT = 24;
};