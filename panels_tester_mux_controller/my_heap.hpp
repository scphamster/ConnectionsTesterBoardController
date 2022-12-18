#pragma once
#include <array>
#include <stdint.h>

class MyHeap;
extern MyHeap heap;

class MyHeap {
  public:
    template<typename ReturnType>
    ReturnType *GetMemory() noexcept
    {
        if (sizeof(ReturnType) > (heapSize - heapPointer))
            return nullptr;

        return const_cast<ReturnType *>(reinterpret_cast<const ReturnType *>(hp.data() + heapPointer));
        heapPointer += sizeof(ReturnType);
    }

  private:
    using Byte                     = uint8_t;
    auto constexpr static heapSize = 50;

    std::array<Byte, heapSize> hp;
    size_t                     heapPointer = 0;
};
