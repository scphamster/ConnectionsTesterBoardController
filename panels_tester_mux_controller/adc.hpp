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

    void static Create() noexcept;
    ADCHandler static *Get() noexcept { return _this; }
    void               Enable() noexcept
    {
        PRR &= ~_BV(PRADC);
        ADCSRA |= _BV(ADEN);
    }
    void Disable() noexcept { ADCSRA &= ~_BV(ADEN); }
    void EnableInterrupt() noexcept
    {
        ADCSRA |= _BV(ADIF);
        ADCSRA |= _BV(ADIE);
    }
    void DisableInterrupt() noexcept { ADCSRA &= ~_BV(ADIE); }
    void SetPrescaller(Prescaller ps) noexcept
    {
        auto status_no_prescaller = ADCSRA bitand static_cast<RegT>(ADCSRA_NoPrescaller);
        ADCSRA                    = status_no_prescaller | static_cast<RegT>(ps);
    }
    void SetSingleChannel(SingleChannel channel) noexcept
    {
        auto mux_reference = ADMUX bitand MUXOnlyReference;
        ADMUX              = static_cast<RegT>(channel) | mux_reference;
    }
    void SetReference(Reference new_ref) noexcept
    {
        auto mux_chanels = ADMUX bitand MUXOnlyChanels;

        ADMUX = static_cast<RegT>(new_ref) | mux_chanels;
    }
    void ClearInterrupt() const noexcept { ADCSRA |= _BV(ADIF); }
    void StartConversion() noexcept { ADCSRA |= _BV(ADSC); }
    void SetLastValue(ResultT value) noexcept
    {
        lastValue      = value;
        newDataArrived = true;
    }

    ResultT MakeSingleConversion() noexcept
    {
        if (not ConversionIsInProgress())
            StartConversion();
        else
            return 0;

        while (ConversionIsInProgress())
            ;

        return ReadResult();
    }
    ResultT ReadResult() const noexcept
    {
        ResultT result = ADCL;
        return result | (ADCH << 8);
    }
    ResultT GetLastValue() noexcept
    {
        newDataArrived = false;
        //        new_data_arrived = 0;
        return lastValue;
        //        return last_adc_value;
    }
    bool ConversionIsInProgress() const noexcept { return (ADCSRA bitand _BV(ADSC)); }
    bool InterruptHappened() const noexcept { return (ADCSRA bitand _BV(ADIF)); }
    bool NewDataArrived() const noexcept { return newDataArrived; }

  protected:
    void DisableDigitalFunction(int pin) noexcept
    {
        if (pin > 7)
            while (true)
                ;

        DIDR0 = (1 << pin);
    }

  private:
    ADCHandler()
    {
        if (initialized)
            while (true)
                ;

        //todo: make configurable
        Enable();
        EnableInterrupt();
        SetPrescaller(Prescaller::_64);
        SetReference(Reference::VCC);
        SetSingleChannel(SingleChannel::_1v1Ref);

        DisableDigitalFunction(0);

        initialized = true;
    }

    ADCHandler static *_this;

    bool static initialized;
    bool    newDataArrived = false;
    ResultT lastValue{ 0 };
};