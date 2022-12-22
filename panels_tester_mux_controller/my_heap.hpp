#pragma once
#include <array>
#include <cstdint>
#include "project_configs.h"

class MyHeap;
extern MyHeap heap;

class MyHeap {
  public:
    template<typename ReturnType>
    ReturnType *GetMemory() noexcept
    {
        if (sizeof(ReturnType) > (cfg_heap_size - heapPointer))
            //            while (true) { }
            return nullptr;
        return const_cast<ReturnType *>(reinterpret_cast<const ReturnType *>(heap_data.data() + heapPointer));
        heapPointer += sizeof(ReturnType);
    }

  private:
    using Byte = uint8_t;
    std::array<Byte, cfg_heap_size> heap_data;
    size_t                          heapPointer = 0;
};
