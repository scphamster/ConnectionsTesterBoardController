#pragma once
#include <avr/io.h>
#include <memory>
#include <optional>

#include "timer.hpp"
#include "USI_TWI_Slave.h"
#include "USI_TWI_Master.h"

class IIC;

// extern IIC *i2c_driver;
extern IIC i2c;

class IIC {
  public:
    using AddrT = uint8_t;
    using Byte  = uint8_t;

    enum class Role {
        Master,
        Slave
    };

    IIC(Role role, AddrT address);

    [[nodiscard]] bool RXBufferHasData() const noexcept { return USI_TWI_Data_In_Receive_Buffer(); }
    template<typename ReturnType>
    ReturnType ReceiveBlocking() noexcept
    {
        while (true) {
            ReturnType                            retval;
            std::array<Byte, sizeof(ReturnType)> *bytes =
              reinterpret_cast<std::array<Byte, sizeof(ReturnType)> *>(&retval);

            for (auto &byte : *bytes) {
                byte = USI_TWI_Receive_Byte();
            }

            return retval;
        }
    }

    template<typename ReturnType>
    std::optional<ReturnType> Receive() noexcept
    {
        if (GetNumberOfBytesInRXBuffer() < sizeof(ReturnType))
            return std::nullopt;

        ReturnType retval;
        auto      *bytes = reinterpret_cast<std::array<Byte, sizeof(ReturnType)> *>(&retval);

        for (auto &byte : *bytes) {
            byte = USI_TWI_Receive_Byte();
        }

        return retval;
    }
    template<typename ReturnType>
    std::optional<ReturnType> Receive(uint32_t timeout_ms) noexcept
    {
        ReturnType retval;
        auto      *buffer = reinterpret_cast<std::array<Byte, sizeof(ReturnType)> *>(&retval);

        auto endTime = Timer8::GetCounterValue() + timeout_ms / timer8_period_ms;

        for (auto &byte : *buffer) {
            while (USI_TWI_Data_In_Receive_Buffer() == 0) {
                if (endTime < Timer8::GetCounterValue()) {
                    return std::nullopt;
                }
            }
            byte = USI_TWI_Receive_Byte();
        }

        return retval;
    }

    template<typename ValueT>
    bool Send(ValueT value) noexcept
    {
        std::array<Byte, sizeof(ValueT)> *array = reinterpret_cast<std::array<Byte, sizeof(ValueT)> *>(&value);

        for (auto &byte : *array) {
            USI_TWI_Transmit_Byte(byte);
        }

        return true;
    }

  protected:
    [[nodiscard]] uint8_t GetNumberOfBytesInRXBuffer() const noexcept { return USI_TWI_Data_In_Receive_Buffer(); }

  private:
};
