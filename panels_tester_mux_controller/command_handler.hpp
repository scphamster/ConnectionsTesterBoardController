#pragma once
#include "project_configs.h"

#include <avr/io.h>
#include <cstdlib>
#include <avr/interrupt.h>
#include <array>
#include <functional>

#include "shifter.hpp"
#include "analog_switch.hpp"
#include "adc.hpp"
#include "timer.hpp"
#include "iic.hpp"
#include "ohm_meter.hpp"

#define MY_ADDRESS 0x25

class Command {
  public:
    using Byte = uint8_t;

    Command(Byte cmd)
      : command{ cmd }
      , i2c{ IIC::Get() }
    { }

    Byte         GetCommand() const noexcept { return command; }
    Byte         ReverseBits(Byte data) { return ~data; }
    virtual bool Execute() noexcept = 0;

    template<typename ReturnType>
    ReturnType WaitForCommandArgs() noexcept
    {
        auto result = i2c->Receive<ReturnType>();
        return result.second;
    }

  protected:
    IIC *i2c = nullptr;

  private:
    Byte command;
};

class SetPinVoltage : public Command {
  public:
    using Multimeter = OhmMeter<Shifter<shifter_size>, mux_pairs_on_board>;

    SetPinVoltage(Multimeter &new_meter)
      : Command{ 0xC1 }
      , meter{ new_meter }
    { }

    bool Execute() noexcept override
    {
        // wait for arguments from master
        auto args = WaitForCommandArgs<Arguments>();
        i2c->Send(args);

        meter.EnableOutputChannel(args.pinNumber);

        return true;
    }

  protected:
    struct Arguments {
        Byte pinNumber;
        Byte voltageLevel;
    };
    struct ReturnType {
        int someInt = 12345;
        //        float someFloat = 33.55f;
        char someChar = 'a';
    };

  private:
    Multimeter &meter;
};

class GetInternalCounterValue : public Command {
  public:
    GetInternalCounterValue()
      : Command{ 0xC2 }
    { }

    bool Execute() noexcept override
    {
        i2c->Send(Timer8::GetCounterValue());

        return true;
    }

  protected:
  private:
};

class CheckVoltages : public Command {
  public:
    using Multimeter = OhmMeter<Shifter<shifter_size>, mux_pairs_on_board>;
    using PinT       = uint8_t;

    CheckVoltages(Multimeter &new_meter)
      : Command{ 0xC3 }
      , adc{ ADCHandler::Get() }
      , meter{ new_meter }
    { }

    bool Execute() noexcept override
    {
        auto args = WaitForCommandArgs<Arguments>();
        i2c->Send(args);

        switch (args.pin) {
        case SpecialMeasurements::MeasureVCC: CheckVCC(); break;
        case SpecialMeasurements::MeasureGND: CheckGND(); break;
        case SpecialMeasurements::MeasureAll: CheckAll(); break;

        default:
            if (args.pin > 31)
                return false;
            CheckOne(args.pin);
        }

        return false;
    }

  protected:
    enum SpecialMeasurements {
        MeasureAll = 33,
        MeasureVCC,
        MeasureGND
    };
    struct Arguments {
        using PinNum = uint8_t;
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
    void CheckAll() noexcept
    {
//        auto testval = std::array<uint16_t, 5>{ 1, 2, 3, 4, 5, /*6, 7, 8, 9, 10 */};

        i2c->Send(meter.GetAllPinsVoltage());
//        i2c->Send(uint16_t{55});
    }
    void CheckOne(PinT pin) noexcept { i2c->Send(meter.GetPinVoltage(pin)); }

  private:
    ADCHandler *adc;
    Multimeter &meter;
};

class CommandHandler {
  public:
    using Byte       = uint8_t;
    using CommandT   = Byte;
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
    {
        Init();
    }
    [[noreturn]] void MainLoop() noexcept
    {
        Timer8::Init(Timer8::Clock::_1024, Timer8::Waveform::CTC, 78);
        auto i2c_driver = IIC::Get();

        while (true) {
            //            if (USI_TWI_Data_In_Receive_Buffer()) {
            //                auto twi_data = USI_TWI_Receive_Byte();
            //                HandleCommand(twi_data);
            //            }
            //
            auto value = i2c_driver->Receive<Byte>();
            if (value.first) {
                HandleCommand(value.second);
            }
        }
    }

  protected:
    enum class CurrentState {
        WaitingForNewCommand,
        CommandReceived_WaitingForArgument,
        CommandReceived_Executing,
        ExecutionCompleted_WaitingForCommand
    };

    void Init() noexcept { }

    void HandleCommand(CommandT cmd) noexcept
    {
        CommandT answer{ unknownCommandResponse };

        switch (cmd) {
        case 0xC1:
            i2c->Send(ReverseBits(cmd));
            setVoltageCmd.Execute();
            break;

        case 0xC2:
            i2c->Send(ReverseBits(cmd));
            getCounterCmd.Execute();
            break;

        case 0xC3:
            i2c->Send(ReverseBits(cmd));
            checkVoltagesCmd.Execute();
            break;

        default: USI_TWI_Transmit_Byte(cmd);
        }
    }

    Byte static ReverseBits(Byte data) noexcept { return ~data; }

  private:
    auto constexpr static commandsNumber         = 3;
    Byte constexpr static unknownCommandResponse = 0x5a;
    std::array<Byte, commandsNumber> static constexpr commands{ 0xC1, 0xC2, 0xC3 };

    IIC       *i2c = nullptr;
    Multimeter meter;

    CurrentState currentState{ CurrentState::WaitingForNewCommand };

    SetPinVoltage           setVoltageCmd;
    GetInternalCounterValue getCounterCmd;
    CheckVoltages           checkVoltagesCmd;
};