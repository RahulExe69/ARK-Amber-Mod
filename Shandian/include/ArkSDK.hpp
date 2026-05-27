#pragma once
// ARK Mobile 2.0.28 | UE4.17 | ARM64
#include "utils.hpp"
#include <cmath>
#include <string>
#include <vector>
#include <unordered_map>

namespace OFF {
    constexpr uintptr_t GWorld        = 0x762f4b8;
    constexpr uintptr_t GEngine       = 0x762c798;
    constexpr uintptr_t GNames        = 0x74f0480;
    constexpr uintptr_t GUObjectArray = 0x74f6458;
    constexpr uintptr_t GMalloc       = 0x73e9450;

    constexpr auto UO_VTable = 0x00, UO_Flags = 0x08, UO_Index  = 0x0C;
    constexpr auto UO_Class  = 0x10, UO_Name  = 0x18, UO_Outer  = 0x20;
    constexpr auto UO_SIZE   = 0x28;

    constexpr auto UF_Next     = 0x28;
    constexpr auto US_Super    = 0x30, US_Children  = 0x38;
    constexpr auto US_PropSize = 0x40, US_MinAlign  = 0x44;
    constexpr auto US_SIZE     = 0x48;

    constexpr auto UFN_Flags     = 0x88, UFN_NumParms  = 0x8C;
    constexpr auto UFN_ParmsSize = 0x8E, UFN_RetOff    = 0x90;
    constexpr auto UFN_Func      = 0xA0;

    constexpr auto UP_ArrayDim  = 0x30, UP_ElemSize  = 0x34;
    constexpr auto UP_PropFlags = 0x38, UP_Offset    = 0x50;
    constexpr auto UP_NextLink  = 0x68;

    constexpr auto UC_Flags     = 0x88;

    constexpr auto UW_PersistentLevel = 0x58,  UW_Levels      = 0x178;
    constexpr auto UW_NetDriver       = 0x1C8, UW_GameState   = 0x248;
    constexpr auto UW_AuthGameMode    = 0x280, UW_TimeSeconds = 0x2C8;

    constexpr auto UL_Actors  = 0x80, UL_FoliageActor = 0x288;

    constexpr auto AA_RootComponent   = 0x108, AA_Owner         = 0x198;
    constexpr auto AA_TimeDilation    = 0x1C8, AA_HiddenFlags   = 0x90;
    constexpr auto AA_OwnedComponents = 0x508;

    constexpr auto AP_Controller    = 0x658, AP_PlayerState    = 0x8C0;
    constexpr auto AP_ControlInput  = 0x618, AC_Pawn           = 0x2A0;

    constexpr auto APC_CameraManager = 0x358;
    constexpr auto APC_StatusComponent = 0x1068;
    constexpr auto APC_TribeID       = 0x29C, APC_TargetingFlags = 0xBA8;
    constexpr auto APC_DeadFlags     = 0xBA0, APC_PlayerStatePtr  = 0x5F0;

    constexpr auto APC_StatusComp_StatusValues = 0x9C0;
    constexpr auto APC_StatusComp_ExpPoints    = 0x828;
    constexpr auto APC_StatusComp_ExtraLevel   = 0x824;

    constexpr auto AC_Mesh = 0x638;

    constexpr auto CMC_MaxWalkSpeed = 0x210, CMC_MaxFlySpeed  = 0x214;
    constexpr auto CMC_GravityScale = 0x224, CMC_MoveMode     = 0x250;
    constexpr auto CMC_Velocity     = 0x1B8;

    constexpr auto ASW_OwningChar  = 0xA98,  ASW_WeaponState = 0x140;
    constexpr auto ASW_WeaponRange = 0x1238, ASW_DamageType  = 0x1248;
    constexpr auto ASW_AmmoCount   = 0xFA0,  ASW_FireFlags   = 0xF50;
    // AShooterWeapon FireFlags bits
    constexpr u64 FF_BlockMounted  = 0x10;   // can't fire while riding
    constexpr u64 FF_BlockSwimming = 0x100;  // can't fire underwater
    constexpr u64 FF_BlockReload   = 0x8;    // mid-reload lock
    constexpr auto ASC_CurrentWeapon = 0x1828;
    constexpr auto ASC_RidingDino    = 0x15D8; // FWeakObjectPtr to APrimalDinoCharacter // AShooterCharacter::CurrentWeapon @ 0x2bc8bac

    constexpr auto UPR_DepthFlag    = 0x413, UPR_DepthStencil = 0x414;
    constexpr auto ABF_RecoilMult   = 0x6D8;

    constexpr auto UC_SizeX = 0x80, UC_SizeY = 0x84, UC_Canvas = 0x280;

    constexpr u32 FN_ChunkSize = 0x4000, FN_Stride = 0x8, FNE_String = 0x10;

    constexpr i32 VF_ProcessEvent          = 71;
    constexpr i32 VF_PostRender            = 130;
    constexpr i32 VF_SimInstantHit         = 354;
    constexpr i32 VF_StartFire             = 330;
    constexpr i32 VF_ReplicateMoveToServer = 340;
}

// ===================================================================
// basic types
// ===================================================================

struct FName {
    u32 CI; i32 N;
    FName() : CI(0), N(0) {}
    bool None() const { return CI == 0; }
};
static_assert(sizeof(FName) == 0x8);

struct FNameEntry { u8 d[0x10]; };

inline FNameEntry* FN_Entry(u32 idx) {
    u64 gn = Mem::Read<u64>(RVA(OFF::GNames));
    if (!Mem::Valid((void*)gn)) return 0;
    u64 ch = Mem::Read<u64>(gn + (idx / OFF::FN_ChunkSize) * OFF::FN_Stride);
    return Mem::Valid((void*)ch)
        ? (FNameEntry*)(ch + (idx % OFF::FN_ChunkSize) * OFF::FN_Stride)
        : 0;
}
inline const char* FN_CStr(FName n) {
    auto* e = FN_Entry(n.CI);
    return e ? (const char*)((u64)e + OFF::FNE_String) : "NULL";
}

inline bool operator==(const FName& a, const FName& b) { return a.CI == b.CI && a.N == b.N; }
inline bool operator!=(const FName& a, const FName& b) { return !(a == b); }

template<typename T>
struct TArray {
    T* Data; i32 Count, Max;
    i32 Num() const { return Count; }
    T& operator[](i32 i) { return Data[i]; }
};
static_assert(sizeof(TArray<void*>) == 0x10);

struct FString {
    char16_t* Data; i32 Count, Max;
    bool Ok() const { return Data && Count > 0; }
    std::string Str() const {
        if (!Ok()) return "";
        std::string out; out.reserve(Count);
        for (i32 i = 0; i < Count; i++) { char16_t c = Data[i]; out.push_back(c <= 0x7F ? (char)c : '?'); }
        return out;
    }
};
static_assert(sizeof(FString) == 0x10);

struct FVector  { float X, Y, Z;  FVector() : X(0),Y(0),Z(0) {} FVector(float x,float y,float z) : X(x),Y(y),Z(z) {}
    FVector operator+(const FVector& o) const { return {X+o.X,Y+o.Y,Z+o.Z}; }
    FVector operator-(const FVector& o) const { return {X-o.X,Y-o.Y,Z-o.Z}; }
    FVector operator*(float s) const { return {X*s,Y*s,Z*s}; }   FVector operator/(float s) const { return {X/s,Y/s,Z/s}; }
    FVector& operator+=(const FVector& o) { X+=o.X;Y+=o.Y;Z+=o.Z; return *this; }
    float Dot(const FVector& o) const { return X*o.X+Y*o.Y+Z*o.Z; }
    float Size() const { return sqrtf(X*X+Y*Y+Z*Z); }  float DistTo(const FVector& o) const { return (*this-o).Size(); }
    FVector Normalized() const { float s=Size(); return s>0.0001f?*this/s:FVector{}; }
    FVector Cross(const FVector& o) const { return {Y*o.Z-Z*o.Y,Z*o.X-X*o.Z,X*o.Y-Y*o.X}; }
};
static_assert(sizeof(FVector) == 0xC);

