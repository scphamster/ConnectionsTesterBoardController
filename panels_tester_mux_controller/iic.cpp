#include "iic.hpp"
#include "my_heap.hpp"

IIC *IIC::_this = nullptr;

IIC::IIC(Role role, AddrT address)
{
    if (role == Role::Slave) {
        USI_TWI_Slave_Initialise(address);
    }
    else {
        while (true)
            ;
    }
}

void
IIC::Create(Role role, AddrT address)
{
    if (_this == nullptr) {
        _this = heap.GetMemory<IIC>();
    }

    *_this = IIC{ role, address };
}