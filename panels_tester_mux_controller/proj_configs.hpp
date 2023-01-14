#pragma once
#include <type_traits>

namespace ProjCfg
{
using CommandSubtype = uint8_t;

enum Memory {
    EEPROMAddressI2CBoardAddress = 0
};
enum I2C {
    DefaultAddress = 32
};

enum class Command : CommandSubtype {
    SetOwnAddress = 0xc5,
    TestDataLink = 0xc6
};

enum Communications : CommandSubtype {
    OK   = 0xa5,
    FAIL = 0x50
};
}