#pragma once

#include <iostream>

#include "protocol/Packet.h"

std::ostream& operator<<(std::ostream& os, MessageTypes type);
std::ostream& operator<<(std::ostream& os, CommandTypes type);
