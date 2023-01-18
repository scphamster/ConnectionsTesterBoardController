#pragma once
#include "project_configs.h"
#include "proj_configs.hpp"
#include "utilities.hpp"

#include <avr/io.h>
#include <cstdlib>
#include <stdint.h>
#include <avr/interrupt.h>
#include <array>
#include <functional>
#include <optional>
#include <cstdint>

#include "shifter.hpp"
#include "analog_switch.hpp"
#include "adc.hpp"
#include "timer.hpp"
#include "iic.hpp"
#include "ohm_meter.hpp"
#include "EEPROMController.hpp"

#define MY_ADDRESS 0x2A
#ifndef UINT32_MAX
#define UINT32_MAX 0XFFFFFFFF
#endif
class Command {
  public:
    using Byte  = uint8_t;
    using ArgsT = Byte;

    Command(Byte cmd)
      : command{ cmd }
    { }

    Byte         GetCommand() const noexcept { return command; }
    Byte         ReverseBits(Byte data) { return ~data; }
    virtual void Execute(Byte args) noexcept = 0;

    template<typename ArgT>
    void AcknowledgeArguments(ArgT args) noexcept
    {
        i2c.Send(args);
    }

    template<typename ReturnType>
    ReturnType WaitForCommandArgs() noexcept
    {
        return i2c.ReceiveBlocking<ReturnType>();
    }

  protected:
  private:
    Byte command;
};

class EnableOutputForPin : public Command {
  public:
    using Multimeter = OhmMeter;
    using PinVoltage = Multimeter::OutputVoltage;

    EnableOutputForPin(Multimeter &new_meter)
      : Command{ enable_voltage_at_pin_cmd_id }
      , meter{ new_meter }
    { }

    void Execute(ArgsT args) noexcept override final
    {
        if (args == Arguments::SpecialPinConfigurations::DisableAll) {
            meter.DisableAllOutputs();
        }
        else {
            meter.EnableOutputForPin(args);
        }
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
    using Multimeter = OhmMeter;
    using PinVoltage = Multimeter::OutputVoltage;

    SetOutputVoltage(Multimeter &new_meter)
      : Command{ set_output_voltage_cmd_id }
      , meter{ new_meter }
    { }

    void Execute(ArgsT args) noexcept final { meter.SelectOutputVoltage(static_cast<Multimeter::OutputVoltage>(args)); }

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

    void Execute(ArgsT dummy_args) noexcept override { i2c.Send(Timer8::GetCounterValue()); }

  protected:
  private:
};

class CheckVoltages : public Command {
  public:
    using Multimeter      = OhmMeter;
    using PinT            = uint8_t;
    using AllPinsVoltageT = Multimeter::AllPinsVoltageValue;

    CheckVoltages(Multimeter &new_meter)
      : Command{ check_voltages_cmd_id }
      , adc{ ADCHandler::Get() }
      , meter{ new_meter }
    { }

    void Execute(ArgsT args) noexcept override final
    {
        switch (static_cast<SpecialMeasurements>(args)) {
        case SpecialMeasurements::MeasureVCC: CheckVCC(); break;
        case SpecialMeasurements::MeasureGND: CheckGND(); break;
        case SpecialMeasurements::MeasureAll: CheckAll(); break;

        default:
            if (args > total_mux_pin_count - 1)
                CheckOne(args);
        }
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
        i2c.Send(adc->MakeSingleConversion());
    }
    void CheckGND() noexcept
    {
        adc->SetReference(ADCHandler::Reference::VCC);
        adc->SetSingleChannel(ADCHandler::SingleChannel::GND);
        i2c.Send(adc->MakeSingleConversion());
    }
    void CheckAll() noexcept { i2c.Send(meter.GetAllPinsVoltage8B()); }
    void CheckOne(PinT pin) noexcept { i2c.Send(meter.GetPinVoltage(pin)); }

  private:
    ADCHandler     *adc;
    Multimeter     &meter;
    AllPinsVoltageT tableOfVoltages;
};

class ChangeAddress : public Command {
  public:
    ChangeAddress()
      : Command(static_cast<Byte>(ProjCfg::Command::ChangeAddress))
    { }

