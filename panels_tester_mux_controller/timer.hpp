#pragma once
#include <avr/io.h>
#include <memory>
#include <array>
#include <functional>

class Timer8 {
  public:
    using RegT       = uint8_t;
    using TimerValue = uint16_t;

    enum class Clock {
        NoClock = 0,
        _1,
        _8,
        _64,
        _256,
        _1024,
        ExternalFalling,
        ExternalRising
    };

    enum class Waveform {
        Normal = 0,
        PWM_PhaseCorrect_0xFF,
        CTC,
        PWM_Fast,
        PWM_PhaseCorrect_OCRA = 5,
        PWM_Fast_OCRA         = 7
    };

    void static Init(Clock clock, Waveform wave, RegT OCR_value)
    {
        if (mutex)
            return;

        mutex = true;

        TCCR0A = (static_cast<RegT>(wave) bitand 0x03);
        TCCR0B = ((static_cast<RegT>(wave) bitand 0x04) << 1) bitor (static_cast<RegT>(clock) bitand 0x07);
        TIMSK0 = 0x02; // compare A interrupt
    }

    void static IncrementCounter() noexcept { counter++; }
    TimerValue static GetCounterValue() noexcept { return counter; }

  protected:
  private:
    // mutex to prevent write from different places simultaneously
    bool static mutex;
    TimerValue static counter;
};