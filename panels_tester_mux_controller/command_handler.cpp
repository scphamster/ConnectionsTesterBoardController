#include "command_handler.hpp"

extern "C" {
void (*USI_TWI_On_Slave_Transmit)(void) = nullptr;
void (*USI_TWI_On_Slave_Receive)(int)   = nullptr;
}

//Command::~Command() = default;

void
OnSlaveReceive(int)
{ }
