#include "cheat/config.hpp"
#include "cheat/globals.hpp"
#include "cheat/esp.hpp"
#include "cheat/minimap.hpp"
#include "menu/menu.hpp"

//render UI
void HookedPostRender(UGameViewportClient* vp, UCanvas* canvas) {
    Shandian::g_PostRenderHook.InvokeOriginal(vp, canvas);

    //every frame
    Shandian::g_CanvasW = (float)canvas->SizeX;
    Shandian::g_CanvasH = (float)canvas->SizeY;

    //3finger toggle
    if (Shandian::g_TogglePending) {
        Shandian::g_MenuOpen     = !Shandian::g_MenuOpen;
        Shandian::g_TogglePending = false;
    }
    if (Shandian::g_MenuOpen &&
        Shandian::g_TouchActive &&
        !Shandian::g_TouchHandled) {
        Shandian::HandleTouch(Shandian::g_TouchX, Shandian::g_TouchY);
    }

    Shandian::DrawESP(canvas);
    Shandian::DrawMenu(canvas);
    Shandian::DrawMinimap(canvas);

    //enemy count on HUD top-center
    if (Shandian::g_Config.ESP_EnemyCount && Shandian::g_MenuOpen) {
        i32 n = 0;
        for (auto& t : Shandian::g_ESPTargets)
            if (t.IsPlayer) n++;
        char b[8];
        snprintf(b, 8, "%d", n);
        SDK::K2_DrawText(canvas, nullptr, b,
            {(float)canvas->SizeX * 0.5f - 10, 6.f},
            {0.9f, 0.6f, 0.1f, 1}, 1.4f);
    }

    //info bar (left-center): FPS / ping / time / lat / lon / players online
    {
        char b[128];

        // FPS
        float fps = 0;
        {
            auto* g = (float*)RVA(0x7781d1);
            if (Mem::Valid(g))
                fps = *g > 0.0001f ? 1.f / (*g) : 60.f;
        }

        // Lat / Lon
        float lat = 0, lon = 0;
        {
            auto* w = SDK::GetGWorld();
            if (w) {
                i32* o = (i32*)((u64)w + 0xB9C);
                lat = o[0] / 100.f;
                lon = o[1] / 100.f;
            }
        }

        // Ping
        float ping = 0;
        auto* pc = SDK::GetLocalController();
        if (pc) {
            void* conn = *(void**)((u64)pc + 0x5F0);
            if (Mem::Valid(conn))
                ping = (float)*(u8*)((u64)conn + 0x5D4);
        }

        i32 pl = SDK::GetNumPlayers();
        float tm = SDK::GetTime();

        snprintf(b, 128,
            "FPS %.0f  %dms  %.0fh  %.1f %.1f  %dp",
            fps, (i32)ping, tm, lat, lon, pl);

        SDK::K2_DrawText(canvas, nullptr, b,
            {8.f, (float)canvas->SizeY * 0.5f - 10},
            {0.85f, 0.8f, 0.75f, 1}, 0.65f);
    }
}

// ═══════════════════════════════════════════════════════
//  InputTouch inline hook
//  type: 0=begin, 1=end, 2=move. idx=finger index.
// ═══════════════════════════════════════════════════════
using ITFn = void(*)(void*, u32, u32, FVector2D*, u64, u32);

//main.cpp
namespace Shandian {
    extern ITFn OrigInputTouch;
}

void HookedInputTouch(void* pc, u32 h, u32 type,
                              FVector2D* pos, u64 time, u32 idx) {
    float tx = pos->X;
    float ty = pos->Y;

    //1920×1080 reference space
    if (Shandian::g_CanvasW > 0) {
        tx *= 1920.f / Shandian::g_CanvasW;
        ty *= 1080.f / Shandian::g_CanvasH;
    }

    if (idx < 5) {
        //begin touch
        if (type == 0) {
            Shandian::g_Finger[idx] = Shandian::TouchFinger{tx, ty, true};
            Shandian::g_TouchX      = tx;
            Shandian::g_TouchY      = ty;
            Shandian::g_TouchActive = true;
            Shandian::g_TouchHandled = false;

            //3 simultaneous touches
            i32 active = 0;
            for (i32 j = 0; j < 5; j++)
                if (Shandian::g_Finger[j].active) active++;
            if (active >= 3)
                Shandian::g_TogglePending = true;

            //drag if user presses & holds the title bar
            if (Shandian::g_MenuOpen) {
                float mx = Shandian::g_MenuX;
                float my = Shandian::g_MenuY;
                if (tx >= mx - 5 &&
                    tx <= mx + Shandian::g_MenuW + 5 &&
                    ty >= my - 5 &&
                    ty <= my + 28) {
                    Shandian::g_Dragging  = true;
                    Shandian::g_DragOffX = tx - mx;
                    Shandian::g_DragOffY = ty - my;
                }
            }
        }
        //enc
        else if (type == 1) {
            Shandian::g_Finger[idx].active = false;
            Shandian::g_Dragging  = false;
            Shandian::g_TouchActive = false;
        }
        //move
        else if (type == 2 && Shandian::g_Finger[idx].active) {
            Shandian::g_TouchX = tx;
            Shandian::g_TouchY = ty;
            if (Shandian::g_Dragging) {
                Shandian::g_MenuX = tx - Shandian::g_DragOffX;
                Shandian::g_MenuY = ty - Shandian::g_DragOffY;
            }
        }
    }

    if (Shandian::OrigInputTouch)
        Shandian::OrigInputTouch(pc, h, type, pos, time, idx);
}