struct FVector2D { float X,Y; FVector2D():X(0),Y(0){} FVector2D(float x,float y):X(x),Y(y){} };
static_assert(sizeof(FVector2D) == 0x8);

struct FRotator { float Pitch,Yaw,Roll; FRotator():Pitch(0),Yaw(0),Roll(0){} FRotator(float p,float y,float r):Pitch(p),Yaw(y),Roll(r){}
    FVector ToVector() const { float cp=cosf(Pitch*0.017453292f),sp=sinf(Pitch*0.017453292f),cy=cosf(Yaw*0.017453292f),sy=sinf(Yaw*0.017453292f); return {cp*cy,cp*sy,sp}; }
    void Norm() { Yaw=fmodf(Yaw,360.f); if(Yaw<-180.f)Yaw+=360.f; if(Yaw>180.f)Yaw-=360.f; }
};
static_assert(sizeof(FRotator) == 0xC);

inline FRotator VecToRot(const FVector& v) { FRotator r; r.Yaw=atan2f(v.Y,v.X)*57.29578f; r.Pitch=atan2f(v.Z,sqrtf(v.X*v.X+v.Y*v.Y))*57.29578f; r.Roll=0; return r; }

struct FQuat        { float X,Y,Z,W; };
struct FTransform   { FQuat Rot; FVector Trans; float _p1C; FVector Scale; float _p2C; };
struct FLinearColor { float R,G,B,A; FLinearColor():R(0),G(0),B(0),A(1){} FLinearColor(float r,float g,float b,float a=1):R(r),G(g),B(b),A(a){} };
struct FColor       { u8 B,G,R,A; };

struct FVector_NetQuantize       { float X,Y,Z; };
struct FVector_NetQuantizeNormal { float X,Y,Z; };

enum EObjectFlags : u32 {
    RF_NoFlags=0, RF_Public=0x1, RF_ClassDefaultObject=0x10, RF_Transient=0x40,
    RF_Dynamic=0x4000000
};
// InternalObjectFlags on FUObjectItem : +0x0B bit 5 = PendingKill
enum EInternalObjectFlags : u32 {
    IF_PendingKill   = 0x20000000, // bit 29
    IF_Unreachable   = 0x40000000,
    IF_Garbage       = 0x80000000,
};
enum EFunctionFlags : u64 {
    FUNC_Native=0x400, FUNC_Event=0x800, FUNC_NetMulticast=0x4000,
    FUNC_Public=0x20000, FUNC_NetServer=0x200000, FUNC_NetClient=0x1000000
};
enum ECollisionChannel : u8 { ECC_Visibility=5, ECC_Camera=6 };
enum EPrimalStatus : u8 {
    STAT_Health=0,STAT_Stamina=1,STAT_Torpidity=2,STAT_Oxygen=3,STAT_Food=4,
    STAT_Water=5,STAT_Temp=6,STAT_Weight=7,STAT_Melee=8,STAT_Speed=9,STAT_Fortitude=10,STAT_Crafting=11
};
enum EPrimalEquipmentType : u8 {
    EQUIP_Helmet=0, EQUIP_Chest=1, EQUIP_Gloves=2, EQUIP_Leggings=3,
    EQUIP_Boots=4, EQUIP_Saddle=5, EQUIP_Shield=6
};
enum EPrimalItemStat : u8 {
    ITEMSTAT_Armor=0, ITEMSTAT_Durability=1, ITEMSTAT_Damage=2,
    ITEMSTAT_Hypothermic=3, ITEMSTAT_Hyperthermic=4, ITEMSTAT_Weight=5
};

// ===================================================================
// containers / math types
// ===================================================================

struct UObject;
inline bool IsValidUObject(UObject* obj);

struct FText { void* TextData; void* RefCount; i32 Flags; i32 _pad14; FText():TextData(0),RefCount(0),Flags(0),_pad14(0){} };
static_assert(sizeof(FText) == 0x18);

struct FMatrix { float M[4][4]; FMatrix(){ memset(M,0,sizeof(M)); M[0][0]=M[1][1]=M[2][2]=M[3][3]=1.f; }
    float* operator[](i32 r){ return M[r]; } const float* operator[](i32 r)const{ return M[r]; }
    FVector TransformPosition(const FVector& v)const{ return {v.X*M[0][0]+v.Y*M[1][0]+v.Z*M[2][0]+M[3][0],v.X*M[0][1]+v.Y*M[1][1]+v.Z*M[2][1]+M[3][1],v.X*M[0][2]+v.Y*M[1][2]+v.Z*M[2][2]+M[3][2]}; }
};
static_assert(sizeof(FMatrix) == 0x40);

struct FWeakObjectPtr {
    u32 ObjectIndex,ObjectSerialNumber;
    FWeakObjectPtr():ObjectIndex(0),ObjectSerialNumber(0){}
    bool IsValid() const { return ObjectIndex!=0; }
    UObject* Get() const;
};
static_assert(sizeof(FWeakObjectPtr) == 0x8);

struct FUObjectItem { UObject* Object; u32 Flags,SerialNumber; u64 _pad10; };
static_assert(sizeof(FUObjectItem) == 0x18);

struct TUObjectArray {
    u8 _pad0[0x10]; FUObjectItem* Objects; i32 MaxElements,NumElements,MaxChunks,NumChunks;
    i32 Num()const{ return NumElements; }
    FUObjectItem* IndexToObject(i32 idx)const{ return (idx>=0&&idx<NumElements)?&Objects[idx]:0; }
    UObject* GetObjectPtr(i32 idx)const{ auto* it=IndexToObject(idx); return it?it->Object:0; }
};

template<typename K,typename V> struct TMap { struct TPair{K Key;V Value;}; void* Data; i32 Num,MaxBits,Max,HashSize; u8 Hash[0x20]; };
template<typename T> struct TSet { T* Data; i32 Num,MaxBits,Max,HashSize; u8 Hash[0x20]; };
struct TBitArray { void* Data; i32 NumBits,MaxBits; };

struct FNetworkGUID { u32 Value; FNetworkGUID():Value(0){} bool IsValid()const{ return Value!=0; } };

struct FHitResult {
    u8 Flags; u8 _p01[3]; float Time; u8 _p08[4];
    FVector Location,ImpactPoint,Normal,ImpactNormal,TraceStart,TraceEnd;
    u8 _p54[4]; i32 FaceIndex;
    FWeakObjectPtr _physMaterial,_actor,_component;
    u8 _p74[0xC]; FName BoneName; i32 ElementIndex; float Distance; u8 _p90[4];
    bool bBlockingHit() const { return Flags&1; }
};

// ===================================================================
// object hierarchy
// ===================================================================

struct UObject {
    void** VTable; u32 Flags; i32 Index; void* ClassPrivate; FName NamePrivate; void* OuterPrivate;
    const char* NameCStr() const { return FN_CStr(NamePrivate); }
    std::string Name() const { auto* c=NameCStr(); return c?c:"NULL"; }
    const char* ClassName() const { return ClassPrivate?((UObject*)ClassPrivate)->NameCStr():nullptr; }
    template<typename T> T* Class() const { return (T*)ClassPrivate; }
    template<typename T> T* Outer() const { return (T*)OuterPrivate; }
    template<typename T> T GetVFunc(i32 idx) const { return (T)VTable[idx]; }
    void ProcessEvent(void* fn,void* p){ auto fp=GetVFunc<void(*)(UObject*,void*,void*)>(OFF::VF_ProcessEvent); fp(this,fn,p); }
    bool IsA(void* cmp) const { if(!ClassPrivate||!cmp)return false; for(void* s=ClassPrivate;s;s=*(void**)((u64)s+OFF::US_Super))if(s==cmp)return true; return false; }
    bool IsA(const char* cn) const { if(!ClassPrivate||!cn)return false; for(void* s=ClassPrivate;s;s=*(void**)((u64)s+OFF::US_Super)){const char* n=((UObject*)s)->NameCStr();if(n&&strstr(n,cn))return true;} return false; }
    bool HasFlag(u32 f) const { return (Flags&f)!=0; }
    UObject* GetOutermost() const;
    bool IsChildOf(void* bc) const;
    bool IsPendingKill() const {
        if (Index <= 0) return true;
        auto* arr = (TUObjectArray*)RVA(OFF::GUObjectArray);
        if (!Mem::Valid(arr)) return true;
        auto* item = arr->IndexToObject(Index);
        return !item || (item->Flags & IF_PendingKill);
    }
    std::string GetNameSafe() const;
};
static_assert(sizeof(UObject) == 0x28);

