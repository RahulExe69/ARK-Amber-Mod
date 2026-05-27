//
// VMTInvoker.hpp - Android/Linux (ELF) port
// Created by Euclid Jan Guillermo (iOS original)
//
//

#pragma once

#include <cstdint>
#include <sys/mman.h>
#include <unistd.h>
#include <utility>

template<typename FuncType>
class VMTInvokerBase
{
protected:
    void* Instance;
    int32_t FunctionIndex;
    FuncType* OriginalFunction;

public:
    VMTInvokerBase(void* instance, int32_t index) : Instance(instance), FunctionIndex(index), OriginalFunction(nullptr) {}

    template<typename... Args>
    auto Invoke(Args&&... args) const -> decltype(auto)
    {
        return (*OriginalFunction)(std::forward<Args>(args)...);
    }
};

template<typename FuncType>
class VMTInvoker : public VMTInvokerBase<FuncType>
{
public:
    using VMTInvokerBase<FuncType>::Instance;
    using VMTInvokerBase<FuncType>::FunctionIndex;
    using VMTInvokerBase<FuncType>::OriginalFunction;

    VMTInvoker(void* instance, int32_t index) : VMTInvokerBase<FuncType>(instance, index)
    {
        void** VTable = *reinterpret_cast<void***>(instance);
        if (VTable)
        {
            OriginalFunction = reinterpret_cast<FuncType*>(VTable[FunctionIndex]);
        }
    }
};
