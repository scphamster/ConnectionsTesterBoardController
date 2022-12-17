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

#define MY_ADDRESS 0x25

class Command {
  public:
    using Byte = uint8_t;

    Command(Byte cmd)
      : command{ cmd }
    { }

    Byte         GetCommand() const noexcept { return command; }
    Byte         ReverseBits(Byte data) { return ~data; }
    virtual bool Execute() noexcept = 0;

    template<typename ReturnType>
    ReturnType GetArguments() noexcept
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

        Arguments args = GetArguments<Arguments>();

        USI_TWI_Transmit_Byte(args.pinNumber);
        USI_TWI_Transmit_Byte(args.voltageLevel);

        return true;
    }

  protected:
    struct Arguments {
        Byte pinNumber;
        Byte voltageLevel;
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
        std::array<Byte, sizeof(uint32_t)> buf;
        *(reinterpret_cast<uint32_t *>(buf.data())) = Timer8::GetCounterValue();
//        *(reinterpret_cast<uint32_t *>(buf.data())) = 123456;

        for (auto const byteval : buf) {
            USI_TWI_Transmit_Byte(byteval);
        }

        return true;
    }
};

class CommandHandler {
  public:
    using Byte     = uint8_t;
    using CommandT = Byte;

    CommandHandler() { Init(); }

    [[noreturn]] void MainLoop() noexcept
    {
        Timer8::Init(Timer8::Clock::_1024, Timer8::Waveform::CTC, 78);

        while (true) {
            if (USI_TWI_Data_In_Receive_Buffer()) {
                auto twi_data = USI_TWI_Receive_Byte();
                HandleCommand(twi_data);
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

    void Init() noexcept
    {
        void OnSlaveReceive(int);

        sei();
        USI_TWI_Slave_Initialise(MY_ADDRESS);
    }

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

        default: USI_TWI_Transmit_Byte(cmd);
        }
    }

    Byte static ReverseBits(Byte data) noexcept { return ~data; }
    void static WaitForArguments() noexcept
    {
        // todo: some watchdog is required here to elliminate foreverloop of waiting for new i2c data;

        while (true) { }
    }

  private:
    auto constexpr static commandsNumber         = 3;
    Byte constexpr static unknownCommandResponse = 0x5a;
    std::array<Byte, commandsNumber> static constexpr commands{ 0xC1, 0xC2, 0xC3 };

    CurrentState currentState{ CurrentState::WaitingForNewCommand };

    SetPinVoltage           setVoltageCmd;
    GetInternalCounterValue getCounterCmd;
};