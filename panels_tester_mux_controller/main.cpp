#include <avr/io.h>

#include "command_handler.hpp"
#include "shifter.hpp"
#include "ohm_meter.hpp"

template<>
std::array<ShifterC::PinStateT, 24> ShifterC::pinsState{};
template<>
ShifterC::PinStateT ShifterC::dataPinState = ShifterC::PinStateT::Low;

ShifterC shifter;
IIC      i2c{ IIC::Role::Slave, MY_ADDRESS };

std::array<AnalogSwitch, 2> OhmMeter::outputMuxes{ AnalogSwitch{ AnalogSwitchPins{ 10, 14, 13, 12, 11 } },
                                                   AnalogSwitch{ AnalogSwitchPins{ 15, 19, 18, 17, 16 } } };
std::array<AnalogSwitch, 2> OhmMeter::inputMuxes{ AnalogSwitch{ AnalogSwitchPins{ 0, 4, 3, 2, 1 } },
                                                  AnalogSwitch{ AnalogSwitchPins{ 5, 9, 8, 7, 6 } } };

void
initialization() noexcept
{
    ADCHandler::Create();
    //    IIC::Create(IIC::Role::Slave, MY_ADDRESS);
    Timer8::Init(Timer8::Clock::_1024, Timer8::Waveform::CTC, timer8_ctc_value);
    sei();
}

class TestPioCTl {
  public:
    using PinNumT = int;
    using RegPtrT = volatile uint8_t *;
    enum class Mode {
        Input,
        Output
    };
    enum class State : bool {
        High = true,
        Low  = false
    };

    template<State new_state>
    void static SetState(PinNumT pin) noexcept
    {
        auto pin_number_in_current_port = pin % 8;

        if constexpr (new_state == State::Low) {
            *PinToPort(pin) &= ~(1 << pin_number_in_current_port);
        }
        else if (new_state == State::High) {
            *PinToPort(pin) |= (1 << pin_number_in_current_port);
        }
    }

    template<PinNumT pin, Mode new_mode>
    void static SetMode() noexcept
    {
        auto pin_number_in_current_port = pin % 8;

        if constexpr (new_mode == Mode::Input) {
            (*PinToDDR(pin)) &= ~(1 << pin_number_in_current_port);
        }
        else if (new_mode == Mode::Output) {
            (*PinToDDR(pin)) |= (1 << pin_number_in_current_port);
        }
    }

    [[noreturn]] void static UnitTest() noexcept
    {
        SetMode<8, Mode::Output>;

        while (true) {
            //            SetState<State::Low>(8);
            //            SetState<State::High>(8);
        }
    }

  protected:
    RegPtrT static PinToPort(PinNumT pin) noexcept
    {
        if (pin < 8) {
            return &PORTA;
        }
        else if (pin > 7 and pin < 16) {
            return &PORTB;
        }
        else
            return nullptr;
    }
    RegPtrT static PinToDDR(PinNumT pin) noexcept
    {
        if (pin < 8) {
            return &DDRA;
        }
        else if (pin > 7 and pin < 16) {
            return &DDRB;
        }
        else
            return nullptr;
    }
};

int
main()
{
//    while (true) {
//        ShifterC::SetDataPinValue(ShifterC::PinStateT::Low);
//        ShifterC::SetDataPinValue(ShifterC::PinStateT::High);
//    }

    initialization();

    auto cmd_handler = CommandHandler{};
    cmd_handler.MainLoop();
    //    cmd_handler.UnitTestCommunications();

    while (true) { }
}
