#pragma once

#include "pio_controller.hpp"
#include <array>
#include <type_traits>
#include <algorithm>

auto constexpr shifter_size = 24;

struct ShifterFunctionalPins {
    using PinNumT = uint8_t;

    PinNumT data;
    PinNumT strobe;
    PinNumT reset;
    PinNumT outEnable;
    PinNumT latch;
};

template<size_t NumberOfPins>
class Shifter {
  public:
    using PinNumT     = uint8_t;
    using PulseCountT = uint8_t;
    using PinStateT   = PioController::State;

    struct AllPinsCommand {
        std::array<bool, NumberOfPins>      affectedPins;
        std::array<PinStateT, NumberOfPins> newStateOfPins;
    };
    void static Init() noexcept
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

        DisableOutput();
        ReleaseReset();

        pinsState.fill(PinStateT::Low);
        EnableOutput();
    }
    Shifter() noexcept
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

        DisableOutput();
        ReleaseReset();

        pinsState.fill(PinStateT::Low);
        EnableOutput();
    }

    void static EnableOutput() noexcept { PioController::SetState(pinsConfigs.outEnable, PioController::State::Low); }
    void static DisableOutput() noexcept { PioController::SetState(pinsConfigs.outEnable, PioController::State::High); }
    template<size_t ArraySize>
    [[gnu::hot]] void static SetData(std::array<PinStateT, ArraySize> const &pins_data) noexcept
    {
        ResetState();

        std::for_each(pins_data.crbegin() + 1, pins_data.crend(), [](auto const it) {
            SetDataPinValue(it);
            MakeSingleStrobePulse();
        });

        SaveStateToLatch();
    }

    void static TestSetData() noexcept
    {
        ResetState();
        SaveStateToLatch();
    }

    std::array<PinStateT, shifter_size> static TestBigArray(std::array<PinStateT, shifter_size> const &arr) noexcept
    {
        SetDataPinValue(arr.at(5));
        //        dummyValue = arr.at(5);
        return arr;
    }
    [[gnu::hot]]void static SetPinState(PinNumT pin, PinStateT new_state) noexcept
    {
        if (pinsState[pin] == new_state)
            return;

        pinsState[pin] = new_state;
        SetData(pinsState);
    }
    void static SetPinsState(AllPinsCommand new_state) noexcept
    {
        for (uint8_t pin_counter = 0; auto &pin_state : pinsState) {
            if (new_state.affectedPins[pin_counter]) {
                pin_state = new_state.newStateOfPins[pin_counter];
            }

            pin_counter++;
        }

        SetData(pinsState);
    }

    [[noreturn]] void static UnitTest() noexcept
    {
        while (true) {
            SetPinState(0, Shifter::PinStateT::High);
            SetPinState(1, Shifter::PinStateT::High);
            SetPinState(5, Shifter::PinStateT::High);
            SetPinState(6, Shifter::PinStateT::High);
            SetPinState(7, Shifter::PinStateT::High);

            SetPinState(0, Shifter::PinStateT::Low);
            SetPinState(1, Shifter::PinStateT::Low);
            SetPinState(5, Shifter::PinStateT::Low);
            SetPinState(6, Shifter::PinStateT::Low);
            SetPinState(7, Shifter::PinStateT::Low);
        }
    }

  protected:
    void static SetDataPinValue(PinStateT state) noexcept { PioController::SetState(pinsConfigs.data, state); }
    void static SaveStateToLatch() noexcept
    {
        PioController::SetState(pinsConfigs.latch, PioController::State::High);
        PioController::SetState(pinsConfigs.latch, PioController::State::Low);
    }
    void static MakeSingleStrobePulse() noexcept
    {
        PioController::SetState(pinsConfigs.strobe, PioController::State::High);
        PioController::SetState(pinsConfigs.strobe, PioController::State::Low);
    }
    void static MakeNStrobePulses(PulseCountT pulses_count) noexcept
    {
        while (pulses_count--) {
            MakeSingleStrobePulse();
        }
    }
    void static SetReset() noexcept { PioController::SetState(pinsConfigs.reset, PioController::State::Low); }
    void static ReleaseReset() noexcept { PioController::SetState(pinsConfigs.reset, PioController::State::High); }
    void static ResetState() noexcept
    {
        SetReset();
        ReleaseReset();
    }

  private:
    ShifterFunctionalPins static constexpr pinsConfigs{ 8, 3, 9, 10, 1 };
    std::array<PinStateT, NumberOfPins> static pinsState;
    PinStateT static dataPinState;
};

using ShifterC = Shifter<24>;

extern ShifterC shifter;