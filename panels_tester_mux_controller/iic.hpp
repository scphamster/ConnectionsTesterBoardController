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

    IIC(Role role, AddrT address)
    {
        if (role == Role::Slave) {
            USI_TWI_Slave_Initialise(address);
        }
        else {
            while (true)
                ;
        }
    }

    void SetNewAddress(AddrT new_address) noexcept {
        USI_TWI_Slave_Initialise(new_address);
    }

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
        auto buffer = std::array<Byte, sizeof(ReturnType)>{};

        auto endTime = Timer8::GetCounterValue() + timeout_ms / timer8_period_ms;

        for (auto &byte : buffer) {
            while (USI_TWI_Data_In_Receive_Buffer() == 0) {
                if (endTime < Timer8::GetCounterValue()) {
                    return std::nullopt;
                }
            }
            byte = USI_TWI_Receive_Byte();
        }

        return *(reinterpret_cast<ReturnType *>(buffer.data()));
    }
    template<typename ValueT>
    void Send(ValueT value) noexcept
    {
        for (auto &byte : *reinterpret_cast<std::array<Byte, sizeof(ValueT)> *>(&value)) {
            USI_TWI_Transmit_Byte(byte);
        }
    }

  protected:
    [[nodiscard]] uint8_t GetNumberOfBytesInRXBuffer() const noexcept { return USI_TWI_Data_In_Receive_Buffer(); }

  private:
};
