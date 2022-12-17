#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#ifdef __cplusplus
extern "C" {
	#endif

uint8_t new_data_arrived = 0;
uint16_t last_adc_value = 123;

//ISR(ADC_vect)
//{
//	uint16_t low =  ADCL;
//	last_adc_value = (low | (ADCH << 8));
//	new_data_arrived = 1;
//}


#ifdef __cplusplus
}
#endif