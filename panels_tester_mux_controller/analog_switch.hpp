#pragma once

#include "shifter.hpp"

struct AnalogSwitchPins {
    using PinNumT = uint8_t;

    AnalogSwitchPins() = default;
    AnalogSwitchPins(PinNumT enable, PinNumT sw0, PinNumT sw1, PinNumT sw2, PinNumT sw3)
      : data{ sw0, sw1, sw2, sw3, enable }
    { }

    //    PinNumT enable;
    //    PinNumT sw0;
    //    PinNumT sw1;
    //    PinNumT sw2;
    //    PinNumT sw3;

    //  private:
    uint8_t static constexpr numberOfPins = 5;
    std::array<PinNumT, numberOfPins> data;
};

template<typename PinControllerT>
class AnalogSwitch {
  public:
    using PinStateT = typename PinControllerT::PinStateT;
    using ChannelT  = uint8_t;

    AnalogSwitch() = default;

    AnalogSwitch(AnalogSwitchPins pins_config, PinControllerT *io_ctl) noexcept
      : pinsConfig{ pins_config }
      , io{ io_ctl }
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
        if (new_channel >= channelsNumber)
            return;

        Disable();

        if (new_channel == pinsState)
            return;

        for (uint8_t pin_counter = 0; pin_counter < 4; pin_counter++) {
            if ((new_channel bitand _BV(pin_counter)) != (pinsState bitand _BV(pin_counter))) {
                io->SetPinState(pinsConfig.data[pin_counter],
                                static_cast<PinStateT>(static_cast<bool>(new_channel bitand _BV(pin_counter))));
            }
        }
        pinsState = new_channel;
    }

    void Enable() noexcept { io->SetPinState(pinsConfig.data[4], PinStateT::Low); }
    void Disable() noexcept { io->SetPinState(pinsConfig.data[4], PinStateT::High); }

  protected:
  private:
    auto constexpr static channelsNumber = 16;
    using PinsStateT                     = uint8_t;

    AnalogSwitchPins pinsConfig;

    PinsStateT      pinsState;
    PinControllerT *io;
};