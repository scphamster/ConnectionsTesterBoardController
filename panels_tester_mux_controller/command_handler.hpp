#pragma once
#include "project_configs.h"

#include <avr/io.h>
#include <cstdlib>
#include <avr/interrupt.h>
#include <array>
#include <functional>
#include <optional>

#include "shifter.hpp"
#include "analog_switch.hpp"
#include "adc.hpp"
#include "timer.hpp"
#include "iic.hpp"
#include "ohm_meter.hpp"

#define MY_ADDRESS 0x25

class Command {
  public:
    using Byte  = uint8_t;
    using ArgsT = Byte;

    Command(Byte cmd)
      : command{ cmd }
      , i2c{ IIC::Get() }
    { }

    Byte         GetCommand() const noexcept { return command; }
    Byte         ReverseBits(Byte data) { return ~data; }
    virtual bool Execute(Byte args) noexcept = 0;

    template<typename ArgT>
    void AcknowledgeArguments(ArgT args) noexcept
    {
        i2c->Send(args);
    }

    template<typename ReturnType>
    ReturnType WaitForCommandArgs() noexcept
    {
        return i2c->ReceiveBlocking<ReturnType>();
    }

  protected:
    IIC *i2c = nullptr;

  private:
    Byte command;
};

class EnableOutputForPin : public Command {
  public:
    using Multimeter = OhmMeter<Shifter<shifter_size>, mux_pairs_on_board>;
    using PinVoltage = Multimeter::OutputVoltage;

    EnableOutputForPin(Multimeter &new_meter)
      : Command{ enable_voltage_at_pin_cmd_id }
      , meter{ new_meter }
    { }

    bool Execute(ArgsT args) noexcept override final
    {
        if (args == Arguments::SpecialPinConfigurations::DisableAll) {
            meter.DisableAllOutputs();
        }
        else {
            meter.EnableOutputForPin(args);
        }

        return true;
    }

  protected:
    struct Arguments {
        enum SpecialPinConfigurations : Byte {
            DisableAll = 254
        };
        Byte pinNumber;
    };

  private:
    Multimeter &meter;
};

class SetOutputVoltage : public Command {
  public:
    using Multimeter = OhmMeter<Shifter<shifter_size>, mux_pairs_on_board>;
    using PinVoltage = Multimeter::OutputVoltage;

    SetOutputVoltage(Multimeter &new_meter)
      : Command{ set_output_voltage_cmd_id }
      , meter{ new_meter }
    { }

    bool Execute(ArgsT args) noexcept override final
    {
        meter.SelectOutputVoltage(static_cast<Multimeter::OutputVoltage>(args));

        return true;
    }

  protected:
    struct Arguments {
        Multimeter::OutputVoltage voltageValue;
    };

  private:
    Multimeter &meter;
};

class GetInternalCounterValue : public Command {
  public:
    GetInternalCounterValue()
      : Command{ check_internal_counter_cmd_id }
    { }

    bool Execute(ArgsT dummy_args) noexcept override
    {
        i2c->Send(Timer8::GetCounterValue());

        return true;
    }

  protected:
  private:
};

class CheckVoltages : public Command {
  public:
    using Multimeter      = OhmMeter<Shifter<shifter_size>, mux_pairs_on_board>;
    using PinT            = uint8_t;
    using AllPinsVoltageT = Multimeter::AllPinsVoltageValue;

    CheckVoltages(Multimeter &new_meter)
      : Command{ check_voltages_cmd_id }
      , adc{ ADCHandler::Get() }
      , meter{ new_meter }
    { }

    bool Execute(ArgsT args) noexcept override final
    {
        switch (static_cast<SpecialMeasurements>(args)) {
        case SpecialMeasurements::MeasureVCC: CheckVCC(); break;
        case SpecialMeasurements::MeasureGND: CheckGND(); break;
        case SpecialMeasurements::MeasureAll: CheckAll(); break;

        default:
            if (args > total_mux_pin_count - 1)
                return false;

            CheckOne(args);
        }

        return true;
    }
    AllPinsVoltageT &GetVoltagesTable() noexcept { return tableOfVoltages; }

  protected:
    enum SpecialMeasurements : Byte {
        MeasureAll = 33,
        MeasureVCC,
        MeasureGND,
        //        MeasureAll
    };
    struct Arguments {
        using PinNum = Byte;
        PinNum pin;
    };

    void CheckVCC() noexcept
    {
        adc->SetReference(ADCHandler::Reference::VCC);
        adc->SetSingleChannel(ADCHandler::SingleChannel::_1v1Ref);
        i2c->Send(adc->MakeSingleConversion());
    }
    void CheckGND() noexcept
    {
        adc->SetReference(ADCHandler::Reference::VCC);
        adc->SetSingleChannel(ADCHandler::SingleChannel::GND);
        i2c->Send(adc->MakeSingleConversion());
    }
    void CheckAll() noexcept { i2c->Send(meter.GetAllPinsVoltage()); }
    void CheckOne(PinT pin) noexcept { i2c->Send(meter.GetPinVoltage(pin)); }