struct UField    : UObject { void* NextField; };
static_assert(sizeof(UField) == 0x30);

struct UStruct   : UField  { void* SuperStruct; void* Children; i32 PropSize,MinAlign; };
static_assert(sizeof(UStruct) == 0x48);

struct UClass    : UStruct { u32 ClassFlags; i32 _p8C; void* ClassWithin; u8 _p94[4]; UObject* CDO; };
struct UScriptStruct : UStruct { u8 _pad[0x40]; };
struct UEnum     : UField  { u8 _pad[0x20]; };

struct UFunction : UStruct {
    u32 FuncFlags; u16 NumParms,ParmsSize,RetOff; u8 _p94[0xA]; void* FirstProp; void* NativeFunc; void* Func;
};

struct UProperty  : UField { i32 ArrayDim,ElemSize; u64 PropFlags; i32 RepIndex; FName RepNotify; u8 _p4C[4]; i32 OffsetInt; u8 _p54[0x14]; void* NextProp; };
struct UBoolProperty : UProperty {};
struct UIntProperty : UProperty {};
struct UFloatProperty : UProperty {};
struct UObjectProperty : UProperty {};
struct UStructProperty : UProperty {};
struct UArrayProperty : UProperty {};
struct UMapProperty : UProperty {};
struct USetProperty : UProperty {};
struct UNameProperty : UProperty {};
struct UStrProperty : UProperty {};
struct UTextProperty : UProperty {};

struct AActor : UObject { u8 _28[0xE0]; void* RootComponent; u8 _110[0x88]; void* Owner; u8 _1A0[0x28]; float TimeDilation; u8 _1CC[4]; };
struct APawn  : AActor  { u8 _1D0[0x448]; FVector ControlInput; u8 _624[0x34]; void* AController; u8 _65C[0x264]; void* PlayerState; };
struct ACharacter : APawn { u8 _8D0[0x690]; };
struct AController : AActor { u8 _1D0[0xCC]; void* Pawn; };
struct APlayerController : AController { u8 _2A8[0xB0]; void* CameraManager; };
struct APlayerCameraManager : AActor { u8 _1D0[0x1B60]; FVector CamLoc; FRotator CamRot; float CamFOV; };
struct APlayerState : AActor { u8 _1D0[0x1D0]; FString PlayerName; };
struct UWorld : UObject { u8 _28[0x30]; void* PersistentLevel; u8 _60[0x118]; TArray<void*> Levels; u8 _188[0xC0]; void* GameState; u8 _250[0x30]; void* AuthGameMode; u8 _288[0x40]; float TimeSeconds; };
struct ULevel : UObject { u8 _28[0x58]; TArray<AActor*> Actors; u8 _90[0x270]; void* WorldSettings; };
struct AGameStateBase : AActor { u8 _1D0[0x220]; TArray<APlayerState*> PlayerArray; };
struct AGameModeBase  : AActor {};
struct UActorComponent : UObject { u8 _28[0xE0]; void* AsOwner; };
struct USceneComponent : UActorComponent { u8 _110[0x50]; FVector RelLocation; FRotator RelRotation; FVector RelScale; u8 _18C[0x34]; FTransform Comp2World; };
struct UPrimitiveComponent : USceneComponent { u8 _1F0[0x20]; };
struct USkeletalMeshComponent : UPrimitiveComponent { u8 _210[0x60]; FTransform BoneSpace[1]; };
struct UCharacterMovementComponent : UActorComponent { u8 _110[0xA0]; float Friction; FVector Velocity; u8 _1C4[0x48]; float MaxWalkSpeed,MaxFlySpeed,MaxSwimSpeed; u8 _21C[0x8]; float GravityScale; u8 _228[0x28]; u8 MoveMode; };
struct UInstancedStaticMeshComponent : UPrimitiveComponent {
    u8 _210[0x798];
    i32 InstanceCount() const { return *(i32*)((u64)this+0xA10); }
    FTransform GetInstanceWorldTransform(i32 idx) const { FTransform t; auto fn=(bool(*)(void*,i32,FTransform*,bool))RVA(0x49fb780); if(fn)fn((void*)this,idx,&t,true); return t; }
};
struct UFoliageInstancedStaticMeshComponent : UInstancedStaticMeshComponent { u8 _pad[0x100]; };
struct AInstancedFoliageActor : AActor { u8 _pIFA[0x5A8]; };
struct UPrimalCharacterStatusComponent : UActorComponent {};
struct UPrimalInventoryComponent : UActorComponent { u8 _pad[0x100]; };
struct UCanvas : UObject { u8 _pUC[0x58]; i32 SizeX,SizeY; u8 _p88[0x1F8]; void* Canvas; };
struct UGameViewportClient : UObject { u8 _pad[0x100]; };
struct UMaterialInterface : UObject {};
struct UMaterialInstanceDynamic : UMaterialInterface {};
struct UTexture2D : UObject {
    // UObject = 0x28 bytes (VTable at 0x00).
    // +0x48 = Flags1  ->  pad = 0x48 - 0x28 = 0x20
    u8 _pad48[0x20];
    u8  Flags1;                   // 0x48
    u8  _pad49[0x1D];
    u8  SRGB;                     // 0x66
    u8  CompressionSettings;      // 0x67
    u8  _pad68[0x88];
    void* PlatformData;           // 0xF0
};

// wrapper
namespace FBulkData {
    inline void* Lock(void* bulk, u32 flags) {
        auto fn = (void*(*)(void*,u32))RVA(0x3572c80);
        return fn ? fn(bulk, flags) : nullptr;
    }
    inline void  Unlock(void* bulk) {
        auto fn = (void(*)(void*))RVA(0x3572d7c);
        if (fn) fn(bulk);
    }
}

inline UTexture2D* CreateTransientTexture(int w, int h, int fmt = 2) {
    auto fn = (UTexture2D*(*)(int,int,unsigned int))RVA(0x4debb90);
    return fn ? fn(w, h, (u32)fmt) : nullptr;
}

inline bool UploadTextureRGBA(UTexture2D* tex, const u8* rgba, int w, int h) {
    if (!tex || !Mem::Valid(tex)) return false;
    void* pd  = *(void**)((u64)tex + 0xF0);
    if (!pd || !Mem::Valid(pd)) return false;
    void* m0  = *(void**)((u64)pd  + 0x10);
    if (!m0 || !Mem::Valid(m0)) return false;
    void* bulk = (void*)((u64)m0 + 0x08);
    u8* dst = (u8*)FBulkData::Lock(bulk, 2u);
    if (!dst) return false;
    memcpy(dst, rgba, (size_t)w * h * 4);
    FBulkData::Unlock(bulk);
    // UpdateResource —> VTable[78]
    auto update = (void(*)(void*))tex->VTable[78];
    if (update) update(tex);
    return true;
}
struct UPhysicalMaterial : UObject {};
struct UFont : UObject {};

struct AShooterCharacter        : ACharacter      { u8 _pad[0x4000]; };
struct APrimalCharacter         : AShooterCharacter {};
struct APrimalDinoCharacter     : APrimalCharacter {};
struct AShooterPlayerController : APlayerController {};
struct AShooterPlayerState      : APlayerState {};
struct AShooterGameMode         : AGameModeBase {};
struct AShooterGameState        : AGameStateBase {};
struct AShooterWeapon           : AActor          { u8 _pad[0x2000]; };
struct AShooterWeapon_Projectile : AShooterWeapon {};
struct APrimalWeaponBow         : AShooterWeapon {};
struct AShooterHUD              : AActor {};
struct APrimalStructure         : AActor {};
struct APrimalBuff              : AActor {};

