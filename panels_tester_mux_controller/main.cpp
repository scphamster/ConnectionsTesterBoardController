#include <avr/io.h>

#include "command_handler.hpp"
#include "shifter.hpp"
#include "ohm_meter.hpp"
#include "EEPROMController.hpp"
#include "proj_configs.hpp"

template<>
std::array<ShifterC::PinStateT, 24> ShifterC::pinsState{};
template<>
ShifterC::PinStateT ShifterC::dataPinState = ShifterC::PinStateT::Low;

ShifterC shifter;
IIC      i2c{ IIC::Role::Slave, ProjCfg::I2C::DefaultAddress };

std::array<AnalogSwitch, 2> OhmMeter::outputMuxes{ AnalogSwitch{ AnalogSwitchPins{ 10, 14, 13, 12, 11 } },
                                                   AnalogSwitch{ AnalogSwitchPins{ 15, 19, 18, 17, 16 } } };
std::array<AnalogSwitch, 2> OhmMeter::inputMuxes{ AnalogSwitch{ AnalogSwitchPins{ 0, 4, 3, 2, 1 } },
                                                  AnalogSwitch{ AnalogSwitchPins{ 5, 9, 8, 7, 6 } } };

void
initialization() noexcept
{
    auto i2c_address = EEPROMController::Read(ProjCfg::Memory::EEPROMAddressI2CBoardAddress);
    if (i2c_address != 0xff and i2c_address != 0)
        i2c.SetNewAddress(i2c_address);

    ADCHandler::Create();
    Timer8::Init(Timer8::Clock::_1024, Timer8::Waveform::CTC, timer8_ctc_value);
    sei();
}

int
main()
{
    initialization();

    auto cmd_handler = CommandHandler{};
    cmd_handler.MainLoop();
    //    cmd_handler.UnitTestCommunications();

    while (true) { }
}
