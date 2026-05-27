#pragma once
#include "config.hpp"

namespace Shandian {

struct CachedGlobals {
    // pre-computed per frame
    FVector  MyPos;
    SDK::CamInfo  Cam;
    AActor*  LocalPawn;
    APlayerController* LocalPC;
    AShooterWeapon* LocalWeapon;
    UCharacterMovementComponent* Movement;
    void*    Inventory;
    i32      LocalTribe;

    // cached UFunction pointers (set on first CallRPC)
    UFunction* FN_ServerEquipItem;
    UFunction* FN_ServerUseItemWithItem;
    UFunction* FN_ServerTransferToRemote;
    UFunction* FN_ServerTransferFromRemote;
    UFunction* FN_ServerRequestInventoryUseItem;
    UFunction* FN_ServerMultiUse;
    UFunction* FN_ServerSuicide;
    UFunction* FN_ServerKillDino;
    UFunction* FN_ClientCheatGhost;
    UFunction* FN_ClientCheatWalk;
    UFunction* FN_ClientCheatFly;
    UFunction* FN_ClientNotifyAdmin;
    // FName CI for class checks (faster than strstr)
    u32 CI_Meat;
    u32 CI_Kibble;
    u32 CI_Berry;
    u32 CI_Crop;

    // FName CI for entity type checks; discovered via strstr at init
    u32 CI_Character;       // player character parent class
    u32 CI_InstancedMesh;   // "InstancedStaticMesh"
    u32 CI_PlantSpecies;    // "PlantSpecies" (for turret fill exclusion)

    // cached UClass pointers for IsA (pointer comparison, fast + exact)
    UClass* Class_DinoCharacter;
    UClass* Class_Structure;
    UClass* Class_SupplyCrate;
    UClass* Class_Beacon;
    UClass* Class_Bed;
    UClass* Class_Dropped;
    UClass* Class_Turret;
    UClass* Class_Cache;
    UClass* Class_Storage;

    // strstr on class name, then return its ClassPrivate->SuperStruct FName CI
    static u32 DiscoverCI(const char* substr, const char* exclude) {
        auto& actors = SDK::GetActors();
        for (i32 i = 0; i < actors.Count; i++) {
            auto* a = actors[i];
            if (!Mem::Valid(a) || !a->ClassPrivate) continue;
            const char* cn = ((UObject*)a->ClassPrivate)->NameCStr();
            if (!cn) continue;
            if (exclude && strstr(cn, exclude)) continue;
            if (strstr(cn, substr)) {
                // Walk up ONE level to the parent UClass and return its CI.
                // This gives us a broader CI that catches all subclasses.
                UClass* cls = (UClass*)a->ClassPrivate;
                if (cls->SuperStruct) {
                    u32 ci = ((UObject*)cls->SuperStruct)->NamePrivate.CI;
                    if (ci) return ci;
                }
                return ((UObject*)a->ClassPrivate)->NamePrivate.CI;
            }
        }
        return 0;
    }

    void InitFrame() {
        LocalPawn  = SDK::GetLocalPawn();
        LocalPC    = SDK::GetLocalController();
        Cam        = SDK::GetCamInfo();
        LocalWeapon = SDK::GetLocalWeapon();
        Movement   = LocalPawn ? SDK::GetMovement(LocalPawn) : nullptr;
        Inventory  = LocalPawn ? SDK::GetInventory(LocalPawn) : nullptr;
        LocalTribe = SDK::GetLocalTribeID();
        if (LocalPawn && LocalPawn->RootComponent)
            MyPos = ((USceneComponent*)LocalPawn->RootComponent)->RelLocation;
    }

    static CachedGlobals& Get() {
        static CachedGlobals g;
        static bool inited = false;
        if (!inited) {
            inited = true;
            g.CI_Meat          = ::FName_FromString("Meat").CI;
            g.CI_Kibble        = ::FName_FromString("Kibble").CI;
            g.CI_Berry         = ::FName_FromString("Berry").CI;
            g.CI_Crop          = ::FName_FromString("Crop").CI;
            // discover entity type CIs via single strstr pass
            g.CI_Character     = DiscoverCI("Character", "Dino");
            g.CI_InstancedMesh = DiscoverCI("InstancedStaticMesh", nullptr);
            g.CI_PlantSpecies  = DiscoverCI("PlantSpecies", nullptr);
            // cached UClass* pointers for exact IsA matching
            g.Class_DinoCharacter = (UClass*)SDK::FindObject("DinoCharacter");
            g.Class_Structure     = (UClass*)SDK::FindObject("Structure");
            g.Class_SupplyCrate   = (UClass*)SDK::FindObject("SupplyCrate");
            g.Class_Beacon        = (UClass*)SDK::FindObject("Beacon");
            g.Class_Bed           = (UClass*)SDK::FindObject("Bed");
            g.Class_Dropped       = (UClass*)SDK::FindObject("DroppedItem");
            g.Class_Turret        = (UClass*)SDK::FindObject("Turret");
            g.Class_Cache         = (UClass*)SDK::FindObject("Cache");
            g.Class_Storage       = (UClass*)SDK::FindObject("Storage");
        }
        return g;
    }
};

// fast class check using pre-cached FName CI
inline bool IsClass(UObject* obj, u32 ci) {
    if (!obj || !obj->ClassPrivate) return false;
    u32 objCI = ((UObject*)obj->ClassPrivate)->NamePrivate.CI;
    if (objCI == ci) return true;
    for (void* s = *(void**)((u64)obj->ClassPrivate + OFF::US_Super); s; s = *(void**)((u64)s + OFF::US_Super))
        if (((UObject*)s)->NamePrivate.CI == ci) return true;
    return false;
}

inline UFunction* GetCachedFn(UFunction*& cache, const char* name) {
    if (!cache) cache = SDK::FindFunction(name);
    return cache;
}

inline bool CallRPCFast(UObject* obj, UFunction*& fnCache, const char* name, void* params, i32 size) {
    auto* fn = GetCachedFn(fnCache, name);
    if (!fn || !obj || !Mem::Valid(obj)) return false;
    obj->ProcessEvent(fn, params);
    return true;
}

}
