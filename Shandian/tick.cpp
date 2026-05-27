#include "cheat/config.hpp"
#include "cheat/globals.hpp"
#include "cheat/combat.hpp"
#include <unistd.h>
#include <atomic>

namespace Shandian {

extern std::atomic<bool> g_Running;
extern bool g_Ready;

void* TickThread(void*) {
    auto& G = CachedGlobals::Get();

    while (g_Running) {
        usleep(16000);

        if (!g_Ready)
            continue;

        if (!Mem::Valid((void*)RVA(OFF::GWorld)) ||
            !Mem::Valid(SDK::GetGWorld())) {
            usleep(100000);
            continue;
        }

        G.InitFrame();
        g_ESPTargets.clear();

        auto& actors     = SDK::GetActors();
        i32 actorCount   = actors.Count;
        static i32 destroyIdx = 0;

        //for aimbot and silent aim stuff
        AActor* bestAim    = nullptr;
        AActor* bestSilent = nullptr;
        AActor* bestMount  = nullptr;
        float bestAimA     = g_Config.Aimbot_FOV;
        float bestSilentD  = g_Config.SilentAim_Enabled ? 10000.f : 0.f;
        float bestMountD   = g_Config.AutoMount_Enabled ? 500.f : 0.f;

        //actor loop
        for (i32 i = 0; i < actorCount; i++) {
            auto* a = actors[i];
            if (!Mem::Valid(a) || !a->ClassPrivate) continue;
            if (a == G.LocalPawn) continue;

            bool isPlayer = IsClass(a, G.CI_Character) && !a->IsA(G.Class_DinoCharacter);
            bool isDino   = a->IsA(G.Class_DinoCharacter);
            bool isStruct = a->IsA(G.Class_Structure) ||
                           a->IsA(G.Class_Bed);
            bool isSupply = a->IsA(G.Class_SupplyCrate) ||
                           a->IsA(G.Class_Beacon);
            bool isTurret = a->IsA(G.Class_Turret);
            bool isCache  = a->IsA(G.Class_Cache) ||
                           a->IsA(G.Class_Storage);

            if (a->RootComponent) {
                FVector pos  = ((USceneComponent*)a->RootComponent)->RelLocation;
                float dist   = G.Cam.Loc.DistTo(pos);

                if (dist < g_Config.ESP_MaxDist) {
                    bool isSleep = isPlayer &&
                        Bits::Test(*(u64*)((u64)a + OFF::AA_HiddenFlags), (u64)1);
                    bool isDead = SDK::IsDead(a);

                    bool showESP =
                        (isPlayer && g_Config.ESP_Players)       ||
                        (isDino   && g_Config.ESP_Dinos)         ||
                        (isStruct && g_Config.ESP_Structures)    ||
                        (isSupply && g_Config.ESP_SupplyCrates);

                    if (showESP) {
                        FVector2D sc = {0, 0};
                        FVector2D hd = {0, 0};
                        bool onScr   = SDK::WorldToScreen(pos, &sc);

                        FVector hp = pos;
                        hp.Z += isDino ? 300.f : 180.f;
                        SDK::WorldToScreen(hp, &hd);

                        // if not on screen/off screen -> edge clamp
                        if (!onScr) {
                            FVector dir = (pos - G.Cam.Loc).Normalized();
                            float fwd   = dir.Dot(G.Cam.Rot.ToVector());

                            if (fwd < -0.5f) {
                                sc     = {-999, -999};
                                onScr = false;
                            } else {
                                FVector r = FRotator(0, G.Cam.Rot.Yaw + 90, 0)
                                                .ToVector();
                                FVector u = G.Cam.Rot.ToVector().Cross(r);
                                float hrz = dir.Dot(r);
                                float vrt = dir.Dot(u);
                                float s   = fminf(
                                    960.f / fabsf(fmaxf(fabsf(hrz), 0.001f)) *
                                        fabsf(fwd),
                                    540.f / fabsf(fmaxf(fabsf(vrt), 0.001f)) *
                                        fabsf(fwd));
                                sc.X   = 960.f + hrz * s;
                                sc.Y   = 540.f - vrt * s;
                                onScr = true;
                            }
                        }

                        // health + level
                        float hpVal = (isDino || isPlayer)
                            ? *(float*)((u64)*(void**)((u64)a +
                                OFF::APC_StatusComponent) +
                                OFF::APC_StatusComp_StatusValues)
                            : 0;
                        i32 level = 0;
                        if (isDino || isPlayer) {
                            auto* sp = (void*)(*(u64*)((u64)a +
                                OFF::APC_StatusComponent));
                            if (Mem::Valid(sp))
                                level = *(i32*)((u64)sp +
                                    OFF::APC_StatusComp_ExpPoints) +
                                    *(i32*)((u64)sp +
                                    OFF::APC_StatusComp_ExtraLevel);
                        }

                        g_ESPTargets.push_back({
                            a, sc, hd, dist, hpVal, level,
                            isPlayer, isDino, isStruct, isSupply, onScr
                        });
                    }
                }
            }

            // aimbot & magic/silent
            if (isPlayer && a->RootComponent &&
                SDK::IsEnemyPlayer(a) && !SDK::IsDead(a)) {

                FVector hp = ((USceneComponent*)a->RootComponent)->RelLocation;
                hp.Z += 180.f;
                float d = G.Cam.Loc.DistTo(hp);

                if (g_Config.Aimbot_Enabled && d < g_Config.Aimbot_MaxDist) {
                    if (!g_Config.Aimbot_VisibleOnly ||
                        SDK::IsVisible(G.Cam.Loc, hp)) {
                        FVector dir = (hp - G.Cam.Loc).Normalized();
                        float ang = acosf(
                            dir.Dot(G.Cam.Rot.ToVector())) * 57.29578f;// rad degree bs
                        if (ang < bestAimA) {
                            bestAimA = ang;
                            bestAim  = a;
                        }
                    }
                }

                if (g_Config.SilentAim_Enabled && d < bestSilentD) {
                    bestSilentD = d;
                    bestSilent = a;
                }
            }

            // destroy claim and force pick
            if (g_Config.DestroyStructs_Enabled ||
                g_Config.ClaimDinos_Enabled     ||
                g_Config.ForceObtain_Enabled) {

                if (a->RootComponent) {
                    bool onScr = true;
                    if (g_Config.DestroyOnlyOnScreen) {
                        FVector2D sc;
                        onScr = SDK::WorldToScreen(
                            ((USceneComponent*)a->RootComponent)->RelLocation,
                            &sc);
                    }

                    if (onScr && destroyIdx <= i) {
                        if (g_Config.DestroyStructs_Enabled && isStruct) {
                            SDK::ServerMultiUseParams mu = {
                                a, SDK::MULTI_DESTROY,
                                *(i32*)((u64)G.LocalPC + 0xC28),
                                true, true
                            };
                            CallRPCFast(G.LocalPC, G.FN_ServerMultiUse,
                                "ServerMultiUse", &mu, sizeof(mu));
                            destroyIdx = i + 1;
                        } else if (g_Config.ClaimDinos_Enabled && isDino) {
                            SDK::ServerMultiUseParams mu = {
                                a, SDK::MULTI_FORCE_OBTAIN,
                                *(i32*)((u64)G.LocalPC + 0xC28),
                                true, true
                            };
                            CallRPCFast(G.LocalPC, G.FN_ServerMultiUse,
                                "ServerMultiUse", &mu, sizeof(mu));
                            destroyIdx = i + 1;
                        } else if (g_Config.ForceObtain_Enabled) {
                            SDK::ServerMultiUseParams mu = {
                                a, SDK::MULTI_FORCE_OBTAIN,
                                *(i32*)((u64)G.LocalPC + 0xC28),
                                true, true
                            };
                            CallRPCFast(G.LocalPC, G.FN_ServerMultiUse,
                                "ServerMultiUse", &mu, sizeof(mu));
                            destroyIdx = i + 1;
                        }
                    }
                }
            }

            // turrets
            if (g_Config.TurretFill_Enabled &&
                isTurret && G.LocalPC && G.Inventory) {

                i32 tt = SDK::GetTribeID(a);
                if (tt > 0 && tt == G.LocalTribe) {
                    FVector tp = ((USceneComponent*)a->RootComponent)
                                     ->RelLocation;
                    if (G.MyPos.DistTo(tp) < 3000.f) {
                        i32 ca = *(i32*)((u64)a + 0x1004);
                        if (ca < g_Config.TurretFill_Amount) {
                            bool isPlant = G.CI_PlantSpecies &&
                                IsClass(a, G.CI_PlantSpecies);
                            const char* na = isPlant
                                ? nullptr
                                : "AdvancedRifleBullet";

                            if (na) {
                                auto* ii = *(TArray<void*>**)(
                                    (u64)G.Inventory + 0x238);
                                i32 ic = *(i32*)((u64)G.Inventory + 0x240);

                                if (ii && ic > 0) {
                                    for (i32 k = 0; k < ic; k++) {
                                        auto* it = (void*)ii->Data[k];
                                        if (!Mem::Valid(it)) continue;

                                        const char* c =
                                            ((UObject*)it)->ClassName();
                                        if (!c || !strstr(c, na)) continue;

                                        SDK::FItemNetID nid =
                                            *(SDK::FItemNetID*)((u64)it +
                                                                 0x1E0);
                                        struct {
                                            void*           Inv;
                                            SDK::FItemNetID Id;
                                            bool            Equip;
                                            i32             Qty;
                                            bool            b1, b2, b3;
                                        } p;
                                        p.Inv   = *(void**)((u64)a + 0xBE8);
                                        p.Id    = nid;
                                        p.Equip = false;
                                        p.Qty   = g_Config.TurretFill_Amount -
                                                  ca;
                                        p.b1 = p.b2 = p.b3 = false;
                                        CallRPCFast(G.LocalPC,
                                            G.FN_ServerTransferToRemote,
                                            "ServerTransferToRemoteInventory",
                                            &p, sizeof(p));
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (g_Config.TurretPin_Enabled && isTurret) {
                i32* pin = (i32*)((u64)a + 0xD00);
                if (Mem::Valid(pin))
                    *pin = g_Config.TurretPin_Value;
            }
            if (g_Config.TurretActivate_Enabled && isTurret) {
                u8* flag = (u8*)((u64)a + 0xC58);
                if (Mem::Valid(flag))
                    Bits::Set(*flag, (u8)2);
            }

            if (g_Config.TurretActivate_Enabled && isTurret) {
                u8* act = (u8*)((u64)a + 0x1B48);
                if (Mem::Valid(act))
                    *act = 1;
            }

            // auto ride
            if (g_Config.AutoMount_Enabled &&
                isDino && a->RootComponent &&
                a->Owner == G.LocalPawn) {

                float d = G.MyPos.DistTo(
                    ((USceneComponent*)a->RootComponent)->RelLocation);
                if (d < bestMountD) {
                    bestMountD = d;
                    bestMount  = a;
                }
            }

            // auto loot
            if ((g_Config.AutoLoot_Enabled ||
                 g_Config.AutoPickup_Enabled) &&
                isCache && a->RootComponent && G.LocalPC) {

                float d = G.MyPos.DistTo(
                    ((USceneComponent*)a->RootComponent)->RelLocation);
                if (d < 500.f) {
                    struct {
                        void*           Inv;
                        SDK::FItemNetID Id;
                        i32             Count;
                        i32             Extra;
                        bool            bFlag1, bFlag2;
                        u8              Option;
                        bool            bFlag3, bFlag4;
                    } p;
                    memset(&p, 0, sizeof(p));
                    p.Inv = a;
                    CallRPCFast(G.LocalPC, G.FN_ServerTransferFromRemote,
                        "ServerTransferFromRemoteInventory",
                        &p, sizeof(p));
                }
            }
        }

        //esp for resources
        g_ResourceTargets.clear();
        if (g_Config.ESP_ResourceMetal   ||
            g_Config.ESP_ResourceObsidian ||
            g_Config.ESP_ResourceCrystal  ||
            g_Config.ESP_ResourceOil      ||
            g_Config.ESP_ResourcePearls   ||
            g_Config.ESP_ResourceStone) {//i think stone was an actor? not sure

            auto* ifa = SDK::GetInstancedFoliageActor();
            if (ifa && Mem::Valid(ifa)) {
                auto& fa = SDK::GetActors();
                for (i32 i = 0;
                     i < fa.Count && g_ResourceTargets.size() < 500;
                     i++) {

                    auto* f = fa[i];
                    if (!Mem::Valid(f) || f->OuterPrivate != ifa) continue;
                    if (!IsClass(f, G.CI_InstancedMesh)) continue;

                    auto* ismc = (UInstancedStaticMeshComponent*)f;
                    i32 n = ismc->InstanceCount();
                    if (n <= 0 || n > 500000) continue;

                    const char* cn = f->ClassPrivate
                        ? ((UObject*)f->ClassPrivate)->NameCStr()
                        : nullptr;
                    if (!cn) continue;

                    const char* label = nullptr;
                    FLinearColor rc = {1, 1, 1, 1};

                    if (strstr(cn, "Metal") &&
                        g_Config.ESP_ResourceMetal) {
                        label = "Metal";
                        rc    = {0.7f, 0.7f, 0.7f, 1};
                    } else if (strstr(cn, "Obsidian") &&
                               g_Config.ESP_ResourceObsidian) {
                        label = "Obs";
                        rc    = {0.2f, 0.1f, 0.5f, 1};
                    } else if (strstr(cn, "Crystal") &&
                               g_Config.ESP_ResourceCrystal) {
                        label = "Crys";
                        rc    = {0.4f, 0.8f, 1, 1};
                    } else if (strstr(cn, "Oil") &&
                               g_Config.ESP_ResourceOil) {
                        label = "Oil";
                        rc    = {0.1f, 0.1f, 0.1f, 1};
                    } else if (strstr(cn, "Silicon") &&
                               g_Config.ESP_ResourcePearls) {
                        label = "Pearl";
                        rc    = {0.9f, 0.9f, 0.9f, 1};
                    } else if (strstr(cn, "Stone") &&
                               g_Config.ESP_ResourceStone) {
                        label = "Stone";
                        rc    = {0.5f, 0.4f, 0.3f, 1};
                    } else {
                        continue;
                    }

                    for (i32 j = 0;
                         j < n && g_ResourceTargets.size() < 500;
                         j += 4) {
                        FTransform t = ismc->GetInstanceWorldTransform(j);
                        FVector2D sc;
                        if (SDK::WorldToScreen(t.Trans, &sc))
                            g_ResourceTargets.push_back({sc, label, rc});
                    }
                }
            }
        }

        // apply aimbot
        if (bestAim && bestAim->RootComponent) {
            FVector hp = ((USceneComponent*)bestAim->RootComponent)
                             ->RelLocation;
            hp.Z += 180.f;

            FRotator tgt = VecToRot(hp - G.Cam.Loc);
            FRotator cur = G.Cam.Rot;
            cur.Norm();
            tgt.Norm();

            FRotator delta = {
                tgt.Pitch - cur.Pitch,
                tgt.Yaw   - cur.Yaw,
                0
            };
            delta.Norm();

            float s = g_Config.Aimbot_Smooth;
            *(FRotator*)((u64)G.LocalPC + 0x3A0) = {
                cur.Pitch + delta.Pitch / s,
                cur.Yaw   + delta.Yaw   / s,
                0
            };
        }

        g_SilentTarget = bestSilent;

        // ── Slow Flyers ──
        g_SlowDinoTarget = nullptr;
        if (g_Config.SlowDinos_Enabled && bestAim) {
            i32 dinoIdx = *(i32*)((u64)bestAim + 0xF40);
            if (dinoIdx > 0) {
                auto* arr = (TUObjectArray*)RVA(OFF::GUObjectArray);
                auto* it  = arr->IndexToObject(dinoIdx);
                if (it && it->Object) {
                    auto* dino = (AActor*)it->Object;
                    if (dino && Mem::Valid(dino) &&
                        dino->ClassPrivate && dino->RootComponent) {
                        const char* cn =
                            ((UObject*)dino->ClassPrivate)->NameCStr();
                        if (cn &&
                            (strstr(cn, "Ptero")   ||
                             strstr(cn, "Argent")  ||
                             strstr(cn, "Quetz")   ||
                             strstr(cn, "Tapejara")||
                             strstr(cn, "Pela")    ||
                             strstr(cn, "Moth")    ||
                             strstr(cn, "Griffin") ||
                             strstr(cn, "Dragon")  ||
                             strstr(cn, "Owl"))) {
                            g_SlowDinoTarget = dino;
                        }
                    }
                }
            }
        }

        // do auto mount
        if (bestMount && G.LocalPawn) {
            auto fn = (bool(*)(void*, void*))RVA(0x29b3fb0);
            if (fn)
                fn(G.LocalPawn, bestMount);
        }

        //add all to thread
        CombatTick();
        AutoTick();
        AutoFeedTick();

        if (destroyIdx >= actorCount || destroyIdx < 0)
            destroyIdx = 0;
    }

    return nullptr;
}

}
