#pragma once
#include <avr/io.h>

class PioController {
  public:
    using PinNumT = int;
    using RegPtrT = volatile uint8_t *;
    enum class Mode {
        Input,
        Output
    };
    enum class State {
        High,
        Low
    };

    PioController() = default;

    void static SetState(PinNumT pin, State new_state) noexcept
    {
        auto pin_number_in_current_port = pin % 8;

        if (new_state == State::Low) {
            *PinToPort(pin) &= ~(1 << pin_number_in_current_port);
        }
        else if (new_state == State::High) {
            *PinToPort(pin) |= (1 << pin_number_in_current_port);
        }
    }

    void static SetMode(PinNumT pin, Mode new_mode) noexcept
    {
        auto pin_number_in_current_port = pin % 8;

        if (new_mode == Mode::Input) {
            (*PinToDDR(pin)) &= ~(1 << pin_number_in_current_port);
        }
        else if (new_mode == Mode::Output) {
            (*PinToDDR(pin)) |= (1 << pin_number_in_current_port);
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