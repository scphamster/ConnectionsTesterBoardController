#include "timer.hpp"
#include <avr/interrupt.h>

bool Timer8::mutex = false;
Timer8::TimerValue Timer8::counter = 0;

extern "C" {
ISR(TIM0_COMPA_vect) {
    Timer8::IncrementCounter();
}
}