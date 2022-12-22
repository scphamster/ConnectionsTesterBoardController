#include "ohm_meter.hpp"

std::array<AnalogSwitch, 2> OhmMeter::outputMuxes{ AnalogSwitch{ AnalogSwitchPins{ 10, 14, 13, 12, 11 } },
                                                   AnalogSwitch{ AnalogSwitchPins{ 15, 19, 18, 17, 16 } } };

std::array<AnalogSwitch, 2> OhmMeter::inputMuxes{ AnalogSwitch{ AnalogSwitchPins{ 0, 4, 3, 2, 1 } },
                                                   AnalogSwitch{ AnalogSwitchPins{ 5, 9, 8, 7, 6 } } };