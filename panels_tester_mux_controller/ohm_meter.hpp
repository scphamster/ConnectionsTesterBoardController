#pragma once
#include "project_configs.h"

#include <array>

#include "shifter.hpp"
#include "analog_switch.hpp"
#include "adc.hpp"
#include "proj_configs.hpp"

class OhmMeter {
  public:
    using Switch              = AnalogSwitch;
    using Byte                = uint8_t;
    using PinNumT             = Byte;
    using PinState            = ShifterC::PinStateT;
    using AdcValueT           = ADCHandler::ResultT;
    using AdcValue8B          = Byte;
    using AllPinsVoltageValue = std::array<AdcValueT, ProjCfg::BoardSpecifics::NumberOfPins>;
    using AllPinsVoltages8B   = std::array<AdcValue8B, ProjCfg::BoardSpecifics::NumberOfPins>;
    enum class OutputVoltage : uint8_t {
        Undefined = 0,
        _09       = high_voltage_reference_select_pin,
        _07       = low_voltage_reference_select_pin,
    };

    OhmMeter() noexcept
    {
        SelectOutputVoltage(OutputVoltage::_07);
    }

    [[noreturn]] void UnitTest() noexcept
    {
        while (true) {
            for (auto i = 0; i < channelsPairsNum; i++) {
                EnableOutputForPin(i);
                EnableInputChannel(i);
            }
        }
    }

    AdcValueT GetPinVoltage(PinNumT pin) noexcept
    {
        ConfigureADCForMeasurement();
        EnableInputChannel(pin);
        return ADCHandler::MakeSingleConversion();
    }

    AllPinsVoltageValue GetAllPinsVoltage() noexcept
    {
        ConfigureADCForMeasurement();

        AllPinsVoltageValue array_of_voltages;

        for (uint8_t pin_num = 0; auto &pin_v_value : array_of_voltages) {
            EnableInputChannel(pin_num);
            pin_v_value = ADCHandler::MakeSingleConversion();

            pin_num++;
        }

        return array_of_voltages;
    }

    AllPinsVoltages8B GetAllPinsVoltage8B() noexcept
    {
        ConfigureADCForMeasurement();
        AllPinsVoltages8B array_of_voltages;

        for (Byte pin_num = 0; auto &pin_voltage : array_of_voltages) {
            EnableInputChannel(pin_num);

            auto voltage = ADCHandler::MakeSingleConversion();
            pin_voltage  = (voltage < 256) ? static_cast<AdcValue8B>(voltage) : static_cast<AdcValue8B>(255);
            pin_num++;
        }

        DisableAllInputs();

        return array_of_voltages;
    }

    void EnableOutputForPin(PinNumT ch)
    {
        Byte mux_num;
        if (ch > 15){
            ch -= 16;
            mux_num = 1;
        }
        else
            mux_num = 0;

        DisableAllOutputs();

        outputMuxes[mux_num].SetChannel(ch);
        outputMuxes[mux_num].Enable();
    }
    void EnableInputChannel(PinNumT ch)
    {
        Byte mux_num;
        if (ch > 15){
            ch -= 16;
            mux_num = 1;
        }
        else
            mux_num = 0;
        DisableAllInputs();

        inputMuxes[mux_num].SetChannel(ch);
        inputMuxes[mux_num].Enable();
    }
    void SelectOutputVoltage(OutputVoltage value) noexcept
    {
        if (currentVoltage == value)
            return;

        if (value == OutputVoltage::_07) {
            shifter.SetPinState(static_cast<PinNumT>(value), PinState::High);
            shifter.SetPinState(static_cast<PinNumT>(OutputVoltage::_09), PinState::Low);
            currentVoltage = value;
        }
        else if (value == OutputVoltage::_09) {
            shifter.SetPinState(static_cast<PinNumT>(value), PinState::High);
            shifter.SetPinState(static_cast<PinNumT>(OutputVoltage::_07), PinState::Low);
            currentVoltage = value;
        }
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

  protected:
    void ConfigureADCForMeasurement() noexcept
    {
        ADCHandler::SetReference(adcReference);
        ADCHandler::SetSingleChannel(adcSensingChannel);
    }
    void SetVoltageAtPin(PinNumT pin) noexcept { }

  private:
    auto constexpr static Mux16PairNum      = 2;
    auto constexpr static channelsPairsNum  = Mux16PairNum * 16;
    auto constexpr static adcSensingChannel = ADCHandler::SingleChannel::_0;
    auto constexpr static adcReference      = ADCHandler::Reference::Internal1v1;

    std::array<Switch, Mux16PairNum> static outputMuxes;
    std::array<Switch, Mux16PairNum> static inputMuxes;

    OutputVoltage currentVoltage = OutputVoltage::Undefined;
};