#pragma once
#include <avr/io.h>

class EEPROMController {
  public:
    using AddressT = uint8_t;
    using DataT = uint8_t;

    static void Write(AddressT address, DataT data) noexcept {
        while(EECR & (1 << EEPE));

        EEAR = address;
        EEDR = data;

        EECR |= (1 << EEMPE);
        EECR |= (1<<EEPE);
    }

    static DataT Read(AddressT address) noexcept {
        while(EECR & (1 << EEPE));

        EEAR = address;
        EECR |= (1 << EERE);
        return EEDR;
    }
};