#pragma once
#include <avr/io.h>

class EEPROMController {
  public:
    using AddressT = uint8_t;
    using DataT    = uint8_t;
    using Byte     = uint8_t;

    static void Write(AddressT address, DataT data) noexcept
    {
        while (EECR & (1 << EEPE))
            ;

        EEAR = address;
        EEDR = data;

        EECR |= (1 << EEMPE);
        EECR |= (1 << EEPE);
    }

    static DataT Read(AddressT address) noexcept
    {
        while (EECR & (1 << EEPE))
            ;

        EEAR = address;
        EECR |= (1 << EERE);
        return EEDR;
    }

    static bool DisableInterruptWriteAndCheck(AddressT address, DataT data) noexcept
    {
        uint8_t sreg_interrupt_status = SREG & _BV(SREG_I);
        if (sreg_interrupt_status) {
            cli();
        }

        Write(address, data);
        auto readback = Read(address);

        SREG |= sreg_interrupt_status;

        if (readback == data)
            return true;
        else
            return false;
    }
    template<typename DataType>
    static bool SuperWrite(AddressT address, DataType data) noexcept
    {
        auto buffer = reinterpret_cast<std::array<Byte, sizeof(data)> *>(&data);

        uint8_t sreg_interrupt_status = SREG & _BV(SREG_I);
        if (sreg_interrupt_status) {
            cli();
        }

        bool readout_is_ok = true;
        Byte byte_counter  = 0;
        for (auto const &byte : *buffer) {
            Write(address + byte_counter, byte);
            auto readback = Read(address);

            if (readback != byte)
                readout_is_ok = false;

            byte_counter++;
        }

        SREG |= sreg_interrupt_status;
        return readout_is_ok;
    }
};