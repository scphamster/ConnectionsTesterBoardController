#include "adc.hpp"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "my_heap.hpp"

bool        ADCHandler::initialized = false;
ADCHandler *ADCHandler::_this       = nullptr;

extern "C" {
ISR(ADC_vect)
{
    //     uint16_t res_low = ADCL;
    //     adc_handle.SetLastValue(res_low | (ADCH << 8));
    //         adc_handle.SetLastValue(125);
}
}

void
ADCHandler::Create() noexcept
{
    if (_this == nullptr) {
        _this = heap.GetMemory<ADCHandler>();
    }

    *_this = ADCHandler{};
}