

#include <avr/io.h>

#include "command_handler.hpp"
#include "shifter.hpp"

#include "ohm_meter.hpp"

void
initialization() noexcept
{
    ADCHandler::Create();
    IIC::Create(IIC::Role::Slave, MY_ADDRESS);
    Shifter<shifter_size>::Create(ShifterFunctionalPins{ 8, 3, 9, 10, 1 });

    sei();
}

void
shifter_test()
{
    using Shifter = Shifter<shifter_size>;

    auto shifter = Shifter::Get();
//    auto meter = OhmMeter<Shifter, 2>{
//        shifter,
//        std::array<AnalogSwitchPins, 2>{ AnalogSwitchPins{ 10, 14, 13, 12, 11 }, AnalogSwitchPins{ 15, 19, 18, 17, 16 } },
//        std::array<AnalogSwitchPins, 2>{ AnalogSwitchPins{ 0, 4, 3, 2, 1 }, AnalogSwitchPins{ 5, 9, 8, 7, 6 } }
//    };
//    meter.UnitTest();
}

int
main()
{
    initialization();
//    shifter_test();

    auto cmd_handler = CommandHandler{};
    cmd_handler.MainLoop();

    while (true) { }
}
