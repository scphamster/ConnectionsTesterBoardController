#include "shifter.hpp"
#include "my_heap.hpp"

template<>
Shifter<shifter_size> *Shifter<shifter_size>::_this = nullptr;
template<>
Shifter<shifter_size>::PinStateT Shifter<shifter_size>::dummyValue = Shifter<shifter_size>::PinStateT ::Low;