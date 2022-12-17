#pragma once

#include "shifter.hpp"

struct AnalogSwitchPins {
    using PinNumT = int;

    PinNumT enable;
    PinNumT sw0;
    PinNumT sw1;
    PinNumT sw2;
    PinNumT sw3;
};

template<typename PinControllerT>
class AnalogSwitch {
  public:
    using PinStateT = typename PinControllerT::PinStateT;
    using ChannelT  = int;

    AnalogSwitch(AnalogSwitchPins pins_config, PinControllerT *io_ctl) noexcept
      : pinsConfig{ pins_config }
      , io{ io_ctl }
    {
        Disable();
    }

    void SetChannel(ChannelT new_channel) noexcept
    {
        if (new_channel >= channelsNumber)
            return;

        if (new_channel bitand _BV(0)) {
            io->SetPinState(pinsConfig.sw0, PinStateT::High);
        }
        else {
            io->SetPinState(pinsConfig.sw0, PinStateT::Low);
        }

        if (new_channel bitand _BV(1)) {
            io->SetPinState(pinsConfig.sw1, PinStateT::High);
        }
        else {
            io->SetPinState(pinsConfig.sw1, PinStateT::Low);
        }

        if (new_channel bitand _BV(2)) {
            io->SetPinState(pinsConfig.sw2, PinStateT::High);
        }
        else {
            io->SetPinState(pinsConfig.sw2, PinStateT::Low);
        }

        if (new_channel bitand _BV(3)) {
            io->SetPinState(pinsConfig.sw3, PinStateT::High);
        }
        else {
            io->SetPinState(pinsConfig.sw3, PinStateT::Low);
        }
    }

    void Enable() noexcept { io->SetPinState(pinsConfig.enable, PinStateT::Low); }
    void Disable() noexcept { io->SetPinState(pinsConfig.enable, PinStateT::High); }

  protected:
  private:
    auto constexpr static channelsNumber = 16;

    AnalogSwitchPins pinsConfig;
    PinControllerT  *io;
};