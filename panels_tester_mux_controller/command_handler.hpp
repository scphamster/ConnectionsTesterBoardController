#pragma once
#include "project_configs.h"
#include "proj_configs.hpp"
#include "utilities.hpp"

#include <avr/io.h>
#include <cstdlib>
#include <cstdint>
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

#ifndef UINT32_MAX
#define UINT32_MAX 0XFFFFFFFF
#endif
class Command {
  public:
    using Byte  = uint8_t;
    using ArgsT = Byte;

    virtual void Execute(Byte args) noexcept = 0;
};

class EnableOutputForPin : public Command {
  public:
    using Multimeter = OhmMeter;
    using PinVoltage = Multimeter::OutputVoltage;

    EnableOutputForPin(Multimeter &new_meter)
      : meter{ new_meter }
    { }

    void Execute(ArgsT args) noexcept final
    {
        if (args == Arguments::SpecialPinConfigurations::DisableAll) {
            meter.DisableAllOutputs();
        }
        else if (args < ProjCfg::BoardSpecifics::NumberOfPins) {
            meter.EnableOutputForPin(args);
        }
        else {
            meter.EnableInputChannel(args - ProjCfg::BoardSpecifics::NumberOfPins);
        }
    }

  protected:
    struct Arguments {
        enum SpecialPinConfigurations : Byte {
            DisableAll = 254
        };
    };

  private:
    Multimeter &meter;
};

class SetOutputVoltageLevel : public Command {
  public:
    using Multimeter = OhmMeter;
    using PinVoltage = Multimeter::OutputVoltage;

    SetOutputVoltageLevel(Multimeter &new_meter)
      : meter{ new_meter }
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
    void Execute(ArgsT dummy_args) noexcept final { i2c.Send(Timer8::GetCounterValue()); }

  protected:
  private:
};

class CheckVoltages : public Command {
  public:
    using Multimeter      = OhmMeter;
    using PinT            = uint8_t;
    using AllPinsVoltageT = Multimeter::AllPinsVoltageValue;

    CheckVoltages(Multimeter &new_meter)
      : meter{ new_meter }
    { }

    void Execute(ArgsT args) noexcept final
    {
        switch (static_cast<SpecialMeasurements>(args)) {
        case SpecialMeasurements::MeasureVCC: CheckVCC(); break;
        case SpecialMeasurements::MeasureGND: CheckGND(); break;
        case SpecialMeasurements::MeasureAll: CheckAll(); break;

        default:
            if (args > ProjCfg::BoardSpecifics::NumberOfPins - 1)
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
        ADCHandler::SetReference(ADCHandler::Reference::VCC);
        ADCHandler::SetSingleChannel(ADCHandler::SingleChannel::_1v1Ref);
        i2c.Send(ADCHandler::MakeSingleConversion());
    }
    void CheckGND() noexcept
    {
        ADCHandler::SetReference(ADCHandler::Reference::VCC);
        ADCHandler::SetSingleChannel(ADCHandler::SingleChannel::GND);
        i2c.Send(ADCHandler::MakeSingleConversion());
    }
    void CheckAll() noexcept { i2c.Send(meter.GetAllPinsVoltage8B()); }
    void CheckOne(PinT pin) noexcept { i2c.Send(meter.GetPinVoltage(pin)); }

  private:
    Multimeter     &meter;
    AllPinsVoltageT tableOfVoltages;
};

class ChangeAddress : public Command {
  public:
    void Execute(ArgsT argument) noexcept final
    {
        if (Timer8::GetCounterValue() > executionDeadline) {
            i2c.Send(ProjCfg::StandardResponse::FAIL);
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
                if (EEPROMController::SafeWriteWithCheck(ProjCfg::Memory::EEPROMAddressI2CBoardAddress, argument)) {
                    i2c.SetNewAddress(new_address_to_be_set);
                    i2c.Send(ProjCfg::StandardResponse::OK);
                }
                else
                    i2c.Send(ProjCfg::StandardResponse::FAIL);
            }
            break;

        default: break;
        }
    }

  protected:
    void Reset()
    {
        executionDeadline          = UINT32_MAX;
        confirmation_process_stage = 0;
        new_address_to_be_set      = 0;
    }
    void OnSuccessfulStage()
    {
        executionDeadline = Timer8::GetCounterValue() + MAX_TIME_TO_WAIT_FOR_NEXT_CONFIRMATION;
        confirmation_process_stage++;
        i2c.Send(ProjCfg::StandardResponse::REPEAT_CMD_TO_CONFIRM);
    }
    void OnFailedStage()
    {
        Reset();
        i2c.Send(ProjCfg::StandardResponse::FAIL);
    }

  private:
    Timer8::TimerValue constexpr static MAX_TIME_TO_WAIT_FOR_NEXT_CONFIRMATION = 500;
    constexpr static Byte passwordOne                                          = 153;
    constexpr static Byte passwordTwo                                          = 102;

