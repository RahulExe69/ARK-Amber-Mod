#include "combat.hpp"
#include "config.hpp"
#include "globals.hpp"

namespace Shandian {

void CombatTick() {
    auto& G = CachedGlobals::Get();

    //no recoil
    if (g_Config.NoRecoil_Enabled && G.LocalPawn) {
        TArray<APrimalBuff*> buffs;
        SDK::GetBuffs((APrimalCharacter*)G.LocalPawn, &buffs);
        if (buffs.Count > 0 && buffs.Data) {
            for (i32 i = 0; i < buffs.Count; i++) {
                if (!Mem::Valid(buffs.Data[i])) continue;
                *(float*)((u64)buffs.Data[i] + OFF::ABF_RecoilMult) = 0.f;
            }
        }
    }

    //no gun shake
    if (g_Config.NoGunShake_Enabled) {
        auto* cm = SDK::GetCameraManager();
        if (cm) {
            auto fn = (void(*)(void*, bool))RVA(0x4c36f6c);
            if (fn) fn(cm, true);
        }
    }

    //fov & wide view
    if (g_Config.FOV_Enabled)
        SDK::PeripheralVision(g_Config.FOV_Value);
    if (g_Config.Peripheral_Enabled)
        SDK::PeripheralVision(g_Config.Peripheral_Value);
    if (g_Config.NoScopeZoom_Enabled) {
        SDK::UnlockFOV();
        SDK::SetFOV(g_Config.FOV_Value);
    }

    //shoot while riding any dino + underwater shooting
    if (G.LocalWeapon) {
        u64* flags = (u64*)((u64)G.LocalWeapon + OFF::ASW_FireFlags);
        u64 clear = 0;
        if (g_Config.UnderwaterShoot_Enabled) clear |= OFF::FF_BlockSwimming;
        if (g_Config.RunWhileFire_Enabled)    clear |= OFF::FF_BlockMounted;
        if (clear) Bits::Clear(*flags, clear);
    }

    //run while fire just increases all speeds
    if (g_Config.RunWhileFire_Enabled && G.Movement) {
        G.Movement->MaxWalkSpeed = 2000.f;
        G.Movement->MaxFlySpeed  = 2000.f;
        G.Movement->MaxSwimSpeed = 2000.f;
    }

    //anti strafe on dino
    if (g_Config.NoStrafe_Enabled && G.LocalPawn) {
        FVector& input = ((APawn*)G.LocalPawn)->ControlInput;
        float mag = sqrtf(input.X * input.X + input.Y * input.Y + input.Z * input.Z);
        if (mag > 0.1f) {
            input.X = mag;
            input.Y = 0;
            input.Z = 0;
        }
    }

    //local fly mode
    if (g_Config.Fly_Enabled && G.LocalPawn) {
        CallRPCFast(G.LocalPawn, G.FN_ClientCheatFly, "ClientCheatFly", nullptr, 0);
        if (G.Movement) {
            G.Movement->MoveMode     = 5;
            G.Movement->GravityScale = 0.f;
        }
    }

    //ghost
    static bool wasGhost = false;
    if (g_Config.GhostMode_Enabled && G.LocalPawn) {
        CallRPCFast(G.LocalPawn, G.FN_ClientCheatGhost, "ClientCheatGhost", nullptr, 0);
    }
    if (!g_Config.GhostMode_Enabled && wasGhost && G.LocalPawn) {
        CallRPCFast(G.LocalPawn, G.FN_ClientCheatWalk, "ClientCheatWalk", nullptr, 0);
    }
    wasGhost = g_Config.GhostMode_Enabled;

    // ── Fake Admin ──
    if (g_Config.FakeAdmin_Enabled && G.LocalPC) {
        struct {
            bool bAdmin;
            bool bShowUI;
        } p = {true, false};
        CallRPCFast(G.LocalPC, G.FN_ClientNotifyAdmin,
                    "ClientNotifyAdmin", &p, sizeof(p));
    }

    //auto fire
    if (g_Config.AutoFire_Enabled && G.LocalWeapon) {
        auto fn = (void(*)(void*, bool))G.LocalWeapon->VTable[OFF::VF_StartFire];
        if (fn) fn(G.LocalWeapon, false);
    }

    //auto reload on 0 ammo
    if (g_Config.AutoReload_Enabled && G.LocalWeapon) {
        i32 ammo = *(i32*)((u64)G.LocalWeapon + OFF::ASW_AmmoCount);
        if (ammo <= 0) {
            auto fn = (void(*)(void*, bool))G.LocalWeapon->VTable[OFF::VF_StartFire];
            if (fn) fn(G.LocalWeapon, true);
        }
    }

    //tribe & ally chams
    if (g_Config.TribeChams_Enabled || g_Config.AllyChams_Enabled) {
        auto& actors = SDK::GetActors();
        i32 myTribe = SDK::GetLocalTribeID();
        for (i32 i = 0; i < actors.Count; i++) {
            auto* a = actors[i];
            if (!Mem::Valid(a) || !a->ClassPrivate) continue;
            if (!a->IsA("Character") || a->IsA("Dino")) continue;
            i32 tribe = SDK::GetTribeID(a);
            if (tribe <= 0) continue;
            void* mesh = *(void**)((u64)a + OFF::AC_Mesh);
            if (g_Config.TribeChams_Enabled && tribe == myTribe) {
                SDK::SetChams((UPrimitiveComponent*)mesh, true, g_Config.Cham_TribeCol);
            } else if (g_Config.AllyChams_Enabled && tribe == myTribe) {
                SDK::SetChams((UPrimitiveComponent*)mesh, true, g_Config.Cham_AllyCol);
            }
        }
    }

    //enemy and dino chams
    if (g_Config.EnemyChams_Enabled)
        SDK::EnableEnemyChams(true, g_Config.Cham_EnemyCol);
    if (g_Config.DinoChams_Enabled)
        SDK::EnableDinoChams(true, g_Config.Cham_DinoCol);

    //player wireframe
    if (g_Config.EnemyWireframe_Enabled) {
        auto& actors = SDK::GetActors();
        for (i32 i = 0; i < actors.Count; i++) {
            auto* a = actors[i];
            if (!SDK::IsEnemyPlayer(a)) continue;
            void* mesh = *(void**)((u64)a + OFF::AC_Mesh);
            if (!Mem::Valid(mesh)) continue;
            i32 numMats = *(i32*)((u64)mesh + 0x5F8);
            for (i32 j = 0; j < numMats && j < 10; j++) {
                void* mat = *(void**)((u64)mesh + 0x5E8 + j * 8);
                if (Mem::Valid(mat))
                    Bits::Set(*(u8*)((u64)mat + 0x828), (u8)1);
            }
        }
    }

    //wireframe sky
    if (g_Config.NoSkyWireframe) {
        auto* lvl = SDK::GetPersistentLevel();
        if (lvl) {
            void* lsa = *(void**)((u64)lvl + 0x100);
            if (Mem::Valid(lsa)) {
                void* sky = *(void**)((u64)lsa + 0x6E0);
                if (Mem::Valid(sky)) {
                    void* skyMat = *(void**)((u64)sky + 0x628);
                    if (Mem::Valid(skyMat))
                        Bits::Set(*(u8*)((u64)skyMat + 0x828), (u8)1);
                    void* waterMat = *(void**)((u64)sky + 0x728);
                    if (Mem::Valid(waterMat))
                        Bits::Set(*(u8*)((u64)waterMat + 0x828), (u8)1);
                }
            }
        }
    }

    //lighting
    if (g_Config.Gamma_Enabled) {
        u64 engine = Mem::Read<u64>(RVA(OFF::GEngine));
        if (Mem::Valid((void*)engine)) {
            void* gus = *(void**)(engine + 0x778);
            if (Mem::Valid(gus))
                *(float*)((u64)gus + 0x28) = g_Config.Gamma_Value;
        }
    }

    //fps
    if (g_Config.UnlockFPS_Enabled) {
        u64 engine = Mem::Read<u64>(RVA(OFF::GEngine));
        if (Mem::Valid((void*)engine))
            Bits::Set(*(u32*)(engine + 0x20A), 1u);
    }

    //chnage time of day
    if (g_Config.TimeOfDay_Enabled && G.LocalPC) {
        auto fn = (void(*)(void*, float))RVA(0x2c3fc0c);
        if (fn) fn(G.LocalPC, g_Config.TimeOfDay_Value * 3600.f);
    }

    //no black torpor stuff
    if (g_Config.NoKnockoutBlur) {
        auto* arr = (TUObjectArray*)RVA(OFF::GUObjectArray);
        if (Mem::Valid(arr)) {
            for (i32 i = 0; i < arr->Num(); i++) {
                auto* obj = arr->GetObjectPtr(i);
                if (!Mem::Valid(obj) || !obj->ClassPrivate) continue;
                if (obj->IsA("PrimalGameData")) {
                    *(u64*)((u64)obj + 0x1130) = 0;
                    break;
                }
            }
        }
    }

    //great for crashing the server
    if (g_Config.ServerCrash_Enabled) {
        static float lastCrash = 0;
        float now = SDK::GetTime();
        if (now - lastCrash > 10.f && G.LocalPC) {
            lastCrash = now;
            SDK::PlaceStructureParams p;
            memset(&p, 0, sizeof(p));
            p.ItemID    = 290;
            p.Location  = G.MyPos;
            p.ItemNetID.Id1 = 10293847;
            p.ItemNetID.Id2 = 10293847;
            p.bFromMultiUse  = true;
            SDK::PlaceStructure(p);
        }
    }
}

void AutoTick() {
    auto& G = CachedGlobals::Get();
    if (!G.LocalPawn || !G.Inventory) return;

    float hp   = SDK::GetHP(G.LocalPawn);
    float stam = SDK::GetStamina(G.LocalPawn);

    auto* itemData = *(TArray<void*>**)((u64)G.Inventory + 0x238);
    i32 itemCount  = *(i32*)((u64)G.Inventory + 0x240);
    if (!itemData || itemCount <= 0) return;

    auto* equipData = *(TArray<void*>**)((u64)G.Inventory + 0x228);
    i32 equipCount  = *(i32*)((u64)G.Inventory + 0x230);
    bool hasSlot[5] = {false};

    //check which armor slots r alr filled
    for (i32 i = 0; equipData && i < equipCount; i++) {
        auto* item = (void*)equipData->Data[i];
        if (!Mem::Valid(item)) continue;
        u8 slot = *(u8*)((u64)item + 314);
        if (slot < 5) hasSlot[slot] = true;
    }

    for (i32 i = 0; i < itemCount; i++) {
        auto* item = (void*)itemData->Data[i];
        if (!Mem::Valid(item)) continue;
        u8 slot = *(u8*)((u64)item + 314);

        // Auto Armor
        if (g_Config.AutoArmor_Enabled && slot < 5 && !hasSlot[slot]) {
            struct {
                SDK::FItemNetID Id;
                bool            b;
            } p;
            p.Id = *(SDK::FItemNetID*)((u64)item + 0x1E0);
            p.b  = false;
            CallRPCFast((UObject*)G.Inventory, G.FN_ServerEquipItem,
                        "ServerEquipItem", &p, sizeof(p));
            hasSlot[slot] = true;
            continue;
        }

        // Auto Brew / Stamina
        if ((g_Config.AutoBrew_Enabled    && hp   < 500.f) ||
            (g_Config.AutoStamina_Enabled && stam < 200.f)) {
            struct {
                SDK::FItemNetID a, b;
                i32             c;
            } p;
            p.a = p.b = *(SDK::FItemNetID*)((u64)item + 0x1E0);
            p.c = 0;
            CallRPCFast((UObject*)G.Inventory, G.FN_ServerUseItemWithItem,
                        "ServerUseItemWithItem", &p, sizeof(p));
        }
    }
}

void AutoFeedTick() {
    if (!g_Config.AutoFeed_Enabled) return;

    auto& G = CachedGlobals::Get();
    if (!G.LocalPawn || !G.LocalPC) return;

    auto& actors = SDK::GetActors();
    for (i32 i = 0; i < actors.Count; i++) {
        auto* a = actors[i];
        if (!Mem::Valid(a) || !a->ClassPrivate) continue;
        if (!a->IsA("DinoCharacter") || a->Owner != G.LocalPawn) continue;

        void* dinoInv = *(void**)((u64)a + 0x228);
        if (!Mem::Valid(dinoInv)) continue;

        auto* items = *(TArray<void*>**)((u64)dinoInv + 0x238);
        i32 ic = *(i32*)((u64)dinoInv + 0x240);
        if (!items || ic <= 0) continue;

        for (i32 j = 0; j < ic; j++) {
            auto* food = (void*)items->Data[j];
            if (!Mem::Valid(food)) continue;

            const char* cn = ((UObject*)food)->ClassName();
            if (!cn) continue;
            if (!strstr(cn, "Meat")   &&
                !strstr(cn, "Kibble") &&
                !strstr(cn, "Berry")  &&
                !strstr(cn, "Crop")) continue;

            struct {
                void*           Inv;
                SDK::FItemNetID Id;
            } p;
            p.Inv = dinoInv;
            p.Id  = *(SDK::FItemNetID*)((u64)food + 0x1E0);

            CallRPCFast(G.LocalPC, G.FN_ServerRequestInventoryUseItem,
                        "ServerRequestInventoryUseItem", &p, sizeof(p));
            break;
        }
    }
}

}