struct UPrimalItem : UObject {
    u8 _padItem[0x5F0];
    UTexture2D* ItemIcon;       // +0x5F0 GetItemIcon @ 0x2a82e94
    UTexture2D* BrokenIcon;     // +0x5F8
    float Durability;           // approximation
    float MaxDurability;
};
struct FTribeData               { u8 _pad[0x100]; };

// ===================================================================
// FString conversion helpers
// ===================================================================

inline FString FStringFromAnsi(const char* txt, char16_t* backing, i32 maxLen) {
    FString out; i32 len = 0;
    while (txt[len] && len < maxLen) { backing[len] = (char16_t)txt[len]; len++; }
    out.Data = backing; out.Count = len; out.Max = maxLen; return out;
}

inline void AnsiToWide(const char* src, char16_t* dst, i32 max) { i32 i=0; while(src[i]&&i<max){dst[i]=(char16_t)src[i];i++;} if(i<max)dst[i]=0; }

inline FName FName_FromString(const char* str) {
    FName out{}; i32 len=strlen(str);
    char16_t* wbuf=(char16_t*)alloca((len+1)*2);
    AnsiToWide(str,wbuf,len+1);
    auto ctor=(void(*)(FName*,const char16_t*,i32))RVA(0x350425c);
    if(ctor)ctor(&out,wbuf,1); return out;
}

// ===================================================================
// UObject helpers
// ===================================================================

inline bool IsValidUObject(UObject* obj) {
    if(!obj||!Mem::Valid(obj))return false;
    if(!Mem::Valid(obj->VTable))return false;
    if(!obj->ClassPrivate||!Mem::Valid(obj->ClassPrivate))return false;
    if(obj->Index<=0||obj->Index>3000000)return false;
    return true;
}
inline UObject* UObject::GetOutermost() const {
    UObject* outer=(UObject*)OuterPrivate; UObject* result=(UObject*)this;
    while(outer){result=outer;outer=(UObject*)outer->OuterPrivate;} return result;
}
inline bool UObject::IsChildOf(void* bc) const {
    if(!ClassPrivate||!bc)return false;
    for(void* s=ClassPrivate;s;s=*(void**)((u64)s+OFF::US_Super))if(s==bc)return true; return false;
}
inline std::string UObject::GetNameSafe() const {
    if(!IsValidUObject((UObject*)this))return"NULL"; auto* c=NameCStr(); return c?c:"NULL";
}
//rly useful
inline UObject* FWeakObjectPtr::Get() const {
    if(!ObjectIndex)return 0;
    auto* arr=(TUObjectArray*)RVA(OFF::GUObjectArray); if(!Mem::Valid(arr))return 0;
    auto* item=arr->IndexToObject(ObjectIndex);
    if(!item||item->SerialNumber!=ObjectSerialNumber)return 0;
    if(!IsValidUObject(item->Object))return 0;
    if(item->Object->IsPendingKill())return 0;
    return item->Object;
}

// ===================================================================
// SDK accessors
// ===================================================================

