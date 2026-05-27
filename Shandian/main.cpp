#include "cheat/config.hpp"
#include "cheat/globals.hpp"
#include "cheat/combat.hpp"
#include "cheat/esp.hpp"
#include "menu/menu.hpp"
#include <pthread.h>
#include <unistd.h>
#include <atomic>

namespace Shandian {
    std::atomic<bool> g_Running{true};
    pthread_t g_ThreadTick;
    void(*OrigProcessEvent)(UObject*, void*, void*) = nullptr;
    using ITFn = void(*)(void*, u32, u32, FVector2D*, u64, u32);
    ITFn OrigInputTouch = nullptr;
    extern void* TickThread(void*);
}

//hooks.cpp
extern void HookedPostRender(UGameViewportClient*, UCanvas*);
extern void HookedInputTouch(void*, u32, u32, FVector2D*, u64, u32);
extern void HookedProcessEvent(UObject*, void*, void*);

/*init*/
__attribute__((constructor))
void ShandianInit() {
    using namespace Shandian;
    while (!Mem::Valid((void*)RVA(OFF::GWorld)))
        usleep(500000);

    u64 engine = Mem::Read<u64>(RVA(OFF::GEngine));
    if (!Mem::Valid((void*)engine)) return;

    void* vp = *(void**)(engine + 0x618);
    if (!Mem::Valid(vp)) return;

    // render
    g_PostRenderHook = VMTHook<void(UGameViewportClient*, UCanvas*)>(
        HookedPostRender, OFF::VF_PostRender);
    g_PostRenderHook.Swap(vp);

    // PE
    OrigProcessEvent = (void(*)(UObject*, void*, void*))RVA(0x3658a14);
    g_PEHook.Install((void*)OrigProcessEvent,
                     (void*)HookedProcessEvent);

    // so we can press shit
    OrigInputTouch = (ITFn)RVA(0x4c3da90);
    g_TouchHook.Install((void*)OrigInputTouch,
                        (void*)HookedInputTouch);

    // init everything once
    CachedGlobals::Get();
    g_Ready = true;
    pthread_create(&g_ThreadTick, nullptr, TickThread, nullptr);
}
