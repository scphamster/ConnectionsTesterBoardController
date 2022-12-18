#pragma once
#include "project_configs.h"

#include <array>

#include "shifter.hpp"
#include "analog_switch.hpp"
#include "adc.hpp"

template<typename PioControllerT, size_t Mux16PairsNum>
class OhmMeter {
  public:
    using Switch              = AnalogSwitch<PioControllerT>;
    using PinNumT             = uint8_t;
    using AdcValueT           = ADCHandler::ResultT;
    using AllPinsVoltageValue = std::array<AdcValueT, total_mux_pin_count>;

    OhmMeter(PioControllerT                             *new_pio,
             std::array<AnalogSwitchPins, Mux16PairsNum> output_mux_pins,
             std::array<AnalogSwitchPins, Mux16PairsNum> input_mux_pins) noexcept
      : pio{ new_pio }
      , adc{ ADCHandler::Get() }
    {
        for (auto switch_num = 0; auto &out_mux : outputMuxes) {
            out_mux = Switch{ output_mux_pins[switch_num], pio };
            switch_num++;
        }

        for (auto switch_num = 0; auto &in_mux : inputMuxes) {
            in_mux = Switch{ input_mux_pins[switch_num], pio };
            switch_num++;
        }
    }

    [[noreturn]] void UnitTest() noexcept
    {
        while (true) {
            for (auto i = 0; i < channelsPairsNum; i++) {
                EnableOutputChannel(i);
                EnableInputChannel(i);
            }
        }
    }

    AdcValueT GetPinVoltage(PinNumT pin) noexcept
    {
        EnableInputChannel(pin);
        ConfigureADCForMeasurement();
        return adc->MakeSingleConversion();
    }

    AllPinsVoltageValue GetAllPinsVoltage() noexcept
    {
        ConfigureADCForMeasurement();

        AllPinsVoltageValue array_of_voltages;

        for (uint8_t pin_num = 0; auto &pin_v_value : array_of_voltages) {
            EnableInputChannel(pin_num);
            pin_v_value = adc->MakeSingleConversion();

            pin_num++;
        }

        return array_of_voltages;
    }

    void EnableOutputChannel(PinNumT ch)
    {
        auto mux_num = ch / 16;
        ch %= 16;

        DisableAllOutputs();

        outputMuxes[mux_num].SetChannel(ch);
        outputMuxes[mux_num].Enable();
    }
    void EnableInputChannel(PinNumT ch)
    {
        auto mux_num = ch / 16;
        ch %= 16;

        DisableAllInputs();

        inputMuxes[mux_num].SetChannel(ch);
        inputMuxes[mux_num].Enable();
    }

  protected:
    void ConfigureADCForMeasurement() noexcept
    {
        adc->SetReference(adcReference);
        adc->SetSingleChannel(adcSensingChannel);
    }
    void DisableAllOutputs() noexcept
    {
        for (auto &mux : outputMuxes) {
            mux.Disable();
        }
    }
    void DisableAllInputs() noexcept
    {
        for (auto &mux : inputMuxes) {
            mux.Disable();
        }
    }

    void SetVoltageAtPin(PinNumT pin) noexcept { }

  private:
    auto constexpr static channelsPairsNum  = Mux16PairsNum * 16;
    auto constexpr static adcSensingChannel = ADCHandler::SingleChannel::_0;
    auto constexpr static adcReference      = ADCHandler::Reference::Internal1v1;
    PioControllerT                   *pio   = nullptr;
    ADCHandler                       *adc   = nullptr;
    std::array<Switch, Mux16PairsNum> outputMuxes;
    std::array<Switch, Mux16PairsNum> inputMuxes;
};