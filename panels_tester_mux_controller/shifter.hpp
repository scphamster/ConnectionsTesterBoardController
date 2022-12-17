#pragma once

#include "pio_controller.hpp"
#include <array>
#include <type_traits>
#include <algorithm>

struct ShifterFunctionalPins {
    using PinNumT = int;

    PinNumT data;
    PinNumT strobe;
    PinNumT reset;
    PinNumT outEnable;
    PinNumT latch;
};

template<size_t NumberOfPins>
class Shifter {
  public:
    using PinNumT = int;
    enum class PinStateT : bool {
        Low  = false,
        High = true
    };

    Shifter(ShifterFunctionalPins shifter_pins_configs) noexcept
      : pinsConfigs{ shifter_pins_configs }
    {
        auto pio = PioController{};

        using Mode  = PioController::Mode;
        using State = PioController::State;

        auto constexpr outputMode = Mode::Output;

        pio.SetMode(pinsConfigs.data, outputMode);
        pio.SetMode(pinsConfigs.strobe, outputMode);
        pio.SetMode(pinsConfigs.reset, outputMode);
        pio.SetMode(pinsConfigs.outEnable, outputMode);
        pio.SetMode(pinsConfigs.latch, outputMode);

        pinsState.fill(PinStateT::Low);

        DisableOutput();
        ReleaseReset();

        EnableOutput();
    }
    void EnableOutput() noexcept { PioController::SetState(pinsConfigs.outEnable, PioController::State::Low); }
    void DisableOutput() noexcept { PioController::SetState(pinsConfigs.outEnable, PioController::State::High); }

    template<size_t ArraySize>
    void SetData(std::array<PinStateT, ArraySize> pins_data) noexcept
    {
        DoReset();

        std::for_each(pins_data.rbegin(), pins_data.rend(), [this](auto const &it) {
            SetDataPinValue(it);
            MakeSingleStrobePulse();
        });

        SaveStateToLatch();
    }

    void SetPinState(PinNumT pin, PinStateT new_state) noexcept
    {
        //        if (pin >= NumberOfPins)
        //            return;

        //        if (pinsState[pin] == new_state)
        //            return;
        //
        pinsState[pin] = new_state;
        SetData(pinsState);
    }

  protected:
    void MakeSingleStrobePulse() const noexcept
    {
        PioController::SetState(pinsConfigs.strobe, PioController::State::High);
        PioController::SetState(pinsConfigs.strobe, PioController::State::Low);
    }
    void MakeNStrobePulses(int pulses_count) const noexcept
    {
        while (pulses_count > 0) {
            MakeSingleStrobePulse();
            pulses_count--;
        }
    }
    void SaveStateToLatch() const noexcept
    {
        PioController::SetState(pinsConfigs.latch, PioController::State::High);
        PioController::SetState(pinsConfigs.latch, PioController::State::Low);
    }
    void SetDataPinValue(PinStateT state) const noexcept
    {
        if (static_cast<bool>(state)) {
            PioController::SetState(pinsConfigs.data, PioController::State::High);
        }
        else {
            PioController::SetState(pinsConfigs.data, PioController::State::Low);
        }
    }
    void SetReset() const noexcept { PioController::SetState(pinsConfigs.reset, PioController::State::Low); }
    void ReleaseReset() const noexcept { PioController::SetState(pinsConfigs.reset, PioController::State::High); }
    void DoReset() const noexcept
    {
        SetReset();
        ReleaseReset();
    }

  private:
    ShifterFunctionalPins               pinsConfigs;
    std::array<PinStateT, NumberOfPins> pinsState{};
};