  private:
    ADCHandler     *adc;
    Multimeter     &meter;
    AllPinsVoltageT tableOfVoltages;
};

class CommandHandler {
  public:
    using Byte       = uint8_t;
    using CommandT   = Byte;
    using ArgsT      = Byte;
    using Multimeter = OhmMeter<Shifter<shifter_size>, mux_pairs_on_board>;

    // todo: make prettier
    CommandHandler()
      : i2c{ IIC::Get() }
      , meter{ Shifter<shifter_size>::Get(),
               std::array<AnalogSwitchPins, 2>{ AnalogSwitchPins{ 10, 14, 13, 12, 11 },
                                                AnalogSwitchPins{ 15, 19, 18, 17, 16 } },
               std::array<AnalogSwitchPins, 2>{ AnalogSwitchPins{ 0, 4, 3, 2, 1 }, AnalogSwitchPins{ 5, 9, 8, 7, 6 } } }
      , setVoltageCmd{ meter }
      , checkVoltagesCmd{ meter }
      , setOutputVoltageCmd{ meter }
    {
        Init();
    }
    [[noreturn]] void MainLoop() noexcept
    {
        while (true) {
            //            auto new_command = i2c->ReceiveBlocking<CommandAndArgs>();
            auto new_command = i2c->Receive<CommandAndArgs>(100);
            if (new_command) {
                HandleCommand(*new_command);
            }
        }
    }
    [[noreturn]] void UnitTestCommunications() noexcept
    {
        while (true) {
            auto byte = USI_TWI_Receive_Byte();
            USI_TWI_Transmit_Byte(byte);

            //            auto value = i2c->Receive<Byte>();
            //            if (value) {
            //                i2c->Send(*value);
            //            }

            //            auto value = i2c->Receive<std::array<Byte, 64>>(400);
            //            if (value) {
            //                i2c->Send(*value);
            //            }
        }
    }

  protected:
    struct CommandAndArgs {
        CommandT cmd;
        ArgsT    args;
    };

    enum class CurrentState {
        WaitingForNewCommand,
        CommandReceived_WaitingForArgument,
        CommandReceived_Executing,
        ExecutionCompleted_WaitingForCommand
    };
    void Init() noexcept { }
    void AcknowledgeCommand(CommandAndArgs cmd_and_args) noexcept
    {
        i2c->Send(CommandAndArgs{ ReverseBits(cmd_and_args.cmd), cmd_and_args.args });
    }
    Byte static ReverseBits(Byte data) noexcept { return ~data; }
    void HandleCommand(CommandAndArgs cmd_and_args) noexcept
    {
        CommandT answer{ unknownCommandResponse };

        switch (cmd_and_args.cmd) {
        case enable_voltage_at_pin_cmd_id:
            AcknowledgeCommand(cmd_and_args);
            setVoltageCmd.Execute(cmd_and_args.args);
            break;

        case check_internal_counter_cmd_id:
            AcknowledgeCommand(cmd_and_args);
            getCounterCmd.Execute(cmd_and_args.args);
            break;

        case check_voltages_cmd_id:
            AcknowledgeCommand(cmd_and_args);
            checkVoltagesCmd.Execute(cmd_and_args.args);
            break;

        case set_output_voltage_cmd_id:
            AcknowledgeCommand(cmd_and_args);
            setOutputVoltageCmd.Execute(cmd_and_args.args);
            break;

        default: i2c->Send(CommandAndArgs{ unknownCommandResponse, cmd_and_args.args });
        }
    }
    void BackgroundMeasurements() noexcept
    {
        //        checkVoltagesCmd.GetVoltagesTable()[currentPin] = meter.GetPinVoltage(currentPin);
        currentPin++;

        if (currentPin >= total_mux_pin_count)
            currentPin = 0;
    }

  private:
    auto constexpr static commandsNumber         = 3;
    Byte constexpr static unknownCommandResponse = 0x5a;
    std::array<Byte, commandsNumber> static constexpr commands{ enable_voltage_at_pin_cmd_id,
                                                                check_internal_counter_cmd_id,
                                                                check_voltages_cmd_id };

    IIC       *i2c = nullptr;
    Multimeter meter;

    CurrentState currentState{ CurrentState::WaitingForNewCommand };

    EnableOutputForPin      setVoltageCmd;
    GetInternalCounterValue getCounterCmd;
    CheckVoltages           checkVoltagesCmd;
    SetOutputVoltage        setOutputVoltageCmd;

    uint8_t currentPin = 0;
};