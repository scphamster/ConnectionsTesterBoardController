#pragma once

#include <avr/io.h>

#ifdef __cplusplus
extern "C" {

#endif

/////////// GENERAL /////////////////
#define F_CPU 8000000UL

////////// USI SECTION //////////////
#define USI_PORT    PORTA
#define USI_PIN     PINA
#define USI_DDR     DDRA
#define USI_SCL_BIT 4
#define USI_SDA_BIT 6
#define USI_DO_BIT  5

///////////////////////////// Heap //////////////////////////////////
uint8_t const static cfg_heap_size = 50;

////////////////////// Mux Section ////////////////////////////////////////////
uint8_t const static mux_pairs_on_board  = 2;
uint8_t const static mux_pins_number     = 16;
uint8_t const static total_mux_pin_count = 32;

//////////////////////////TIMER ///////////////////////////////////////
uint8_t const static timer8_ctc_value = 156;   // 20ms period if f main = 8MHz
uint8_t const static timer8_period_ms = 20;
/////////////// Output voltage value selection shifter pins ////////////////////
uint8_t const static high_voltage_reference_select_pin = 20;
uint8_t const static low_voltage_reference_select_pin  = 21;

uint8_t const static enable_voltage_at_pin_cmd_id  = 0xc1;
uint8_t const static check_internal_counter_cmd_id = 0xc2;
uint8_t const static check_voltages_cmd_id         = 0xc3;
uint8_t const static set_output_voltage_cmd_id     = 0xc4;
#ifdef __cplusplus
}
#endif