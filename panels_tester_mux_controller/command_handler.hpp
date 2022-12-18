#pragma once
#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <array>
#include <functional>
#define F_CPU 8000000UL
#include <util/delay.h>

#include "pio_controller.hpp"
#include "shifter.hpp"
#include "analog_switch.hpp"

#include "USI_TWI_Slave.h"
#include "adc.hpp"
#include "timer.hpp"

#include "iic.hpp"

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
        std::array<Byte, sizeof(ReturnType)> buffer{};
        size_t                               byte_counter = 0;

        for (auto &byte : buffer) {
            while (not USI_TWI_Data_In_Receive_Buffer())
                ;

            byte = USI_TWI_Receive_Byte();
        }

        return *(reinterpret_cast<ReturnType *>(buffer.data()));
    }

  protected:
    IIC *i2c = nullptr;

  private:
    Byte command;
};

class SetPinVoltage : public Command {
  public:
    SetPinVoltage()
      : Command{ 0xC1 }
    { }

    bool Execute() noexcept override
    {
        // wait for arguments from master
        auto args = WaitForCommandArgs<Arguments>();
        i2c->Send(args);

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
    CheckVoltages()
      : Command{ 0xC3 }
      , adc{ ADCHandler::Get() }
    { }

    bool Execute() noexcept override
    {
        auto args = WaitForCommandArgs<Arguments>();
        i2c->Send(args);

        switch (args.pin) {
        case SpecialMeasurements::MeasureVCC: CheckVCC(); break;
        case SpecialMeasurements::MeasureGND: CheckGND(); break;
        case SpecialMeasurements::MeasureAll: break;

        default:
            if (args.pin > 31)
                return false;
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

  private:
    ADCHandler *adc;
};

class CommandHandler {
  public:
    using Byte     = uint8_t;
    using CommandT = Byte;

    CommandHandler()
      : i2c{ IIC::Get() }
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
            USI_TWI_Transmit_Byte(ReverseBits(cmd));
            setVoltageCmd.Execute();
            break;

        case 0xC2:
            USI_TWI_Transmit_Byte(ReverseBits(cmd));
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

    IIC *i2c = nullptr;

    CurrentState currentState{ CurrentState::WaitingForNewCommand };

    SetPinVoltage           setVoltageCmd;
    GetInternalCounterValue getCounterCmd;
    CheckVoltages           checkVoltagesCmd;
};