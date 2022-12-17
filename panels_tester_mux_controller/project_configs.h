#pragma once

#include <avr/io.h>

#ifdef __cplusplus
extern "C" {

#endif

/////////// GENERAL /////////////////
#define F_CPU 8000000UL

////////// USI SECTION //////////////
#define USI_PORT      PORTA
#define USI_PIN       PINA
#define USI_DDR       DDRA
#define USI_SCL_BIT 4
#define USI_SDA_BIT    6
#define USI_DO_BIT    5

#ifdef __cplusplus
}
#endif