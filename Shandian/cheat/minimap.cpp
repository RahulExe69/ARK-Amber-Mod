#include "minimap.hpp"
#include "config.hpp"
#include "globals.hpp"
#include "map_data.hpp"

namespace Shandian {

static UTexture2D* g_MapTex = nullptr;
static bool g_MapTried = false;

void EnsureMapTexture() {
    if (g_MapTried) return;
    g_MapTried = true;
    if (!g_MapSize) return;

    u8* rgba = (u8*)SDK::FMalloc(g_MapW * g_MapH * 4);
    if (!rgba) return;

    //zlib->rgba decompress
    using UnzFn = bool(*)(void*,int,const void*,int,unsigned int);
    auto unz = (UnzFn)RVA(0x347d440);
    if (!unz || !unz(rgba, g_MapW * g_MapH * 4,
                       g_MapData, g_MapSize, 15)) {
        SDK::FFree(rgba);
        return;
    }

    auto* tex = CreateTransientTexture(g_MapW, g_MapH, 2);
    if (!tex) { SDK::FFree(rgba); return; }
    tex->SRGB = 1;
    tex->CompressionSettings = 16;
    tex->Flags1 |= 2u;
    if (UploadTextureRGBA(tex, rgba, g_MapW, g_MapH))
        g_MapTex = tex;
    SDK::FFree(rgba);
}
//from ct
static void WorldToMap(FVector world, float& mx, float& my) {
    auto* ws = SDK::GetWorldSettings();
    if (!ws || !Mem::Valid(ws)) { mx = 0; my = 0; return; }
    float originX = *(float*)((u64)ws + 0xa54);
    float originY = *(float*)((u64)ws + 0xa58);
    float scaleX  = *(float*)((u64)ws + 0xa4c);
    float scaleY  = *(float*)((u64)ws + 0xa50);
    mx = (world.X - originX) / (10.f * scaleX);
    my = (world.Y - originY) / (10.f * scaleY);
}
static void MapDeltaToPixel(float mapDX, float mapDY,
                             float halfSize, float range,
                             float& px, float& py) {
    float s = halfSize / range;
    px = mapDX * s * 20.48f;
    py = mapDY * s * 20.48f;
    float dist = sqrtf(px * px + py * py);
    if (dist > halfSize) {
        px = (px / dist) * halfSize;
        py = (py / dist) * halfSize;
    }
}

void DrawMinimap(UCanvas* canvas) {
    if (!g_Config.Minimap_Enabled) return;
    auto& G = CachedGlobals::Get();
    if (!G.LocalPawn || !G.LocalPawn->RootComponent) return;

    // lazy-init map texture on first draw
    if (!g_MapTex) EnsureMapTexture();

    float cx = (float)canvas->SizeX - 110;
    float cy = 120;
    float size = 180;
    float half = size * 0.5f;
    float range = 30.f;

    //bg circle
    for (i32 a = 0; a < 64; a++) {
        float r1 = a * 3.14159f / 32.f;
        float r2 = (a + 1) * 3.14159f / 32.f;
        SDK::K2_DrawBox(canvas,
            {cx + half + cosf(r1) * half,
             cy + half + sinf(r1) * half},
            {cosf(r2) * half - cosf(r1) * half,
             sinf(r2) * half - sinf(r1) * half},
            1.f, {0.02f, 0.02f, 0.02f, 0.7f});
    }

    // player map coords
    FVector pp = G.MyPos;
    float pmx, pmy;
    WorldToMap(pp, pmx, pmy);

    //map texture overlay
    if (g_MapTex && Mem::Valid(g_MapTex)) {
        float uvScale = range / 100.f;
        float uvX = (pmx / 100.f) - uvScale;
        float uvY = (pmy / 100.f) - uvScale;
        SDK::K2_DrawTexture(canvas, g_MapTex,
            {cx, cy}, {size, size},
            {uvX, uvY},
            {uvX + uvScale * 2.f, uvY + uvScale * 2.f},
            {1, 1, 1, 0.85f});
    }

    // border

    // border
    SDK::K2_DrawBox(canvas, {cx - 1, cy - 1}, {size + 2, size + 2},
                    1.f, {0.15f, 0.15f, 0.15f, 0.6f});

    // cardinal lines + N label
    SDK::K2_DrawLine(canvas, {cx + half, cy}, {cx + half, cy + size},
                     0.5f, {0.1f, 0.1f, 0.1f, 0.3f});
    SDK::K2_DrawLine(canvas, {cx, cy + half}, {cx + size, cy + half},
                     0.5f, {0.1f, 0.1f, 0.1f, 0.3f});
    SDK::K2_DrawText(canvas, nullptr, "N", {cx + half - 6, cy + 4},
                     {0.5f, 0.5f, 0.5f, 1}, 0.6f);

    // player arrow in cam direction
    float yawRad = G.Cam.Rot.Yaw * 3.14159f / 180.f;
    float ax = cx + half + cosf(yawRad) * 8;
    float ay = cy + half + sinf(yawRad) * 8;
    float bx = cx + half + cosf(yawRad - 2.5f) * 5;
    float by = cy + half + sinf(yawRad - 2.5f) * 5;
    float dx = cx + half + cosf(yawRad + 2.5f) * 5;
    float dy = cy + half + sinf(yawRad + 2.5f) * 5;
    SDK::K2_DrawLine(canvas, {ax, ay}, {bx, by}, 1.5f, {0, 1, 0, 1});
    SDK::K2_DrawLine(canvas, {ax, ay}, {dx, dy}, 1.5f, {0, 1, 0, 1});
    SDK::K2_DrawLine(canvas, {bx, by}, {dx, dy}, 1.5f, {0, 1, 0, 1});

    // enemy markers
    if (g_Config.Minimap_Players) {
        auto& actors = SDK::GetActors();
        for (i32 i = 0; i < actors.Count; i++) {
            auto* a = actors[i];
            if (!Mem::Valid(a) || !a->ClassPrivate) continue;
            if (!a->RootComponent) continue;
            if (a == G.LocalPawn) continue;

            bool isPlayer = IsClass(a, G.CI_Character) &&
                           !a->IsA(G.Class_DinoCharacter);
            bool isDino   = a->IsA(G.Class_DinoCharacter);
            bool isTurret = a->IsA(G.Class_Turret);

            if (!isPlayer && !isDino && !isTurret) continue;
            if ((isDino   && !g_Config.Minimap_Dinos)   ||
                (isTurret && !g_Config.Minimap_Turrets)) continue;

            FVector wp = ((USceneComponent*)a->RootComponent)
                             ->RelLocation;
            float mx, my;
            WorldToMap(wp, mx, my);
            float dd = sqrtf((mx - pmx) * (mx - pmx) +
                             (my - pmy) * (my - pmy));
            if (dd > range * 1.5f) continue;

            float px, py;
            MapDeltaToPixel(mx - pmx, my - pmy, half, range, px, py);
            float sx = cx + half + px;
            float sy = cy + half + py;

            if (isPlayer) {
                i32 tribe = SDK::GetTribeID(a);
                FLinearColor col = (tribe > 0 && tribe == G.LocalTribe)
                    ? FLinearColor{0, 1, 0, 1}
                    : FLinearColor{1, 0, 0, 1};
                SDK::K2_DrawBox(canvas, {sx - 3, sy - 3}, {6, 6},
                                0.f, col);
            } else if (isDino) {
                SDK::K2_DrawBox(canvas, {sx - 2, sy - 2}, {4, 4},
                                0.f, {0, 0.8f, 1, 1});
            } else if (isTurret) {
                SDK::K2_DrawLine(canvas, {sx - 3, sy - 3},
                                 {sx + 3, sy + 3}, 1.f,
                                 {1, 0.5f, 0, 1});
                SDK::K2_DrawLine(canvas, {sx + 3, sy - 3},
                                 {sx - 3, sy + 3}, 1.f,
                                 {1, 0.5f, 0, 1});
            }
        }
    }

    char b[32];
    snprintf(b, 32, "%.0fm", range * 100.f);
    SDK::K2_DrawText(canvas, nullptr, b,
                     {cx + 4, cy + size - 14},
                     {0.4f, 0.4f, 0.4f, 1}, 0.5f);
}

}
