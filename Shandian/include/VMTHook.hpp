//
// VMTHook.hpp - Android/Linux (ELF) port
// android port, thanks Euclid Jan G.
//
#pragma once

#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <utility>
#include <sys/mman.h>
#include <unistd.h>

template<typename FuncType>
class VMTHook
{
private:
    void** OriginalVTable;
    void** DetourVTable;
    FuncType* OriginalFunction;
    FuncType* NewFunction;
    int32_t FunctionIndex;
    void* HookedInstance;

    int GetNumMethods(void** VTable) const
    {
        int Count = 0;
        while (VTable[Count])
        {
            // Bounds check: pointer should be in a reasonable code range
            uintptr_t addr = reinterpret_cast<uintptr_t>(VTable[Count]);
            if (addr < 0x1000 || addr > 0x7FFFFFFFFFFF)
                break;
            ++Count;
        }
        return Count;
    }

    bool IsValidPointer(uintptr_t address)
    {
        // ELF address space on Android arm64
        return address >= 0x1000 && address < 0x7FFFFFFFFFFF;
    }

    bool MakeWritable(void* addr, size_t len)
    {
        uintptr_t page = reinterpret_cast<uintptr_t>(addr) & ~(uintptr_t)0xFFF;
        return mprotect(reinterpret_cast<void*>(page), len + 0x1000, PROT_READ | PROT_WRITE) == 0;
    }

public:
    VMTHook(FuncType* NewFunc, int32_t Index) : NewFunction(NewFunc), FunctionIndex(Index), HookedInstance(nullptr) {}

    void Swap(void* Class)
    {
        if (!Class || !NewFunction)
            return;

        void** VTable = *reinterpret_cast<void***>(Class);
        if (!VTable || VTable[FunctionIndex] == reinterpret_cast<void*>(NewFunction) || !IsValidPointer(reinterpret_cast<uintptr_t>(VTable)))
            return;

        // If the instance changed, reset first
        if (HookedInstance && HookedInstance != Class)
        {
            Reset(HookedInstance);
        }

        int MethodCount = GetNumMethods(VTable);
        DetourVTable = static_cast<void**>(malloc(MethodCount * sizeof(void*)));
        if (!DetourVTable)
            return;

        memcpy(DetourVTable, VTable, MethodCount * sizeof(void*));
        OriginalVTable = VTable;
        OriginalFunction = reinterpret_cast<FuncType*>(DetourVTable[FunctionIndex]);
        DetourVTable[FunctionIndex] = reinterpret_cast<void*>(NewFunction);

        MakeWritable(Class, sizeof(void*));
        *(void***)Class = DetourVTable;
    }

    void Reset(void* Class)
    {
        if (Class && OriginalVTable)
        {
            MakeWritable(Class, sizeof(void*));
            *(void***)Class = OriginalVTable;
        }
        if (DetourVTable)
        {
            free(DetourVTable);
            DetourVTable = nullptr;
        }
        OriginalVTable = nullptr;
        HookedInstance = nullptr;
    }

    bool IsHooked() const { return DetourVTable != nullptr; }

    template<typename... Args>
    auto InvokeOriginal(Args&&... args) const -> decltype(auto)
    {
        return (*OriginalFunction)(std::forward<Args>(args)...);
    }
};