namespace SDK {

inline UWorld* GetGWorld(){ return Mem::Read<UWorld*>(RVA(OFF::GWorld)); }
inline ULevel* GetPersistentLevel(){ auto* w=GetGWorld(); return w?(ULevel*)w->PersistentLevel:0; }
inline void* GetWorldSettings(){ auto* l=GetPersistentLevel(); return l?l->WorldSettings:0; }
inline TArray<AActor*>& GetActors(){ static TArray<AActor*> e; auto* l=GetPersistentLevel(); return l?l->Actors:e; }
inline AGameStateBase* GetGameState(){ auto* w=GetGWorld(); return w?(AGameStateBase*)w->GameState:0; }
inline float GetTime(){ auto* w=GetGWorld(); return w?w->TimeSeconds:0.f; }
inline TArray<APlayerState*>& GetPlayerArray(){ static TArray<APlayerState*> e; auto* gs=GetGameState(); return gs?gs->PlayerArray:e; }

inline APlayerController* GetLocalController() {
    auto& a=GetActors();
    for(i32 i=0;i<a.Count;i++){ auto* act=a[i]; if(!Mem::Valid(act)||!act->ClassPrivate)continue; if(((APlayerController*)act)->Pawn){ auto* n=((UClass*)act->ClassPrivate)->NameCStr(); if(n&&strstr(n,"PlayerController"))return(APlayerController*)act; } }
    return 0;
}
inline APawn* GetLocalPawn(){ auto* pc=GetLocalController(); return pc?(APawn*)pc->Pawn:0; }
inline APlayerCameraManager* GetCameraManager(){ auto* pc=GetLocalController(); return pc?(APlayerCameraManager*)pc->CameraManager:0; }
inline AShooterWeapon* GetLocalWeapon(){ auto* p=GetLocalPawn(); if(!p)return 0; auto& a=GetActors(); for(i32 i=0;i<a.Count;i++){ auto* act=a[i]; if(!Mem::Valid(act)||!act->ClassPrivate||act->Owner!=p)continue; if(strstr(((UClass*)act->ClassPrivate)->NameCStr(),"Weapon"))return(AShooterWeapon*)act; } return 0; }

template<typename T>
T* FindComponent(AActor* actor, const char* className) {
    if(!Mem::Valid(actor))return 0;
    auto* data=*(TArray<void*>**)((u64)actor+OFF::AA_OwnedComponents);
    i32 count=*(i32*)((u64)actor+OFF::AA_OwnedComponents+0x28);
    if(!data||count<=0)return 0;
    for(i32 i=0;i<count;i++){
        auto* comp=(UActorComponent*)data->Data[i];
        if(!Mem::Valid(comp)||!comp->ClassPrivate)continue;
        if(comp->IsA(className))return(T*)comp;
    }
    return 0;
}

inline UCharacterMovementComponent* GetMovement(AActor* a){ return FindComponent<UCharacterMovementComponent>(a,"CharacterMovement"); }
inline USkeletalMeshComponent* GetMesh(AActor* a){ return FindComponent<USkeletalMeshComponent>(a,"SkeletalMesh"); }
inline UPrimalInventoryComponent* GetInventory(AActor* a){ return FindComponent<UPrimalInventoryComponent>(a,"Inventory"); }
inline UPrimalCharacterStatusComponent* GetStatusComponent(AActor* a){ return FindComponent<UPrimalCharacterStatusComponent>(a,"Status"); }

//tribe/enemy
inline i32 GetTribeID(AActor* a){ if(!Mem::Valid(a))return 0; return *(i32*)((u64)a+OFF::APC_TribeID); }
inline i32 GetLocalTribeID(){ auto* lp=GetLocalPawn(); return lp?GetTribeID(lp):0; }
inline bool IsEnemy(AActor* a){ if(!Mem::Valid(a))return false; auto* lp=GetLocalPawn(); if(!lp||a==lp)return false; i32 mt=GetTribeID(lp),tt=GetTribeID(a); if(mt==0&&tt==0)return true; if(mt==tt)return false; return true; }
inline bool IsEnemyPlayer(AActor* a){ if(!Mem::Valid(a)||!a->ClassPrivate)return false; auto* lp=GetLocalPawn(); if(!lp||a==lp)return false; const char* n=((UClass*)a->ClassPrivate)->NameCStr(); if(!n)return false; if(!strstr(n,"Character")&&!strstr(n,"Player"))return false; i32 mf=*(i32*)((u64)lp+OFF::APC_TargetingFlags),tf=*(i32*)((u64)a+OFF::APC_TargetingFlags); if(((mf>>12)&1)!=((tf>>12)&1))return false; return IsEnemy(a); }
inline bool IsDead(AActor* a){ if(!Mem::Valid(a))return true; return (*(u8*)((u64)a+OFF::APC_DeadFlags)>>5)&1; }
//most of the stats
inline float GetStatusValue(AActor* a,i32 idx){ auto* sc=GetStatusComponent(a); if(!Mem::Valid(sc))return 0.f; return *(float*)((u64)sc+OFF::APC_StatusComp_StatusValues+idx*4); }
inline float GetHP(AActor* a){ return GetStatusValue(a,0); }
inline float GetStamina(AActor* a){ return GetStatusValue(a,1); }
inline float GetTorpidity(AActor* a){ return GetStatusValue(a,2); }
inline float GetFood(AActor* a){ return GetStatusValue(a,3); }
inline float GetWater(AActor* a){ return GetStatusValue(a,4); }
inline float GetWeight(AActor* a){ return GetStatusValue(a,7); }
inline i32 GetLevel(AActor* a){ auto* sc=GetStatusComponent(a); if(!Mem::Valid(sc))return 0; return *(i32*)((u64)sc+OFF::APC_StatusComp_ExpPoints)+*(i32*)((u64)sc+OFF::APC_StatusComp_ExtraLevel); }

//line trace
using RaycastFn=u64(*)(UWorld*,FHitResult*,u32,void*,void*,float,float,float,float,float,float);
constexpr u64 RC_ADDR=0x480369c;
inline bool LineTraceSingle(FVector s,FVector e,FHitResult* o,u32 ch=5){ auto* w=GetGWorld(); if(!w)return false; u8 qb[0x100]={},rb[0x100]={}; auto fn=(RaycastFn)RVA(RC_ADDR); return fn(w,o,ch,qb,rb,s.X,s.Y,s.Z,e.X,e.Y,e.Z)!=0; }
inline bool IsVisible(FVector f,FVector t){ FHitResult h={}; LineTraceSingle(f,t,&h); return h._actor.Get()==0; }

//w2s
struct CamInfo { FVector Loc; FRotator Rot; float FOV; bool Ok; };
inline CamInfo GetCamInfo(){ CamInfo c={}; auto* cm=GetCameraManager(); if(!cm)return c; c.Loc=cm->CamLoc; c.Rot=cm->CamRot; c.FOV=cm->CamFOV; c.Ok=true; return c; }

using ProjectWorldToScreenFn=bool(*)(const APlayerController*,const FVector&,FVector2D&,bool);
inline bool WorldToScreen(FVector wp,FVector2D* out){ auto* pc=GetLocalController(); if(!pc)return false; auto fn=(ProjectWorldToScreenFn)RVA(0x499ab84); return fn?fn(pc,wp,*out,false):false; }

// ===================================================================
// FindObject / GetFullName
// ===================================================================

inline UObject* FindObject(const char* name){ i32 len=strlen(name); char16_t* wbuf=(char16_t*)alloca((len+1)*2); for(i32 i=0;i<len;i++)wbuf[i]=name[i]; wbuf[len]=0; FName fname; auto ctor=(void(*)(FName*,const char16_t*,i32))RVA(0x350425c); if(ctor)ctor(&fname,wbuf,1); if(fname.None())return 0; auto fn=(UObject*(*)(UClass*,UObject*,FName,bool,bool,u32,u32))RVA(0x366a64c); return fn?fn(0,0,fname,false,false,0,0):0; }
template<typename T>T* FindObject(const char* name){ return(T*)FindObject(name); }
inline UClass* StaticClass(const char* name){ return(UClass*)FindObject(name); }

inline std::string GetFullName(UObject* obj,const UObject* stopOuter=0){ static char buf[1024]; FString fs; fs.Data=(char16_t*)buf; fs.Count=0; fs.Max=512; auto fn=(void(*)(UObject*,const UObject*,FString*))RVA(0x36569f8); if(fn)fn(obj,stopOuter,&fs); buf[fs.Count]=0; return std::string(buf,fs.Count); }

//draw
struct FCanvasUVTri { FVector2D V0_Pos,V0_UV,V1_Pos,V1_UV,V2_Pos,V2_UV; };

using K2_DrawLineFn=void(*)(UCanvas*,FVector2D,FVector2D,float,FLinearColor);
using K2_DrawTextFn=void(*)(UCanvas*,UFont*,const FString&,FVector2D,FLinearColor,float,FLinearColor,FVector2D,bool,bool,bool,FLinearColor);
using K2_DrawBoxFn=void(*)(UCanvas*,FVector2D,FVector2D,float,FLinearColor);
using K2_DrawTextureFn=void(*)(UCanvas*,UTexture2D*,FVector2D,FVector2D,FVector2D,FVector2D,FLinearColor,int,float,FVector2D);
using K2_DrawMaterialFn=void(*)(UCanvas*,UMaterialInterface*,FVector2D,FVector2D,FVector2D,FVector2D,float,FVector2D);
using K2_DrawPolygonFn=void(*)(UCanvas*,UTexture2D*,FVector2D,FVector2D,i32,FLinearColor);
using K2_DrawTriangleFn=void(*)(UCanvas*,UTexture2D*,TArray<FCanvasUVTri>);
using K2_DrawMaterialTriangleFn=void(*)(UCanvas*,UMaterialInterface*,TArray<FCanvasUVTri>);
using SetDrawColorFn=void(*)(UCanvas*,const FLinearColor&);
using DrawTextFn=void(*)(UCanvas*,UFont*,const FString&,float,float,float,float,const void*);
using CanvasDrawItemFn=void(*)(UCanvas*,void*,float,float);
using CanvasStrLenFn=void(*)(UCanvas*,UFont*,const FString&,float&,float&);

inline void K2_DrawLine(UCanvas* c,FVector2D f,FVector2D t,float th,FLinearColor cl){ auto fn=(K2_DrawLineFn)RVA(0x4e5a138); if(fn)fn(c,f,t,th,cl); }
inline void K2_DrawBox(UCanvas* c,FVector2D p,FVector2D s,float th,FLinearColor cl){ auto fn=(K2_DrawBoxFn)RVA(0x4e5a888); if(fn)fn(c,p,s,th,cl); }
inline void K2_DrawText(UCanvas* c,UFont* font,const char* txt,FVector2D pos,FLinearColor col,float scale=1.f){ FString fs;char16_t buf[256];fs=FStringFromAnsi(txt,buf,256); auto fn=(K2_DrawTextFn)RVA(0x4e5a400); if(fn)fn(c,font,fs,pos,col,scale,{0,0,0,0},{0,0},false,false,false,{0,0,0,0}); }
inline void K2_DrawTexture(UCanvas* c,UTexture2D* tex,FVector2D sp,FVector2D ss,FVector2D up,FVector2D us,FLinearColor tint,int bm=0,float r=0,FVector2D pv={0,0}){ auto fn=(K2_DrawTextureFn)RVA(0x4e5a1c0); if(fn)fn(c,tex,sp,ss,up,us,tint,bm,r,pv); }
inline void K2_DrawMaterial(UCanvas* c,UMaterialInterface* mat,FVector2D sp,FVector2D ss,FVector2D up,FVector2D us,float r=0,FVector2D pv={0,0}){ auto fn=(K2_DrawMaterialFn)RVA(0x4e5a298); if(fn)fn(c,mat,sp,ss,up,us,r,pv); }
inline void K2_DrawPolygon(UCanvas* c,UTexture2D* tex,FVector2D sp,FVector2D rad,i32 ns,FLinearColor col){ auto fn=(K2_DrawPolygonFn)RVA(0x4e5abd0); if(fn)fn(c,tex,sp,rad,ns,col); }
inline void K2_DrawTriangle(UCanvas* c,UTexture2D* tex,TArray<FCanvasUVTri> tris){ auto fn=(K2_DrawTriangleFn)RVA(0x4e5a914); if(fn)fn(c,tex,tris); }
inline void K2_DrawMaterialTriangle(UCanvas* c,UMaterialInterface* mat,TArray<FCanvasUVTri> tris){ auto fn=(K2_DrawMaterialTriangleFn)RVA(0x4e5aa7c); if(fn)fn(c,mat,tris); }
inline void CanvasSetColor(UCanvas* c,FLinearColor col){ auto fn=(SetDrawColorFn)RVA(0x4e57888); if(fn)fn(c,col); }
inline void DrawText(UCanvas* c,UFont* font,const char* txt,float x,float y,float scale=1.f){ char16_t buf[256]; FString fs=FStringFromAnsi(txt,buf,256); auto fn=(DrawTextFn)RVA(0x4e58e3c); if(fn)fn(c,font,fs,x,y,scale,scale,nullptr); }
inline void CanvasStrLen(UCanvas* c,UFont* font,const FString& txt,float& w,float& h){ auto fn=(CanvasStrLenFn)RVA(0x4e59168); if(fn)fn(c,font,txt,w,h); }

// ===================================================================
// bones
// ===================================================================

inline FVector GetBoneLocation(USkeletalMeshComponent* mesh,FName bn,i32 sp=0){ auto fn=(FVector(*)(void*,FName,i32))RVA(0x48bb70c); return fn?fn(mesh,bn,sp):FVector{}; }
inline i32 GetBoneIndex(USkeletalMeshComponent* mesh,FName bn){ auto fn=(i32(*)(void*,FName))RVA(0x48b9a10); return fn?fn(mesh,bn):-1; }
inline FName GetBoneName(USkeletalMeshComponent* mesh,i32 idx){ auto fn=(FName(*)(void*,i32))RVA(0x48b9ad0); return fn?fn(mesh,idx):FName{}; }

// ===================================================================
// FOV / camera
// ===================================================================

inline void SetFOV(float fov){ auto* cm=GetCameraManager(); if(!cm)return; auto fn=(void(*)(void*,float))RVA(0x4c34508); if(fn)fn(cm,fov); }
inline void UnlockFOV(){ auto* cm=GetCameraManager(); if(!cm)return; auto fn=(void(*)(void*))RVA(0x4c34510); if(fn)fn(cm); }
inline float GetFOVAngle(){ auto* cm=GetCameraManager(); if(!cm)return 90.f; auto fn=(float(*)(void*))RVA(0x4c344f4); return fn?fn(cm):90.f; }
inline void PeripheralVision(float fov){ UnlockFOV(); SetFOV(fov); }
inline void GetPlayerViewPoint(FVector& loc,FRotator& rot){ auto* pc=GetLocalController(); if(!pc)return; auto fn=(void(*)(void*,FVector*,FRotator*))RVA(0x4c397e8); if(fn)fn(pc,&loc,&rot); }

// ===================================================================
// material / color / chams
// ===================================================================

inline void SetMaterialColor(UMaterialInstanceDynamic* mid,const char* param,FLinearColor col){ auto fn=(void(*)(void*,FName,FLinearColor))RVA(0x4ab81bc); if(!fn)return; FName p=FName_FromString(param); fn(mid,p,col); }
inline UMaterialInstanceDynamic* CreateDynamicMaterialInstance(UPrimitiveComponent* comp,i32 idx=0){ auto fn=(void*(*)(void*,i32,void*))RVA(0x48880bc); return(UMaterialInstanceDynamic*)(fn?fn(comp,idx,0):0); }

inline void SetChams(UPrimitiveComponent* mesh,bool on,FLinearColor col={1,0,0,1}){ if(!Mem::Valid(mesh))return; u8* fb=(u8*)mesh+OFF::UPR_DepthFlag; if(on){Bits::Set(*fb,(u8)(1<<6)); auto* mid=CreateDynamicMaterialInstance(mesh,0); if(mid)SetMaterialColor(mid,"BodyColor",col); }else{Bits::Clear(*fb,(u8)(1<<6));} }

inline void RGBWeapons(FLinearColor col){ auto& a=GetActors(); for(i32 i=0;i<a.Count;i++){ auto* act=a[i]; if(!Mem::Valid(act)||!act->ClassPrivate)continue; if(!act->IsA("Weapon")||act->Owner!=GetLocalPawn())continue; auto* mesh=GetMesh(act); if(!mesh)continue; auto* mid=CreateDynamicMaterialInstance(mesh,0); if(mid)SetMaterialColor(mid,"BodyColor",col); } }
inline void RGBPlayer(FLinearColor col){ auto* lp=GetLocalPawn(); if(!lp)return; auto* mesh=GetMesh(lp); if(!mesh)return; auto* mid=CreateDynamicMaterialInstance(mesh,0); if(mid)SetMaterialColor(mid,"BodyColor",col); }
inline void EnableEnemyChams(bool on,FLinearColor col={1,0,0,1}){ auto& a=GetActors(); for(i32 i=0;i<a.Count;i++){ auto* act=a[i]; if(!IsEnemyPlayer(act))continue; auto* mesh=GetMesh(act); SetChams(mesh,on,col); } }
inline void EnableDinoChams(bool on,FLinearColor col={0,1,0,1}){ auto& a=GetActors(); for(i32 i=0;i<a.Count;i++){ auto* act=a[i]; if(!Mem::Valid(act)||!act->ClassPrivate)continue; if(!act->IsA("DinoCharacter"))continue; auto* mesh=GetMesh(act); SetChams(mesh,on,col); } }

//for speed hax
using RepMoveFn=void(*)(UCharacterMovementComponent*,float,const FVector&);
inline void ReplicateMoveToServer(UCharacterMovementComponent* cmc,float delta,const FVector& accel){ auto fn=(RepMoveFn)RVA(0x48457f4); if(fn)fn(cmc,delta,accel); }

// ===================================================================
// rocket spam / fire projectile
// ===================================================================

// ===================================================================
// foliage / resource ESP
// ===================================================================

inline AInstancedFoliageActor* GetInstancedFoliageActor(){ auto* l=GetPersistentLevel(); if(!l)return 0; auto* wp=(FWeakObjectPtr*)((u64)l+OFF::UL_FoliageActor); return(AInstancedFoliageActor*)wp->Get(); }

inline void ForEachFoliageInstance(void(*cb)(const FVector&,i32,const char*)){
    auto* ifa=GetInstancedFoliageActor(); if(!ifa)return;
    auto& a=GetActors();
    for(i32 i=0;i<a.Count;i++){ auto* act=a[i]; if(!Mem::Valid(act)||act->OuterPrivate!=ifa)continue; if(!act->IsA("InstancedStaticMesh"))continue;
        auto* ismc=(UInstancedStaticMeshComponent*)act; i32 n=ismc->InstanceCount(); if(n<=0||n>500000)continue;
        const char* mn=act->ClassPrivate?((UObject*)act->ClassPrivate)->NameCStr():"Unknown";
        for(i32 j=0;j<n;j++){ FTransform t=ismc->GetInstanceWorldTransform(j); cb(t.Trans,j,mn); }
    }
}

// ===================================================================
// object cache (player/dino/struct lists
// ===================================================================

struct ObjectCache { std::vector<AActor*> Players,Dinos,Structures; i32 FrameCounter; };
inline ObjectCache g_Cache;

inline void RefreshCache(i32 rate=30){
    if(++g_Cache.FrameCounter%rate!=0)return;
    g_Cache.Players.clear(); g_Cache.Dinos.clear(); g_Cache.Structures.clear();
    auto& a=GetActors();
    for(i32 i=0;i<a.Count;i++){ auto* act=a[i]; if(!Mem::Valid(act)||!act->ClassPrivate)continue;
        if(strstr(((UClass*)act->ClassPrivate)->NameCStr(),"Character")&&!strstr(((UClass*)act->ClassPrivate)->NameCStr(),"Dino"))g_Cache.Players.push_back(act);
        else if(act->IsA("DinoCharacter"))g_Cache.Dinos.push_back(act);
        else if(act->IsA("Structure"))g_Cache.Structures.push_back(act);
    }
}

inline void GetBuffs(APrimalCharacter* c,TArray<APrimalBuff*>* out){ auto fn=(void(*)(void*,void*))RVA(0x299080c); if(fn)fn(c,out); }
inline UPrimalInventoryComponent* GetLocalInventory(){ auto* p=GetLocalPawn(); if(!p)return 0; return FindComponent<UPrimalInventoryComponent>(p,"Inventory"); }

// ===================================================================
// memory allocation (FMemory via GMalloc @ 0x73e51d0)
// ===================================================================

inline void* FMalloc(u32 size){ auto fn=(void*(*)(u32))RVA(0x33c1bfc); return fn?fn(size):0; }
inline void FFree(void* ptr){ auto fn=(void(*)(void*))RVA(0x33c1d14); if(fn)fn(ptr); }
inline void* FRealloc(void* ptr,u32 size){ auto fn=(void*(*)(void*,u32))RVA(0x33c1c80); return fn?fn(ptr,size):0; }

// ===================================================================
// cached component queries
// ===================================================================

struct CachedComponents {
    USkeletalMeshComponent*          Mesh;
    UCharacterMovementComponent*     Movement;
    UPrimalCharacterStatusComponent* Status;
    UPrimalInventoryComponent*       Inventory;
    AActor*                          Owner;  // set when this cache was filled
    bool                             Valid() const { return Owner && Mem::Valid(Owner); }
};

inline CachedComponents CacheComponents(AActor* a) {
    CachedComponents c = {};
    if (!Mem::Valid(a)) return c;
    c.Owner = a;
    auto* data = *(TArray<void*>**)((u64)a + OFF::AA_OwnedComponents);
    i32  count = *(i32*)((u64)a + OFF::AA_OwnedComponents + 0x28);
    if (!data || count <= 0) return c;
    for (i32 i = 0; i < count; i++) {
        auto* comp = (UActorComponent*)data->Data[i];
        if (!Mem::Valid(comp) || !comp->ClassPrivate) continue;
        const char* n = ((UObject*)comp->ClassPrivate)->NameCStr();
        if (!n) continue;
        if (strstr(n, "SkeletalMesh"))      c.Mesh      = (USkeletalMeshComponent*)comp;
        else if (strstr(n, "CharacterMovement")) c.Movement  = (UCharacterMovementComponent*)comp;
        else if (strstr(n, "Status"))       c.Status    = (UPrimalCharacterStatusComponent*)comp;
        else if (strstr(n, "Inventory"))    c.Inventory = (UPrimalInventoryComponent*)comp;
        // all four found? stop early & too lazy to go find the actual full names
        if (c.Mesh && c.Movement && c.Status && c.Inventory) break;
    }
    return c;
}

inline std::unordered_map<AActor*, CachedComponents> g_ComponentCache;
inline CachedComponents& GetCached(AActor* a) {
    auto it = g_ComponentCache.find(a);
    if (it != g_ComponentCache.end() && it->second.Valid()) return it->second;
    g_ComponentCache[a] = CacheComponents(a);
    return g_ComponentCache[a];
}

// ===================================================================
// armor / equipment ESP
// ===================================================================

// EquippedItems array: UPrimalInventoryComponent + 0x238 (568), Count at +0x240 (576)
// verified from GetEquippedItemOfType @ 0x2a5b910
inline UPrimalItem* GetEquippedItem(AActor* a, EPrimalEquipmentType slot) {
    auto* inv = GetInventory(a);
    if (!inv) return 0;
    auto fn = (UPrimalItem*(*)(void*,u8))RVA(0x2a6a320);
    return fn ? fn(inv, (u8)slot) : 0;
}
inline float GetEquippedArmorRating(AActor* a, EPrimalEquipmentType slot) {
    auto* inv = GetInventory(a);
    if (!inv) return 0.f;
    auto fn = (float(*)(void*,u8))RVA(0x2a6c654);
    return fn ? fn(inv, (u8)slot) : 0.f;
}
inline float GetTotalArmor(AActor* a) {
    float total = 0.f;
    for (u8 s = 0; s <= 4; s++) total += GetEquippedArmorRating(a, (EPrimalEquipmentType)s);
    return total;
}
inline float GetItemDurability(UPrimalItem* item) {
    if (!item) return 0.f;
    auto fn = (float(*)(void*))RVA(0x2a87a28);
    return fn ? fn(item) : 0.f;
}
inline UTexture2D* GetItemIcon(UPrimalItem* item) {
    if (!item) return 0;
    auto fn = (UTexture2D*(*)(void*))RVA(0x2a918a4);
    return fn ? fn(item) : 0;
}

struct ArmorInfo { UTexture2D* Icon; float Rating, Durability; const char* SlotName; bool HasItem; };
inline ArmorInfo GetArmorSlot(AActor* a, EPrimalEquipmentType slot, const char* name) {
    ArmorInfo ai = {};
    ai.SlotName = name;
    auto* item = GetEquippedItem(a, slot);
    if (item) {
        ai.HasItem    = true;
        ai.Icon       = GetItemIcon(item);
        ai.Rating     = GetEquippedArmorRating(a, slot);
        ai.Durability = GetItemDurability(item);
    }
    return ai;
}

// ===================================================================
// movement replication (speed hack surface)
// ===================================================================
constexpr u64 FN_NotifyPlayerJoined = 0x3241c8c;
constexpr u64 FN_DownloadDungeon   = 0x7c2df8;
constexpr u64 FN_PlayerHasPrimalPass = 0x7cc6f4;

inline i32 GetNumPlayers(){ auto* gs=GetGameState(); if(!gs)return 0; auto fn=(i32(*)(void*))RVA(0x2c9bb00); return fn?fn(gs):0; }
inline void GetWorldOrigin(float& lat,float& lon){ auto* w=GetGWorld(); if(!w)return; i32* o=(i32*)((u64)w+0xB9C); lat=o[0]/100.f; lon=o[1]/100.f; }
inline AShooterWeapon* GetCurrentWeapon(AActor* a){ if(!Mem::Valid(a))return 0; return*(AShooterWeapon**)((u64)a+OFF::ASC_CurrentWeapon); }

constexpr auto AC_ControlRotation = 0x3A0;
inline void InstantTurn(float yaw){ auto* pc=GetLocalController(); if(!pc)return; ((FRotator*)((u64)pc+AC_ControlRotation))->Yaw=yaw; }

inline float GetFPS(){ static float f=0; float* gft=(float*)RVA(0x7781d1); if(!Mem::Valid(gft))return f; f=*gft>0.0001f?1.f/(*gft):60.f; return f; }

// ===================================================================
// function / property helpers
// ===================================================================

inline UFunction* FindFunction(const char* name){ return SDK::FindObject<UFunction>(name); }
inline void CallFunctionByName(UObject* obj,const char* fnName,void* params){ auto* fn=FindFunction(fnName); if(fn)obj->ProcessEvent(fn,params); }

inline bool CallRPC(UObject* obj, const char* fnName, void* params, i32) {
    auto* fn = FindFunction(fnName);
    if (!fn || !obj || !Mem::Valid(obj)) return false;
    obj->ProcessEvent(fn, params);
    return true;
}


struct ServerRespawnParams   { i32 PointIndex; bool bUseSpawnPoint; };
struct ServerResurrectParams { u32 ImplantNetID; };
struct ClientMessageParams   { FString Msg; FName Type; float Time; };
struct FireProjectileParams  { FVector Location; FVector_NetQuantizeNormal Direction; bool bClientOnly; };
struct ServerFireParams      { FVector Location; FVector_NetQuantizeNormal Direction; };

struct ServerMultiUseParams   { UObject* Target; i32 UseIndex; i32 Extra; bool bFlag1, bFlag2; };

enum EMultiUseIndex : i32 {
    MULTI_FORCE_OBTAIN = 0x7A,
    MULTI_DESTROY      = 0xC9,
    MULTI_CLAIM        = 0xCC,
};
constexpr auto PC_MultiUseExtra = 0xC28;

inline void ServerMultiUse(UObject* target, i32 useIdx, bool f1 = true, bool f2 = true) {
    auto* pc = GetLocalController();
    ServerMultiUseParams p = {target, useIdx, *(i32*)((u64)pc + PC_MultiUseExtra), f1, f2};
    CallRPC(pc, "ServerMultiUse", &p, sizeof(p));
}

struct ServerKillDinoParams { APrimalDinoCharacter* Dino; };
inline void ServerKillDino(APrimalDinoCharacter* dino) {
    ServerKillDinoParams p = {dino};
    CallRPC(GetLocalController(), "ServerKillDino", &p, sizeof(p));
}

inline void ServerSuicide() { CallRPC(GetLocalController(),"ServerSuicide",nullptr,0); }
inline void ServerRespawn() { ServerRespawnParams p={0,false}; CallRPC(GetLocalController(),"ServerRequestRespawnAtPoint",&p,sizeof(p)); }
inline void ServerAdminCmd(const char* cmd){ auto* pc=GetLocalController(); if(!pc)return; struct{FString Cmd;}p; char16_t b[256]; p.Cmd=FStringFromAnsi(cmd,b,256); CallRPC(pc,"ServerAdminManConsoleCommand",&p,sizeof(p)); }
inline void SetAdminIcon(bool on){ CallRPC(GetLocalController(),"SetAdminIcon",&on,1); }
inline void ServerResurrectDino(u32 id){ ServerResurrectParams p={id}; CallRPC(GetLocalPawn(),"ServerResurrectDino",&p,sizeof(p)); }
inline void ClientMessage(const char* msg,float time=5.f){ auto* pc=GetLocalController(); if(!pc)return; char16_t b[256]; ClientMessageParams p; p.Msg=FStringFromAnsi(msg,b,256); p.Type=FName_FromString("Engine"); p.Time=time; CallRPC(pc,"ClientMessage",&p,sizeof(p)); }
inline void FireProjectile(APrimalDinoCharacter* dino,FVector loc,FVector dir,bool clientOnly=false){ FireProjectileParams p={loc,{dir.X,dir.Y,dir.Z},clientOnly}; CallRPC(dino,"FireProjectile",&p,sizeof(p)); }
inline void ServerFireProjectile(AShooterWeapon_Projectile* wpn,FVector loc,FVector dir){ ServerFireParams p={loc,{dir.X,dir.Y,dir.Z}}; CallRPC(wpn,"ServerFireProjectile",&p,sizeof(p)); }

// ===================================================================
// structure placement / dupe
// ===================================================================

struct FItemNetID { u32 Id1, Id2; FItemNetID() : Id1(0), Id2(0) {} };

struct PlaceStructureParams {
    i32 ItemID; FVector Location, ExtraLocation; FRotator Rotation;
    APawn* Planter; APrimalDinoCharacter* RidingDino; FName SnapPoint;
    FItemNetID ItemNetID; void* SnapData;
    bool bSnapped, bValidated, bUsingMultiUse, bFromMultiUse; i32 UnlockLevel;
};

inline void PlaceStructure(const PlaceStructureParams& p){ auto* pc=GetLocalController(); auto* fn=FindFunction("ServerRequestPlaceStructure"); if(pc&&fn)pc->ProcessEvent(fn,(void*)&p); }
inline void DupeStructure(i32 itemID,FVector loc,FRotator rot,FItemNetID netID,APawn* planter=nullptr){ if(!planter)planter=GetLocalPawn(); PlaceStructureParams p={}; p.ItemID=itemID;p.Location=loc;p.Rotation=rot;p.Planter=planter;p.ItemNetID=netID; PlaceStructure(p); PlaceStructure(p); }
inline void PlaceFloating(i32 itemID,FVector loc,FRotator rot,FItemNetID netID,APawn* planter=nullptr){ if(!planter)planter=GetLocalPawn(); PlaceStructureParams p={}; p.ItemID=itemID;p.Location=loc;p.Rotation=rot;p.Planter=planter;p.ItemNetID=netID;p.bSnapped=false; PlaceStructure(p); }

} // namespace SDK

