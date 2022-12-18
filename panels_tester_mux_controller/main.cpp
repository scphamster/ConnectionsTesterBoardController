#include <avr/io.h>

#include "command_handler.hpp"

void initialization() noexcept {
    ADCHandler::Create();
    IIC::Create(IIC::Role::Slave, MY_ADDRESS);

    sei();
}

int
main()
{
    initialization();
    auto  cmd_handler = CommandHandler{};

    cmd_handler.MainLoop();

    while (true) { }
}
