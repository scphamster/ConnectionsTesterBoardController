#pragma once
#include <avr/io.h>
#include <stdlib.h>

class ADCHandler;

extern ADCHandler adc_handle;

class ADCHandler {
  public:
    using RegT    = uint8_t;
    using ResultT = uint16_t;
    enum Masks {
        MUXOnlyChanels      = 0x3f,
        MUXOnlyReference    = 0xc0,
        ADCSRA_NoPrescaller = 0xf8
    };
    enum class Reference {
        VCC         = 0,
        External    = 0x40,
        Internal1v1 = 0x80
    };
    enum class SingleChannel {
        _0 = 0,
        _1,
        _2,
        _3,
        _4,
        _5,
        _6,
        _7,
        GND     = 32,
        _1v1Ref = 33,
        ADC8    = 34
    };
    enum class Prescaller {
        _2 = 0,
        _4 = 2,
        _8,
        _16,
        _32,
        _64,
        _128
    };

    static void Init() noexcept
    {
        Enable();

        SetPrescaler(Prescaller::_64);
        SetReference(Reference::VCC);
        SetSingleChannel(SingleChannel::_1v1Ref);

        DisableDigitalFunction(0);
    }
    static void Enable() noexcept
    {
        PRR &= ~_BV(PRADC);
        ADCSRA |= _BV(ADEN);
    }
//    static void Disable() noexcept { ADCSRA &= ~_BV(ADEN); }
//    static void EnableInterrupt() noexcept
//    {
//        ADCSRA |= _BV(ADIF);
//        ADCSRA |= _BV(ADIE);
//    }
//    static void DisableInterrupt() noexcept { ADCSRA &= ~_BV(ADIE); }
    static void SetPrescaler(Prescaller ps) noexcept
    {
        auto status_no_prescaller = ADCSRA bitand static_cast<RegT>(ADCSRA_NoPrescaller);
        ADCSRA                    = status_no_prescaller | static_cast<RegT>(ps);
    }
    static void SetSingleChannel(SingleChannel channel) noexcept
    {
        auto mux_reference = ADMUX bitand MUXOnlyReference;
        ADMUX              = static_cast<RegT>(channel) | mux_reference;
    }
    static void SetReference(Reference new_ref) noexcept
    {
        auto mux_chanels = ADMUX bitand MUXOnlyChanels;

        ADMUX = static_cast<RegT>(new_ref) | mux_chanels;
    }
//    static void ClearInterrupt() noexcept { ADCSRA |= _BV(ADIF); }
    static void StartConversion() noexcept { ADCSRA |= _BV(ADSC); }

    static ResultT MakeSingleConversion() noexcept
    {
        if (not ConversionIsInProgress())
            StartConversion();
        else
            return 0;

        while (ConversionIsInProgress())
            ;

        return ReadResult();
    }
    static ResultT ReadResult() noexcept
    {
        ResultT result = ADCL;
        return result | (ADCH << 8);
    }
    static bool ConversionIsInProgress() noexcept { return (ADCSRA bitand _BV(ADSC)); }

  protected:
    static void DisableDigitalFunction(int pin) noexcept
    {
        if (pin > 7)
            while (true)
                ;

        DIDR0 = (1 << pin);
    }

  private:
};