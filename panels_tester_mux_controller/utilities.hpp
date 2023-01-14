#pragma once

#include <type_traits>

consteval auto ToUnderlying(auto argument) {
    return static_cast<std::underlying_type_t<decltype(argument)>>(argument);
}