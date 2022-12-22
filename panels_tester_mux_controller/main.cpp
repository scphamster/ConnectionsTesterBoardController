

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
    Timer8::Init(Timer8::Clock::_1024, Timer8::Waveform::CTC, timer8_ctc_value);
//    Timer8::Run();
    sei();
}

int
main()
{
    initialization();
    //    shifter_test();

    auto cmd_handler = CommandHandler{};
    cmd_handler.MainLoop();
//        cmd_handler.UnitTestCommunications();

    while (true) { }
}
