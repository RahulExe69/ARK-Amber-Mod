#pragma once
#include "config.hpp"

namespace Shandian {

inline AActor* GetBestTarget(float fov, float maxDist, bool visibleOnly) {
    auto cam = SDK::GetCamInfo();
    if (!cam.Ok) return nullptr;

    AActor* best = nullptr;
    float bestA = fov;
    auto& actors = SDK::GetActors();

    for (i32 i = 0; i < actors.Count; i++) {
        auto* a = actors[i];
        if (!SDK::IsEnemyPlayer(a) || !a->RootComponent) continue;
        if (SDK::IsDead(a)) continue;

        auto* sc = (USceneComponent*)a->RootComponent;
        FVector hp = sc->RelLocation; hp.Z += 180.f;
        float dist = cam.Loc.DistTo(hp);
        if (dist > maxDist) continue;

        if (visibleOnly && !SDK::IsVisible(cam.Loc, hp)) continue;

        FVector dir = (hp - cam.Loc).Normalized();
        float ang = acosf(dir.Dot(cam.Rot.ToVector())) * 57.29578f;
        if (ang < bestA) { bestA = ang; best = a; }
    }
    return best;
}

inline AActor* GetClosestEnemy(float maxDist) {
    auto* lp = SDK::GetLocalPawn();
    if (!lp || !lp->RootComponent) return nullptr;
    FVector myPos = ((USceneComponent*)lp->RootComponent)->RelLocation;

    AActor* best = nullptr;
    float bestD = maxDist;
    auto& actors = SDK::GetActors();

    for (i32 i = 0; i < actors.Count; i++) {
        auto* a = actors[i];
        if (!SDK::IsEnemyPlayer(a) || !a->RootComponent) continue;
        if (SDK::IsDead(a)) continue;

        auto* sc = (USceneComponent*)a->RootComponent;
        float dist = myPos.DistTo(sc->RelLocation);
        if (dist < bestD) { bestD = dist; best = a; }
    }
    return best;
}

inline void AimbotTick() {
    if (!g_Config.Aimbot_Enabled) return;
    auto* best = GetBestTarget(g_Config.Aimbot_FOV, g_Config.Aimbot_MaxDist, g_Config.Aimbot_VisibleOnly);
    if (!best || !best->RootComponent) return;

    auto cam = SDK::GetCamInfo();
    if (!cam.Ok) return;
    auto* sc = (USceneComponent*)best->RootComponent;
    FVector hp = sc->RelLocation; hp.Z += 180.f;
    FRotator tgt = VecToRot(hp - cam.Loc);
    FRotator cur = cam.Rot;
    cur.Norm(); tgt.Norm();
    FRotator delta{tgt.Pitch - cur.Pitch, tgt.Yaw - cur.Yaw, 0};
    delta.Norm();
    float s = g_Config.Aimbot_Smooth;
    *(FRotator*)((u64)SDK::GetLocalController() + 0x3A0) =
        FRotator{cur.Pitch + delta.Pitch / s, cur.Yaw + delta.Yaw / s, 0};
}

}
