#include "esp.hpp"
#include "config.hpp"

namespace Shandian {

void DrawESP(UCanvas* canvas) {
    if (!Mem::Valid(canvas)) return;

    auto* lp = SDK::GetLocalPawn();
    FVector lpPos = {};
    if (lp && lp->RootComponent)
        lpPos = ((USceneComponent*)lp->RootComponent)->RelLocation;

    FVector2D lpScreen;
    bool lpOnScreen = SDK::WorldToScreen(lpPos, &lpScreen);

    for (auto& t : g_ESPTargets) {
        float h = fabsf(t.ScreenPos.Y - t.HeadScreen.Y);
        float w = h * 0.4f;
        if (w < 2.f) w = 2.f;
        float x = t.ScreenPos.X - w * 0.5f;
        float y = t.ScreenPos.Y - h;

        // Color: default by type, then tribe override
        FLinearColor col = t.IsPlayer   ? g_Config.ESP_EnemyCol :
                           t.IsDino     ? g_Config.ESP_DinoCol :
                           t.IsStructure ? g_Config.ESP_StructureCol :
                                           g_Config.ESP_SupplyCol;

        if (t.IsPlayer) {
            i32 tribe = SDK::GetTribeID(t.Actor);
            if (tribe > 0 && tribe == SDK::GetLocalTribeID())
                col = g_Config.ESP_TribeCol;
        }

        // Box outline
        SDK::K2_DrawBox(canvas, {x - 1, y - 1}, {w + 2, h + 2}, 1.f, {0, 0, 0, 0.8f});
        SDK::K2_DrawBox(canvas, {x, y}, {w, h}, 1.f, col);

        // HP bar
        if (g_Config.ESP_HPBar && t.HP > 0) {
            float barW = w + 4;
            float maxHP = t.IsDino ? 10000.f : 1000.f;
            SDK::K2_DrawBox(canvas, {x - 2, y - 6}, {barW, 3}, 0.f,
                            {0, 0, 0, 0.7f});
            SDK::K2_DrawBox(canvas, {x - 2, y - 6},
                            {barW * fminf(t.HP / maxHP, 1.f), 3}, 0.f,
                            {0, 1, 0, 1});
        }

        // Text labels
        char buf[64];
        if (g_Config.ESP_Distance) {
            snprintf(buf, 64, "%.0fm", t.Dist / 100.f);
            SDK::K2_DrawText(canvas, nullptr, buf,
                             {x + w + 2, y + 2}, {1, 1, 1, 1}, 0.7f);
        }
        if (g_Config.ESP_Level && t.Level > 0) {
            snprintf(buf, 64, "Lv%d", t.Level);
            SDK::K2_DrawText(canvas, nullptr, buf,
                             {x, y - 10}, {1, 1, 0, 1}, 0.7f);
        }
        if (g_Config.ESP_DinoHP && t.IsDino) {
            snprintf(buf, 64, "%.0f", t.HP);
            SDK::K2_DrawText(canvas, nullptr, buf,
                             {x + w + 2, y + 12}, {1, 1, 1, 1}, 0.7f);
        }

        // Lines from screen center to target
        if (g_Config.ESP_PlayerLines && t.IsPlayer && lpOnScreen) {
            SDK::K2_DrawLine(canvas,
                {(float)canvas->SizeX * 0.5f, 6.f},
                t.ScreenPos, 1.f, {1, 0, 0, 0.5f});
        }
        if (g_Config.ESP_DinoLines && t.IsDino && lpOnScreen) {
            SDK::K2_DrawLine(canvas,
                {(float)canvas->SizeX * 0.5f, 6.f},
                t.ScreenPos, 1.f, {0, 0.8f, 1, 0.5f});
        }

        // bones esp
        if (g_Config.ESP_Bones && t.IsPlayer && t.Actor) {
            void* mesh = *(void**)((u64)t.Actor + OFF::AC_Mesh);
            if (Mem::Valid(mesh)) {
                auto* transforms = *(FTransform**)((u64)mesh + 0x980);
                i32 boneCount    = *(i32*)((u64)mesh + 0x9C8);
                if (transforms && boneCount > 0) {
                    auto cam = SDK::GetCamInfo();
                    auto drawBone = [&](i32 a, i32 b) {
                        if (a < boneCount && b < boneCount) {
                            FVector2D sa, sb;
                            if (SDK::WorldToScreen(transforms[a].Trans, &sa) &&
                                SDK::WorldToScreen(transforms[b].Trans, &sb))
                                SDK::K2_DrawLine(canvas, sa, sb, 1.f,
                                                 {1, 1, 1, 0.7f});
                        }
                    };
                    // spine
                    drawBone(9, 6);
                    drawBone(27, 6);
                    drawBone(6, 5);
                    drawBone(5, 4);
                    // left arm
                    drawBone(33, 36);
                    drawBone(36, 38);
                    // right arm
                    drawBone(59, 62);
                    drawBone(62, 64);
                    // left leg
                    drawBone(81, 82);
                    drawBone(82, 84);
                    // right leg
                    drawBone(87, 90);
                }
            }
        }

        // Armor / weapon icons
        if (g_Config.ESP_ArmorWeapons && t.IsPlayer && t.Actor) {
            auto* inv = SDK::GetInventory(t.Actor);
            if (inv && Mem::Valid(inv)) {
                //weapon name
                auto* wpn = SDK::GetCurrentWeapon(t.Actor);
                if (wpn && wpn->ClassPrivate) {
                    const char* wn = ((UObject*)wpn->ClassPrivate)->NameCStr();
                    if (wn) SDK::K2_DrawText(canvas, nullptr, wn,
                                             {x - 16, y - 16},
                                             {1, 1, 1, 1}, 0.55f);
                }

                // Armor icons + durability
                auto* equipData = *(TArray<void*>**)((u64)inv + 0x228);
                i32 equipCount  = *(i32*)((u64)inv + 0x230);
                float ax = x + w + 4.f;
                float ay = y;
                for (i32 s = 0; equipData && s < equipCount && s < 6; s++) {
                    auto* item = (void*)equipData->Data[s];
                    if (!Mem::Valid(item)) continue;
                    auto* icon = SDK::GetItemIcon((UPrimalItem*)item);
                    if (icon)
                        SDK::K2_DrawTexture(canvas, icon, {ax, ay}, {16, 16},
                                            {0, 0}, {1, 1}, {1, 1, 1, 1});
                    float dur = SDK::GetItemDurability((UPrimalItem*)item);
                    char db[8];
                    snprintf(db, 8, "%.0f%%", dur * 100.f);
                    SDK::K2_DrawText(canvas, nullptr, db,
                                     {ax + 18, ay + 2},
                                     {0.8f, 0.8f, 0.8f, 1}, 0.45f);
                    ay += 18.f;
                }
            }
        }
    }

    // Resource ESP
    for (auto& r : g_ResourceTargets) {
        SDK::K2_DrawText(canvas, nullptr, r.Label, r.Screen, r.Color, 0.6f);
        SDK::K2_DrawBox(canvas,
                        {r.Screen.X - 2, r.Screen.Y - 2}, {4, 4},
                        0.f, r.Color);
    }
}

}
