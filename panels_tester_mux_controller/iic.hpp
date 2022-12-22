#pragma once
#include <avr/io.h>
#include <memory>
#include <optional>

#include "timer.hpp"
#include "USI_TWI_Slave.h"
#include "USI_TWI_Master.h"

class IIC;

extern IIC *i2c_driver;

class IIC {
  public:
    using AddrT = uint8_t;
    using Byte  = uint8_t;

    enum class Role {
        Master,
        Slave
    };

    void static Create(Role role, AddrT address);
    IIC static *Get() { return _this; }

    [[nodiscard]] bool RXBufferHasData() const noexcept { return USI_TWI_Data_In_Receive_Buffer(); }

    //    template<typename ReturnType>
    //    std::pair<bool, ReturnType> Receive(size_t timeout_ms)
    //    {
    //        if (timeout_ms < 100)
    //            timeout_ms = 1;
    //
    //        auto timeout_timestamp = timeout_ms / 100 + Timer8::GetCounterValue();
    //
    //        while (Timer8::GetCounterValue() < timeout_timestamp) {
    //            if (GetNumberOfBytesInRXBuffer() < sizeof(ReturnType))
    //                continue;
    //
    //            std::array<Byte, sizeof(ReturnType)> buffer;
    //
    //            for (auto &byte : buffer) {
    //                byte = USI_TWI_Receive_Byte();
    //            }
    //
    //            return { true, *(reinterpret_cast<ReturnType *>(buffer.data())) };
    //        }
    //
    //        return { false, ReturnType{} };
    //    }

    //    template<typename ReturnType>
    //    std::pair<bool, ReturnType> Receive()
    //    {
    //        while (true) {
    //            if (GetNumberOfBytesInRXBuffer() < sizeof(ReturnType))
    //                continue;
    //
    //            std::array<Byte, sizeof(ReturnType)> buffer;
    //
    //            for (auto &byte : buffer) {
    //                byte = USI_TWI_Receive_Byte();
    //            }
    //
    //            return { true, *(reinterpret_cast<ReturnType *>(buffer.data())) };
    //        }
    //    }
    template<typename ReturnType>
    ReturnType ReceiveBlocking() noexcept
    {
        while (true) {
            //            if (GetNumberOfBytesInRXBuffer() < sizeof(ReturnType))
            //                continue;

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
    IIC(Role role, AddrT address);
    IIC static *_this;
};
