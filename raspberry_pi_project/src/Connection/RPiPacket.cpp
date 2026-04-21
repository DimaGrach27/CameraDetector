#include "RPiPacket.h"

#include <cstdint>

std::ostream& operator<<(std::ostream& os, MessageTypes type)
{
    switch (type)
    {
        case MessageTypes::Command:
            os << "Command";
            break;

        case MessageTypes::Response:
            os << "Response";
            break;

        case MessageTypes::MessageError:
            os << "Error";
            break;

        default:
            os << "Unknown";
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, CommandTypes type)
{
    switch (type)
    {
        case CommandTypes::SetLED:
            os << "SetLED";
            break;

        case CommandTypes::Motion:
            os << "Motion";
            break;

        case CommandTypes::MotorLeft:
            os << "MotorLeft";
            break;

        case CommandTypes::MotorRight:
            os << "MotorRight";
            break;

        default:
            os << "Unknown";
    }

    return os;
}