    void Execute(ArgsT argument) noexcept final
    {
        if (Timer8::GetCounterValue() > executionDeadline) {
            i2c.Send(ProjCfg::Communications::FAIL);
            OnFailedStage();
            return;
        }

        switch (confirmation_process_stage) {
        case 0:
            if (argument > 0 and argument < 128) {
                new_address_to_be_set = argument;
                OnSuccessfulStage();
            }
            else
                OnFailedStage();
            break;

        case 1:
            if (argument == new_address_to_be_set)
                OnSuccessfulStage();
            else
                OnFailedStage();
            break;

        case 2:
            if (argument == passwordOne)
                OnSuccessfulStage();
            else
                OnFailedStage();
            break;

        case 3:
            if (argument == passwordTwo)
                OnSuccessfulStage();
            else
                OnFailedStage();
            break;

        case 4:
            if (argument == passwordOne)
                OnSuccessfulStage();
            else
                OnFailedStage();
            break;
        case 5:
            if (argument == new_address_to_be_set) {
                if (EEPROMController::DisableInterruptWriteAndCheck(ProjCfg::Memory::EEPROMAddressI2CBoardAddress,
                                                                    argument)) {
                    i2c.SetNewAddress(new_address_to_be_set);
                    i2c.Send(ProjCfg::Communications::OK);
                }
                else
                    i2c.Send(ProjCfg::Communications::FAIL);
            }
            break;

        default: break;
        }
    }

  protected:
    void Reset(){
        executionDeadline          = UINT32_MAX;
        confirmation_process_stage = 0;
        new_address_to_be_set      = 0;
    }
    void OnSuccessfulStage()
    {
        executionDeadline = Timer8::GetCounterValue() + MAX_TIME_TO_WAIT_FOR_NEXT_CONFIRMATION;
        confirmation_process_stage++;
        i2c.Send(ProjCfg::Communications::REPEAT_CMD_TO_CONFIRM);
    }
    void OnFailedStage()
    {
        Reset();
        i2c.Send(ProjCfg::Communications::FAIL);
    }
  private:
    Timer8::TimerValue constexpr static MAX_TIME_TO_WAIT_FOR_NEXT_CONFIRMATION = 500;
    constexpr static Byte passwordOne                                          = 153;
    constexpr static Byte passwordTwo                                          = 102;

    Byte confirmation_process_stage = 0;
    Byte new_address_to_be_set      = 0;

    Byte               confirmationsLeftToExecuteAddressChange = ProjCfg::NUMBER_OF_CONFIRMATIONS_TO_CHANGE_ADDRESSS;
    Timer8::TimerValue executionDeadline                       = UINT32_MAX;
};

class CommandHandler {
  public:
    using Byte       = uint8_t;
    using CommandT   = Byte;
    using ArgsT      = Byte;
    using Multimeter = OhmMeter;

    // todo: make prettier
    CommandHandler()
      : meter{}
      , setVoltageCmd{ meter }
      , checkVoltagesCmd{ meter }
      , setOutputVoltageCmd{ meter }
    {
        Init();
    }
    [[noreturn]] void MainLoop() noexcept
    {
        while (true) {
            auto new_command = i2c.Receive<CommandAndArgs>(100);
            if (new_command) {
                HandleCommand(*new_command);
            }
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
        i2c.Send(CommandAndArgs{ ReverseBits(cmd_and_args.cmd), cmd_and_args.args });
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

        case ToUnderlying(ProjCfg::Command::ChangeAddress):
            AcknowledgeCommand(cmd_and_args);
            changeAddressCmd.Execute(cmd_and_args.args);
            break;

        default: i2c.Send(CommandAndArgs{ unknownCommandResponse, cmd_and_args.args });
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
    auto constexpr static commandsNumber         = 5;
    Byte constexpr static unknownCommandResponse = 0x5a;
    std::array<Byte, commandsNumber> static constexpr commands{ enable_voltage_at_pin_cmd_id,
                                                                check_internal_counter_cmd_id,
                                                                check_voltages_cmd_id,
                                                                ToUnderlying(ProjCfg::Command::ChangeAddress),
                                                                ToUnderlying(ProjCfg::Command::TestDataLink) };

    OhmMeter meter;

    CurrentState currentState{ CurrentState::WaitingForNewCommand };

    EnableOutputForPin      setVoltageCmd;
    GetInternalCounterValue getCounterCmd;
    CheckVoltages           checkVoltagesCmd;
    SetOutputVoltage        setOutputVoltageCmd;
    ChangeAddress           changeAddressCmd;

    uint8_t currentPin = 0;
};