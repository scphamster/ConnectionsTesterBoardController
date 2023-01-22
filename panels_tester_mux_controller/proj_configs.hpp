#pragma once
#include <type_traits>

namespace ProjCfg
{
using CommandSubtype = uint8_t;
using Byte           = uint8_t;

enum Memory {
    EEPROMAddressI2CBoardAddress = 0,
    ElectricalCharacteristicsAddress = 1,
    ElectricalCharacteristicsSize = 6

};
enum I2C {
    DefaultAddress = 32
};

enum class Command : CommandSubtype {
    EnableVoltageAtPin   = 0xc1,
    CheckInternalCounter = 0xc2,
    CheckVoltages        = 0xc3,
    SetVoltageLevel      = 0xc4,
    ChangeAddress        = 0xc5,
    TestDataLink         = 0xc6,
    GetFirmwareVersion   = 0xc7,
};

enum StandardResponse : CommandSubtype {
    OK                    = 0xa5,
    REPEAT_CMD_TO_CONFIRM = 0xAF,
    UnknownCommand        = 0x5a,
    FAIL                  = 0x50
};

enum Delay : Byte {
    TimeoutForSingleCommandReception = 100
};

enum BoardSpecifics : Byte {
    NumberOfPins = 32,
};
constexpr Byte FIRMWARE_VERSION = 18;
}