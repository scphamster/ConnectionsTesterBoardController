#pragma once
#include <type_traits>

namespace ProjCfg
{
using CommandSubtype = uint8_t;
using Byte = uint8_t;

enum Memory {
    EEPROMAddressI2CBoardAddress = 0
};
enum I2C {
    DefaultAddress = 32
};

enum class Command : CommandSubtype {
    ChangeAddress = 0xc5,
    TestDataLink = 0xc6
};

enum Communications : CommandSubtype {
    OK   = 0xa5,
    REPEAT_CMD_TO_CONFIRM = 0xAF,
    FAIL = 0x50
};


Byte constexpr NUMBER_OF_CONFIRMATIONS_TO_CHANGE_ADDRESSS = 3;
}