    Byte confirmation_process_stage = 0;
    Byte new_address_to_be_set      = 0;

    Timer8::TimerValue executionDeadline = UINT32_MAX;
};

class SetInternalParameters : public Command {
  public:
    void Execute(ArgsT args) noexcept final
    {
        if (args_counter < ProjCfg::Memory::ElectricalCharacteristicsSize) {
            write_buffer[args_counter] = args;
            args_counter++;
            i2c.Send(ProjCfg::StandardResponse::REPEAT_CMD_TO_CONFIRM);
        }
        else if (args_counter == ProjCfg::ElectricalCharacteristicsSize) {
            auto result = EEPROMController::SuperWrite(ProjCfg::ElectricalCharacteristicsAddress, write_buffer);

            i2c.FlushBuffers();

            if (result)
                i2c.Send(ProjCfg::StandardResponse::OK);
            else
                i2c.Send(ProjCfg::StandardResponse::FAIL);

            args_counter = 0;
        }
        else {
            i2c.Send(ProjCfg::StandardResponse::FAIL);
        }
    }

  private:
    Byte                                                             args_counter = 0;
    std::array<Byte, ProjCfg::Memory::ElectricalCharacteristicsSize> write_buffer;
};

class GetInternalParameters : public Command {
  public:
    void Execute(ArgsT args) noexcept final
    {
        auto result = EEPROMController::SuperRead<std::array<Byte, ProjCfg::Memory::ElectricalCharacteristicsSize>>(
          ProjCfg::Memory::ElectricalCharacteristicsAddress);

        //        i2c.FlushBuffers();
        i2c.Send(result);
    }
};

class CommandHandler {
  public:
    using Byte       = uint8_t;
    using CommandT   = Byte;
    using ArgsT      = Byte;
    using Multimeter = OhmMeter;

    CommandHandler()
      : meter{}
      , setVoltageCmd{ meter }
      , checkVoltagesCmd{ meter }
      , setOutputVoltageCmd{ meter }
    { }
    [[noreturn]] void MainLoop() noexcept
    {
        while (true) {
            auto new_command = i2c.Receive<CommandAndArgs>(ProjCfg::Delay::TimeoutForSingleCommandReception);
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

    void AcknowledgeCommand(CommandAndArgs cmd_and_args) noexcept
    {
        i2c.Send(CommandAndArgs{ ReverseBits(cmd_and_args.cmd), cmd_and_args.args });
    }
    Byte static ReverseBits(Byte data) noexcept { return ~data; }
    void HandleCommand(CommandAndArgs cmd_and_args) noexcept
    {
        switch (cmd_and_args.cmd) {
        case ToUnderlying(ProjCfg::Command::EnableVoltageAtPin):
            AcknowledgeCommand(cmd_and_args);
            setVoltageCmd.Execute(cmd_and_args.args);
            break;

        case ToUnderlying(ProjCfg::Command::CheckInternalCounter):
            AcknowledgeCommand(cmd_and_args);
            getCounterCmd.Execute(cmd_and_args.args);
            break;

        case ToUnderlying(ProjCfg::Command::CheckVoltages):
            AcknowledgeCommand(cmd_and_args);
            checkVoltagesCmd.Execute(cmd_and_args.args);
            break;

        case ToUnderlying(ProjCfg::Command::SetVoltageLevel):
            AcknowledgeCommand(cmd_and_args);
            setOutputVoltageCmd.Execute(cmd_and_args.args);
            break;

        case ToUnderlying(ProjCfg::Command::ChangeAddress):
            AcknowledgeCommand(cmd_and_args);
            changeAddressCmd.Execute(cmd_and_args.args);
            break;

        case ToUnderlying(ProjCfg::Command::GetFirmwareVersion):
            AcknowledgeCommand(cmd_and_args);
            i2c.Send(ProjCfg::FIRMWARE_VERSION);
            break;

        case ToUnderlying(ProjCfg::Command::SetInternalParameters):
            AcknowledgeCommand(cmd_and_args);
            setParametersCmd.Execute(cmd_and_args.args);
            break;

        case ToUnderlying(ProjCfg::Command::GetInternalParameters):
            AcknowledgeCommand(cmd_and_args);
            getParametersCmd.Execute(cmd_and_args.args);
            break;

        default: i2c.Send(CommandAndArgs{ ProjCfg::UnknownCommand, cmd_and_args.args });
        }
    }

  private:
    OhmMeter meter;

    EnableOutputForPin      setVoltageCmd;
    GetInternalCounterValue getCounterCmd;
    CheckVoltages           checkVoltagesCmd;
    SetOutputVoltageLevel   setOutputVoltageCmd;
    ChangeAddress           changeAddressCmd;
    SetInternalParameters   setParametersCmd;
    GetInternalParameters   getParametersCmd;
};