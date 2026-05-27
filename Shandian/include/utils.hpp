#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>

using i8  = std::int8_t;
using u8  = std::uint8_t;
using i16 = std::int16_t;
using u16 = std::uint16_t;
using i32 = std::int32_t;
using u32 = std::uint32_t;
using i64 = std::int64_t;
using u64 = std::uint64_t;

namespace Mem {
    struct MapRange { u64 start, end; };
    static MapRange g_Ranges[128];
    static i32      g_RangeCount = 0;

    inline void InitMaps() {
        if (g_RangeCount) return;
        FILE* f = fopen("/proc/self/maps", "r");
        if (!f) return;
        char line[256];
        while (fgets(line, sizeof(line), f) && g_RangeCount < 128) {
            u64 start, end;
            char perm[5] = {};
            if (sscanf(line, "%lx-%lx %4c", &start, &end, perm) == 3) {
                if (perm[0] == 'r') g_Ranges[g_RangeCount++] = {start, end};
            }
        }
        fclose(f);
    }

    inline bool Valid(void* p) {
        InitMaps();
        auto a = (u64)p;
        if (a < 0x1000) return false;
        for (i32 i = 0; i < g_RangeCount; i++)
            if (a >= g_Ranges[i].start && a < g_Ranges[i].end)
                return true;
        return false;
    }

    template<typename T> T Read(u64 a) {
        return Valid((void*)a) ? *(T*)a : T{};
    }
    template<typename T> void Write(u64 a, T v) {
        if (Valid((void*)a)) *(T*)a = v;
    }

    inline void PatchVt(void** vt, i32 idx, void* fn) {
        u64 pg = (u64)&vt[idx] & ~(u64)0xFFF;
        mprotect((void*)pg, 0x1000, PROT_READ | PROT_WRITE);
        vt[idx] = fn;
    }

    inline u64 Base() {
        static u64 b = 0;
        if (b) return b;
        InitMaps();
        FILE* f = fopen("/proc/self/maps", "r");
        if (!f) return 0;
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, "libUE4.so") && line[0] != ' ') {
                b = strtoull(line, 0, 16);
                break;
            }
        }
        fclose(f);
        return b;
    }
}

namespace Bits {
    template<typename T>
    inline void Set(T& val, T mask)   { val |= mask; }
    template<typename T>
    inline void Clear(T& val, T mask) { val &= ~mask; }
    template<typename T>
    inline void Toggle(T& val, T mask) { val ^= mask; }
    template<typename T>
    inline bool Test(T val, T mask)   { return (val & mask) != 0; }
}

#define RVA(o) (Mem::Base() + (o))
