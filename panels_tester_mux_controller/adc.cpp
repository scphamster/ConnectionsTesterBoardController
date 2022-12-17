#include "adc.hpp"
#include <avr/io.h>
#include <avr/interrupt.h>

bool       ADCHandler::initialized = false;
ADCHandler adc_handle;

 extern "C" {
 ISR(ADC_vect)
 {
     uint16_t res_low = ADCL;
     adc_handle.SetLastValue(res_low | (ADCH << 8));
//         adc_handle.SetLastValue(125);
 }
 }