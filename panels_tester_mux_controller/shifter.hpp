#pragma once

#include "pio_controller.hpp"
#include <array>
#include <type_traits>
#include <algorithm>
#include "my_heap.hpp"

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

    void static  Create(ShifterFunctionalPins const shifter_pins_configs) noexcept
    {
        _this  = heap.GetMemory<Shifter<shifter_size>>();
        *_this = Shifter<24>{ shifter_pins_configs };
    }
    Shifter<shifter_size> static *Get() noexcept
    {
        if (_this == nullptr)
            while (true) { }

        return _this;
    }

    void EnableOutput() noexcept { PioController::SetState(pinsConfigs.outEnable, PioController::State::Low); }
    void DisableOutput() noexcept { PioController::SetState(pinsConfigs.outEnable, PioController::State::High); }
    template<size_t ArraySize>
    void SetData(std::array<PinStateT, ArraySize> const &pins_data) noexcept
    {
        DoReset();

        std::for_each(pins_data.crbegin(), pins_data.crend(), [this](auto const &it) {
            SetDataPinValue(it);
            MakeSingleStrobePulse();
        });

        SaveStateToLatch();
    }

    void TestSetData() noexcept
    {
        DoReset();
        SaveStateToLatch();
    }

    std::array<PinStateT, shifter_size> TestBigArray(std::array<PinStateT, shifter_size>  const &arr) noexcept {
        SetDataPinValue(arr.at(5));
//        dummyValue = arr.at(5);
        return arr;

    }
    void SetPinState(PinNumT pin, PinStateT new_state) noexcept
    {
        if (pinsState[pin] == new_state)
            return;

        pinsState[pin] = new_state;
        SetData(pinsState);
    }
    void SetPinsState(AllPinsCommand new_state) noexcept
    {
        for (uint8_t pin_counter = 0; auto &pin_state : pinsState) {
            if (new_state.affectedPins[pin_counter]) {
                pin_state = new_state.newStateOfPins[pin_counter];
            }

            pin_counter++;
        }

        SetData(pinsState);
    }

    [[noreturn]] void UnitTest() noexcept
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

    // todo: make protected after test
    void SetDataPinValue(PinStateT state) noexcept
    {
        if (dataPinState == state)
            return;

        PioController::SetState(pinsConfigs.data, state);

        dataPinState = state;
    }
    // todo: make protected after test

    void SaveStateToLatch() const noexcept
    {
        PioController::SetState(pinsConfigs.latch, PioController::State::High);
        PioController::SetState(pinsConfigs.latch, PioController::State::Low);
    }

  protected:
    void MakeSingleStrobePulse() const noexcept
    {
        PioController::SetState(pinsConfigs.strobe, PioController::State::High);
        PioController::SetState(pinsConfigs.strobe, PioController::State::Low);
    }
    void MakeNStrobePulses(PulseCountT pulses_count) const noexcept
    {
        while (pulses_count > 0) {
            MakeSingleStrobePulse();
            pulses_count--;
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
    Shifter(ShifterFunctionalPins const shifter_pins_configs) noexcept
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

        DisableOutput();
        ReleaseReset();

        pinsState.fill(PinStateT::Low);
        EnableOutput();
    }

    Shifter static                     *_this;
    ShifterFunctionalPins               pinsConfigs;
    std::array<PinStateT, NumberOfPins> pinsState{};

    PinStateT dataPinState = PinStateT::Low;
    static PinStateT dummyValue;
};