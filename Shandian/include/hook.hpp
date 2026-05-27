#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>

using u8  = uint8_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i32 = int32_t;

struct InlineHook
{
    void* m_target  = nullptr;
    void* m_hook    = nullptr;
    void* m_tramp   = nullptr;

    u8    m_orig[32]{};
    i32   m_patchSize = 16;

    bool  m_installed = false;

    //
    // ARM64 absolute jump:
    //
    // LDR X17, #8
    // BR  X17
    // <absolute address>
    //
    static inline void EmitAbsoluteJump(void* address, void* destination)
    {
        u32* out = (u32*)address;

        // LDR X17, #8
        out[0] = 0x58000051;

        // BR X17
        out[1] = 0xD61F0220;

        // absolute destination
        *(u64*)&out[2] = (u64)destination;
    }

    static inline bool IsB(u32 insn)
    {
        return ((insn & 0x7C000000) == 0x14000000);
    }

    static inline bool IsBL(u32 insn)
    {
        return ((insn & 0x7C000000) == 0x94000000);
    }

    static inline bool IsBCond(u32 insn)
    {
        return ((insn & 0xFF000010) == 0x54000000);
    }

    static inline bool IsCBZ(u32 insn)
    {
        return ((insn & 0x7E000000) == 0x34000000);
    }

    static inline bool IsCBNZ(u32 insn)
    {
        return ((insn & 0x7E000000) == 0x35000000);
    }

    static inline bool IsTBZ(u32 insn)
    {
        return ((insn & 0x7E000000) == 0x36000000);
    }

    static inline bool IsTBNZ(u32 insn)
    {
        return ((insn & 0x7E000000) == 0x37000000);
    }

    static inline bool IsADR(u32 insn)
    {
        return ((insn & 0x9F000000) == 0x10000000);
    }

    static inline bool IsADRP(u32 insn)
    {
        return ((insn & 0x9F000000) == 0x90000000);
    }

    static inline bool IsLiteralLDR(u32 insn)
    {
        return ((insn & 0x3B000000) == 0x18000000);
    }

    static inline bool IsPCRelative(u32 insn)
    {
        return
            IsB(insn)          ||
            IsBL(insn)         ||
            IsBCond(insn)      ||
            IsCBZ(insn)        ||
            IsCBNZ(insn)       ||
            IsTBZ(insn)        ||
            IsTBNZ(insn)       ||
            IsADR(insn)        ||
            IsADRP(insn)       ||
            IsLiteralLDR(insn);
    }

    static bool Protect(void* addr, size_t size, int prot)
    {
        size_t pageSize = sysconf(_SC_PAGESIZE);

        uintptr_t start =
            (uintptr_t)addr & ~(pageSize - 1);

        uintptr_t end =
            ((uintptr_t)addr + size + pageSize - 1)
            & ~(pageSize - 1);

        return mprotect(
            (void*)start,
            end - start,
            prot
        ) == 0;
    }

//refresh
    static inline void FlushCache(void* addr, size_t size)
    {
        __builtin___clear_cache(
            (char*)addr,
            (char*)addr + size
        );
    }
    static void* AllocTrampoline(size_t size)
    {
        void* mem = mmap(
            nullptr,
            size,
            PROT_READ | PROT_WRITE | PROT_EXEC,
            MAP_PRIVATE | MAP_ANONYMOUS,
            -1,
            0
        );

        if (mem == MAP_FAILED)
            return nullptr;

        return mem;
    }
    bool Install(void* target, void* hook)
    {
        if (m_installed)
            return false;

        if (!target || !hook)
            return false;

        m_target = target;
        m_hook   = hook;
        for (i32 i = 0; i < m_patchSize; i += 4)
        {
            u32 insn =
                *(u32*)((u8*)target + i);
            if (IsPCRelative(insn))
            {
                return false;
            }
        }
//orig
        memcpy(
            m_orig,
            target,
            m_patchSize
        );
        constexpr size_t trampSize = 64;
        m_tramp = AllocTrampoline(trampSize);

        if (!m_tramp)
            return false;

        memcpy(
            m_tramp,
            target,
            m_patchSize
        );
        void* returnAddr =
            (void*)((u8*)target + m_patchSize);

        EmitAbsoluteJump(
            (u8*)m_tramp + m_patchSize,
            returnAddr
        );

        FlushCache(m_tramp, trampSize);
        if (!Protect(
            target,
            m_patchSize,
            PROT_READ | PROT_WRITE | PROT_EXEC))
        {
            munmap(m_tramp, trampSize);
            m_tramp = nullptr;
            return false;
        }
        u8 patch[16]{};

        EmitAbsoluteJump(
            patch,
            hook
        );

//apply
        memcpy(
            target,
            patch,
            sizeof(patch)
        );

        FlushCache(target, sizeof(patch));
        Protect(
            target,
            m_patchSize,
            PROT_READ | PROT_EXEC
        );

        m_installed = true;
        return true;
    }
    bool Remove()
    {
        if (!m_installed)
            return false;

        if (!Protect(
            m_target,
            m_patchSize,
            PROT_READ | PROT_WRITE | PROT_EXEC))
        {
            return false;
        }

        memcpy(
            m_target,
            m_orig,
            m_patchSize
        );

        FlushCache(
            m_target,
            m_patchSize
        );

        Protect(
            m_target,
            m_patchSize,
            PROT_READ | PROT_EXEC
        );

        if (m_tramp)
        {
            munmap(m_tramp, 64);
            m_tramp = nullptr;
        }

        m_installed = false;
        return true;
    }
//get orig callable
    template<typename T>
    T Original() const
    {
        return (T)m_tramp;
    }
};