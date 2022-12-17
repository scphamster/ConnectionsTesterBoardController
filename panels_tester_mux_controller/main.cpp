#include <avr/io.h>

#include "command_handler.hpp"

int
main()
{
    auto  cmd_handler = CommandHandler{};

    cmd_handler.MainLoop();

    while (true) { }
}
