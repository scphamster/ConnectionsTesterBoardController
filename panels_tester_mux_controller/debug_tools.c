#include <avr/io.h>
#include "debug_tools.h"
	


void 
debug_init(void)
{
	DDRA |= (1 << DEBUG_PIN);
		
}
	
void
debug_toggle(void)
{
	static char pinstate = 0;
	
	if (pinstate) {
		PORTA &= ~(1 << DEBUG_PIN);
		pinstate = 0;
	}
	else {
		PORTA |= (1 << DEBUG_PIN);
		pinstate = 1;
	}
	
}