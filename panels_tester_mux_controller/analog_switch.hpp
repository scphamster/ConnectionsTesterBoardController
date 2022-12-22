#pragma once

#include "shifter.hpp"

struct AnalogSwitchPins {
    using PinNumT = uint8_t;

    AnalogSwitchPins() = default;
    AnalogSwitchPins(PinNumT enable, PinNumT sw0, PinNumT sw1, PinNumT sw2, PinNumT sw3)
      : data{ sw0, sw1, sw2, sw3, enable }
    { }

    PinNumT GetEnablePinNum() const noexcept { return data[4]; }

    //  private:
    uint8_t static constexpr numberOfPins = 5;
    std::array<PinNumT, numberOfPins> data;
};

class AnalogSwitch {
  public:
    using ShifterC  = Shifter<24>;
    using PinStateT = ShifterC::PinStateT;
    using ChannelT  = uint8_t;

    AnalogSwitch() = default;

    AnalogSwitch(AnalogSwitchPins pins_config) noexcept
      : pinsConfig{ pins_config }
      , io{ ShifterC::Get() }
    {
        Disable();
    }
    [[noreturn]] void UnitTest() noexcept
    {
        while (true) {
            for (auto i = 0; i < 16; i++) {
                SetChannel(i);
            }
        }
    }

    void SetChannel(ChannelT new_channel) noexcept
    {
        if (new_channel == pinsState)
            return;

        Disable();

        for (uint8_t pin_counter = 0; pin_counter < 4; pin_counter++) {
            //            if (static_cast<bool>(new_channel bitand _BV(pin_counter)) != static_cast<bool>(pinsState bitand
            //            _BV(pin_counter))) {
            io->SetPinState(pinsConfig.data[pin_counter],
                            static_cast<PinStateT>(static_cast<bool>(new_channel bitand _BV(pin_counter))));
            //            }
        }
        pinsState = new_channel;
    }

    void Enable() noexcept { io->SetPinState(pinsConfig.GetEnablePinNum(), PinStateT::Low); }
    void Disable() noexcept { io->SetPinState(pinsConfig.GetEnablePinNum(), PinStateT::High); }

  protected:
  private:
    auto constexpr static channelsNumber = 16;
    using PinsStateT                     = uint8_t;

    AnalogSwitchPins pinsConfig;

    PinsStateT pinsState;
    ShifterC  *io;
};