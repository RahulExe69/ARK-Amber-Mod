#pragma once
#include "ArkSDK.hpp"
#include "hook.hpp"
#include "VMTHook.hpp"
#include <vector>
#include <cmath>

namespace Shandian {

inline bool g_Ready = false;
inline float g_DeltaTime = 0.016f;
struct TouchFinger { float x,y; bool active; };
inline TouchFinger g_Finger[5] = {};
inline AActor* g_SilentTarget = nullptr;
inline AActor* g_SlowDinoTarget = nullptr;

inline float g_TouchX=0,g_TouchY=0,g_DragOffX=0,g_DragOffY=0,g_CanvasW=1920,g_CanvasH=1080;
inline bool  g_TouchActive=false,g_TouchHandled=false,g_Dragging=false,g_TogglePending=false;

struct Config {
    // === General ===
    bool SpeedHack_Enabled  = false;
    i32  SpeedHack_Level    = 2;
    bool FOV_Enabled        = false;
    float FOV_Value         = 110.f;
    bool Fly_Enabled        = false;
    bool GhostMode_Enabled  = false;
    bool TimeOfDay_Enabled  = false;
    float TimeOfDay_Value   = 12.f;
    bool NoSkyWireframe     = false;
    bool NoKnockoutBlur     = false;
    bool UnlockFPS_Enabled  = false;
    float Gamma_Value       = 2.2f;
    bool Gamma_Enabled      = false;

    // === ESP: Players ===
    bool ESP_Players        = true;
    bool ESP_Sleeping       = false;
    bool ESP_DeadPlayers    = true;
    bool ESP_PlayerLines    = true;
    bool ESP_PlayerNames    = false;
    bool ESP_HPBar          = true;
    bool ESP_Level          = true;
    bool ESP_Bones          = false;

    // === ESP: Dinosaurs ===
    bool ESP_Dinos          = true;
    bool ESP_DinoHP         = true;
    bool ESP_DinoLines      = true;
    bool ESP_Distance       = true;

    // === ESP: Structures ===
    bool ESP_Structures     = false;
    bool ESP_Storage        = false;
    bool ESP_SupplyCrates   = true;
    bool ESP_Beds           = false;
    bool ESP_DroppedItems   = true;

    // === ESP: Resources ===
    bool ESP_ResourceMetal    = false;
    bool ESP_ResourceObsidian = false;
    bool ESP_ResourceCrystal  = false;
    bool ESP_ResourceOil      = false;
    bool ESP_ResourcePearls   = false;
    bool ESP_ResourceStone    = false;
    float ESP_MaxDist       = 10000.f;

    // === PvP ===
    bool Aimbot_Enabled     = false;
    float Aimbot_FOV        = 90.f;
    float Aimbot_Smooth     = 5.f;
    bool Aimbot_VisibleOnly = true;
    float Aimbot_MaxDist    = 10000.f;
    bool SilentAim_Enabled  = false;
    bool CrashEnemy_Enabled = false;
    bool SlowDinos_Enabled  = false;
    bool FakeAdmin_Enabled  = false;
    bool AutoFire_Enabled   = false;
    bool AutoReload_Enabled = false;
    bool NoRecoil_Enabled   = false;
    bool NoGunShake_Enabled = false;
    bool NoScopeZoom_Enabled= false;
    bool RunWhileFire_Enabled = false;
    bool UnderwaterShoot_Enabled = false;
    bool NoStrafe_Enabled   = false;
    bool EnemyWireframe_Enabled = false;
    bool TribeChams_Enabled = false;
    bool AllyChams_Enabled  = false;
    bool EnemyChams_Enabled = false;
    bool DinoChams_Enabled  = false;

    // === Misc ===
    bool UnlockNotes_Enabled = false;
    bool AutoArmor_Enabled   = false;
    bool AutoBrew_Enabled    = false;
    bool AutoStamina_Enabled = false;
    bool AutoMount_Enabled   = false;
    bool AutoLoot_Enabled    = false;
    bool AutoPickup_Enabled  = false;
    bool AutoFeed_Enabled    = false;
    bool TurretFill_Enabled  = false;
    i32  TurretFill_Amount   = 100;
    bool TurretPin_Enabled   = false;
    i32  TurretPin_Value     = 1234;
    bool TurretActivate_Enabled = false;
    bool ServerCrash_Enabled   = false;
    bool ESP_ArmorWeapons    = false;
    bool ESP_EnemyCount      = true;
    bool SendMsg_Open        = false;

    // minimap
    bool Minimap_Enabled    = false;
    bool Minimap_Players    = true;
    bool Minimap_Dinos      = true;
    bool Minimap_Turrets    = true;
    bool Peripheral_Enabled  = false;
    float Peripheral_Value   = 120.f;
    bool DestroyStructs_Enabled  = false;
    bool ClaimDinos_Enabled      = false;
    bool ForceObtain_Enabled     = false;
    bool DestroyOnlyOnScreen     = true;
    i32  DestroyRatePerTick      = 5;

    // === Colors ===
    FLinearColor ESP_TribeCol    = {0,1,0,1};
    FLinearColor ESP_EnemyCol    = {1,0,0,1};
    FLinearColor ESP_DinoCol     = {0,0.8f,1,1};
    FLinearColor ESP_StructureCol= {1,1,0,1};
    FLinearColor ESP_SupplyCol   = {0.8f,0.4f,1,1};
    FLinearColor Cham_EnemyCol   = {1,0,0,1};
    FLinearColor Cham_AllyCol    = {0,1,0,1};
    FLinearColor Cham_TribeCol   = {0,0.6f,1,1};
    FLinearColor Cham_DinoCol    = {0,1,0,1};
    FLinearColor Menu_AccentCol  = {0.9f,0.6f,0.1f,1};
    FLinearColor Menu_BgCol      = {0.04f,0.03f,0.02f,0.92f};
};
inline Config g_Config;

// global hooks
inline VMTHook<void(UGameViewportClient*,UCanvas*)> g_PostRenderHook(nullptr, OFF::VF_PostRender);
inline VMTHook<void(UCharacterMovementComponent*,float,const FVector&)> g_ReplicateMoveHook(nullptr, OFF::VF_ReplicateMoveToServer);
extern void(*OrigProcessEvent)(UObject*,void*,void*);
inline InlineHook g_PEHook;
inline InlineHook g_TouchHook;

// esps cache
struct ESPTarget {
    AActor* Actor; FVector2D ScreenPos, HeadScreen;
    float Dist, HP; i32 Level;
    bool IsPlayer, IsDino, IsStructure, IsSupply, OnScreen;
};
inline std::vector<ESPTarget> g_ESPTargets;
struct ResourceTarget { FVector2D Screen; const char* Label; FLinearColor Color; };
inline std::vector<ResourceTarget> g_ResourceTargets;

}