//funcs
namespace Fn {
    constexpr u64 EquipItem=0x2a50e54,GetEquippedItem=0x2a609d8,ConsumeArmor=0x2a662c8;
    constexpr u64 LocalUseItem=0x2a64f88,ServerUseItem=0x2a6742c;
    constexpr u64 DinoMount=0x29b3fb0,ClearMount=0x29b3e70;
    constexpr u64 TransferAll=0x2d67ab0,TransferStacks=0x2d67788;
    constexpr u64 GetRecoilMult=0x2c3b384,CanFire=0x2dba2ac;
    constexpr u64 StartFire=0x2da7758,SimInstantHit=0x2da9b54;
    constexpr u64 ServerNotify=0x2dace78,ServerWeaponFire=0x31f36d8;
    constexpr u64 ReplicateMove=0x48457f4,LineTrace=0x47fdfb0,Raycast=0x480dad8;
    constexpr u64 IsOfTribe=0x29b3af8,IsAlliedTeam=0x29c3348,CanTarget=0x2a1feb8;
    constexpr u64 GetBoneLocation=0x48bcadc,GetBoneIndex=0x48ba9e0,GetBoneName=0x48baaa0;
    constexpr u64 GetSocketLocation=0x48a3780,FillCompSpaceTransforms=0x48b3428;
    constexpr u64 UShooterGVpClient_PostRender=0x2cc7ef0;
    constexpr u64 UCanvas_K2_DrawLine=0x4e5a138,K2_DrawText=0x4e5a400,K2_DrawBox=0x4e5a888;
    constexpr u64 UCanvas_K2_DrawTexture=0x4e5a1c0,K2_DrawMaterial=0x4e5a298;
    constexpr u64 UCanvas_K2_DrawPolygon=0x4e5abd0,K2_DrawTriangle=0x4e5a914,K2_DrawMaterialTriangle=0x4e5aa7c;
    constexpr u64 UCanvas_SetDrawColor=0x4e57888,UCanvas_DrawText=0x4e58e3c;
    constexpr u64 UCanvas_StrLen=0x4e59168,UCanvas_DrawItem=0x4e59ef8;
    constexpr u64 StaticFindObject=0x365c628,StaticFindObjectFast=0x366a64c;
    constexpr u64 UObject_ProcessEvent=0x3648550,GetFullName=0x36569f8;
    constexpr u64 ProjectWorldToScreen=0x49938b0,StopAllCameraShakes=0x4c31c98;
    constexpr u64 ServerNotifyShot=0x3283d54,ServerNotifyShot_Impl=0x2da1d30;
    constexpr u64 FireProjectile=0x2fa8110;
    constexpr u64 SetFOV=0x4c2f234,UnlockFOV=0x4c2f23c,GetFOVAngle=0x4c2f220;
    constexpr u64 SetVectorParamInternal=0x4ab2ee8,CreateDynamicMaterialInstance=0x4880de8;
    constexpr u64 GetComponentByClass=0x4672864,FindComponentByClass=0x4672540;
    constexpr u64 GetViewportClient=0x2cba208,GetPlayerViewPoint=0x4c34514;
    constexpr u64 ClientMessage=0x52aebdc,FNameCtor=0x350425c;
    constexpr u64 ServerSuicide=0x31e4768,ServerRespawn=0x31e38cc;
    constexpr u64 ServerAdminCmd=0x31e0d84,SetAdminIcon=0x2d4a72c;
    constexpr u64 ServerResurrectDino=0x31446d4,GetCurrentWeapon=0x2bc8bac;
    constexpr u64 GetNumPlayers=0x2c929b4,GetWorldOriginLocation=0x4992eb4;
    constexpr u64 NotifyPlayerJoined=0x3241c8c,PlayerHasPrimalPass=0x7cc6f4;
    constexpr u64 DownloadDungeon=0x7c2df8,PlaceStructure=0x31e34d0;
    constexpr u64 CreateTransient=0x4debb90,BulkDataLock=0x3572c80,BulkDataUnlock=0x3572d7c;
}