//inline PE hook aswell, i thought it would be better to do it this way rather than vtable hook
namespace Shandian {
    extern void(*OrigProcessEvent)(UObject*, void*, void*);
}

void HookedProcessEvent(UObject* obj, void* fn, void* params) {
    if (!Mem::Valid(fn) || !Mem::Valid(obj)) {
        if (Shandian::OrigProcessEvent)
            Shandian::OrigProcessEvent(obj, fn, params);
        return;
    }

    const char* fnName = ((UFunction*)fn)->NameCStr();

    //for silent aim and crash
    if (fnName && strstr(fnName, "ServerNotifyShot")) {
        struct ShotParams {
            TArray<float>                     ModelHeightChecks;
            TArray<FVector_NetQuantizeNormal> ShootDirs;
            TArray<FHitResult>                Impacts;
        };
        auto* p = (ShotParams*)params;

        if (p->Impacts.Count > 0) {

            //slow flyer
            if (Shandian::g_Config.SlowDinos_Enabled &&
                Shandian::g_SlowDinoTarget) {

                auto* dino = Shandian::g_SlowDinoTarget;
                if (dino && Mem::Valid(dino) &&
                    dino->RootComponent &&
                    !dino->IsPendingKill() &&
                    Shandian::OrigProcessEvent) {

                    auto* sc = (USceneComponent*)dino->RootComponent;
                    FVector body = sc->RelLocation;
                    auto* arr  = (TUObjectArray*)RVA(OFF::GUObjectArray);
                    auto* it   = arr->IndexToObject(dino->Index);

                    FHitResult savedHit  = p->Impacts.Data[0];
                    i32       savedCount = p->Impacts.Count;

                    p->Impacts.Data[0]._actor.ObjectIndex       = dino->Index;
                    p->Impacts.Data[0]._actor.ObjectSerialNumber =
                        it ? it->SerialNumber : 0;
                    p->Impacts.Data[0].Location    = body;
                    p->Impacts.Data[0].ImpactPoint = body;
                    if (p->ModelHeightChecks.Count > 0)
                        p->ModelHeightChecks.Data[0] = 0.f;

                    Shandian::OrigProcessEvent(obj, fn, params);

                    p->Impacts.Data[0] = savedHit;
                    p->Impacts.Count   = savedCount;

                    Shandian::g_SlowDinoTarget = nullptr;
                }
            }

            if (Shandian::g_Config.CrashEnemy_Enabled) {
                p->Impacts.Data[0]._actor.ObjectIndex       = 0;
                p->Impacts.Data[0]._actor.ObjectSerialNumber = 0xDEAD;
            } else if (Shandian::g_Config.SilentAim_Enabled) {
                auto* target = Shandian::g_SilentTarget;
                if (target && Mem::Valid(target) &&
                    target->RootComponent &&
                    !target->IsPendingKill()) {

                    auto* sc = (USceneComponent*)target->RootComponent;
                    FVector head = sc->RelLocation;
                    head.Z += 180.f;

                    auto* arr = (TUObjectArray*)RVA(OFF::GUObjectArray);
                    auto* it  = arr->IndexToObject(target->Index);

                    p->Impacts.Data[0]._actor.ObjectIndex       = target->Index;
                    p->Impacts.Data[0]._actor.ObjectSerialNumber =
                        it ? it->SerialNumber : 0;
                    p->Impacts.Data[0].Location    = head;
                    p->Impacts.Data[0].ImpactPoint = head;
                    p->Impacts.Data[0].Flags       = 1;
                }
            }
        }
    }

    //speed hack
    if (Shandian::g_Config.SpeedHack_Enabled &&
        fnName && strstr(fnName, "ReplicateMoveToServer")) {
        struct MoveParams {
            float   Delta;
            FVector Accel;
        };
        auto* p = (MoveParams*)params;
        p->Delta *= (1.f / (float)Shandian::g_Config.SpeedHack_Level);
    }

    //invoke our orig
    if (Shandian::OrigProcessEvent)
        Shandian::OrigProcessEvent(obj, fn, params);
}
