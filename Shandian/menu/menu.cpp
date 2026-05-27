#include "menu.hpp"

namespace Shandian {

void HandleTouch(float tx, float ty) {
    if (!g_TouchActive || g_TouchHandled || g_Dragging)
        return;

    float tabW = g_MenuW / 4.f;
    float tabY = g_MenuY + 28.f;
    for (i32 i = 0; i < 4; i++) {
        float ti = g_MenuX + i * tabW;
        if (InRect(tx, ty, ti, tabY, tabW - 2, 22)) {
            g_MenuTab      = i;
            g_TouchHandled = true;
            return;
        }
    }

    auto tog = [&](auto& cfg, i32& r) {
        if (InRect(tx, ty, g_MenuX + 10, g_MenuY + 56 + r * 22, 200, 20)) {
            cfg = !cfg;
            g_TouchHandled = true;
        }
    };
    auto sld = [&](float& val, float mn, float mx, i32& r) {
        float sx = g_MenuX + 90;
        float sy = g_MenuY + 68 + r * 22;
        float sw = 100;
        if (InRect(tx, ty, sx, sy - 4, sw, 12)) {
            float pct = (tx - sx) / sw;
            if (pct < 0) pct = 0;
            if (pct > 1) pct = 1;
            val = mn + pct * (mx - mn);
            g_TouchHandled = true;
        }
    };

    i32 r = 0;

    //general
    if (g_MenuTab == 0) {
        tog(g_Config.SpeedHack_Enabled, r); r++;
        sld(*(float*)&g_Config.SpeedHack_Level, 1, 10, r); r++;
        tog(g_Config.FOV_Enabled, r); r++;
        if (g_Config.FOV_Enabled) {
            sld(g_Config.FOV_Value, 30, 180, r); r++;
        }
        tog(g_Config.Fly_Enabled, r); r++;
        tog(g_Config.GhostMode_Enabled, r); r++;
        tog(g_Config.TimeOfDay_Enabled, r); r++;
        sld(g_Config.TimeOfDay_Value, 0, 24, r); r++;
        tog(g_Config.NoSkyWireframe, r); r++;
        tog(g_Config.NoKnockoutBlur, r); r++;
        tog(g_Config.Gamma_Enabled, r); r++;
        if (g_Config.Gamma_Enabled) {
            sld(g_Config.Gamma_Value, 0.5f, 5, r); r++;
        }
        tog(g_Config.UnlockFPS_Enabled, r); r++;
    }

    //esp
    else if (g_MenuTab == 1) {
        tog(g_Config.ESP_Players, r);       r++;
        tog(g_Config.ESP_Sleeping, r);      r++;
        tog(g_Config.ESP_DeadPlayers, r);   r++;
        tog(g_Config.ESP_Dinos, r);         r++;
        tog(g_Config.ESP_DinoHP, r);        r++;
        tog(g_Config.ESP_Structures, r);    r++;
        tog(g_Config.ESP_SupplyCrates, r);  r++;
        tog(g_Config.ESP_DroppedItems, r);  r++;
        tog(g_Config.ESP_Beds, r);          r++;
        tog(g_Config.ESP_Storage, r);       r++;
        tog(g_Config.ESP_ResourceMetal, r); r++;
        tog(g_Config.ESP_ResourceCrystal, r); r++;
        tog(g_Config.ESP_ResourceObsidian, r); r++;
        tog(g_Config.ESP_ResourceOil, r); r++;
        tog(g_Config.ESP_ResourcePearls, r); r++;
        tog(g_Config.ESP_ResourceStone, r); r++;
        tog(g_Config.ESP_PlayerNames, r);   r++;
        tog(g_Config.ESP_PlayerLines, r);   r++;
        tog(g_Config.ESP_DinoLines, r);     r++;
        tog(g_Config.ESP_Bones, r);         r++;
        tog(g_Config.ESP_HPBar, r);         r++;
        tog(g_Config.ESP_Level, r);         r++;
        tog(g_Config.ESP_Distance, r);      r++;
        sld(g_Config.ESP_MaxDist, 1000, 50000, r); r++;
    }

    //pvp
    else if (g_MenuTab == 2) {
        tog(g_Config.Aimbot_Enabled, r); r++;
        if (g_Config.Aimbot_Enabled) {
            sld(g_Config.Aimbot_FOV, 5, 360, r);    r++;
            sld(g_Config.Aimbot_Smooth, 1, 20, r);  r++;
        }
        tog(g_Config.SilentAim_Enabled, r);       r++;
        tog(g_Config.CrashEnemy_Enabled, r);       r++;
        tog(g_Config.SlowDinos_Enabled, r);        r++;
        tog(g_Config.FakeAdmin_Enabled, r);        r++;
        tog(g_Config.AutoFire_Enabled, r);         r++;
        tog(g_Config.AutoReload_Enabled, r);       r++;
        tog(g_Config.EnemyChams_Enabled, r);       r++;
        tog(g_Config.TribeChams_Enabled, r);       r++;
        tog(g_Config.AllyChams_Enabled, r);        r++;
        tog(g_Config.DinoChams_Enabled, r);        r++;
        tog(g_Config.EnemyWireframe_Enabled, r);   r++;
        tog(g_Config.NoRecoil_Enabled, r);         r++;
        tog(g_Config.NoGunShake_Enabled, r);      r++;
        tog(g_Config.NoScopeZoom_Enabled, r);     r++;
        tog(g_Config.NoStrafe_Enabled, r);         r++;
        tog(g_Config.RunWhileFire_Enabled, r);     r++;
        tog(g_Config.UnderwaterShoot_Enabled, r);  r++;
    }

    //misc
    else if (g_MenuTab == 3) {
        tog(g_Config.AutoArmor_Enabled, r);        r++;
        tog(g_Config.AutoBrew_Enabled, r);         r++;
        tog(g_Config.AutoStamina_Enabled, r);      r++;
        tog(g_Config.AutoFeed_Enabled, r);         r++;
        tog(g_Config.AutoMount_Enabled, r);        r++;
        tog(g_Config.AutoLoot_Enabled, r);         r++;
        tog(g_Config.AutoPickup_Enabled, r);       r++;
        tog(g_Config.UnlockNotes_Enabled, r);      r++;
        tog(g_Config.DestroyStructs_Enabled, r);   r++;
        tog(g_Config.DestroyOnlyOnScreen, r);      r++;
        tog(g_Config.ForceObtain_Enabled, r);      r++;
        tog(g_Config.ClaimDinos_Enabled, r);       r++;
        tog(g_Config.ServerCrash_Enabled, r);      r++;
        tog(g_Config.Minimap_Enabled, r);          r++;
        tog(g_Config.Minimap_Players, r);          r++;
        tog(g_Config.Minimap_Dinos, r);            r++;
        tog(g_Config.Minimap_Turrets, r);          r++;
    }
}

void DrawCheckbox(UCanvas* c, const char* label, bool* val,
                   float& y, float x) {
    // checkbox border
    SDK::K2_DrawBox(c, {x, y + 8}, {14, 14}, 1,
                    {0.3f, 0.25f, 0.2f, 1});
    // filled indicator
    if (*val)
        SDK::K2_DrawBox(c, {x + 2, y + 10}, {10, 10}, 0,
                        g_Config.Menu_AccentCol);
    // label
    SDK::K2_DrawText(c, nullptr, label,
                     {x + 20, y + 6},
                     {0.85f, 0.8f, 0.75f, 1}, 0.75f);
    y += 22;
}

void DrawSlider(UCanvas* c, const char* label, float* val,
                float min, float max, float& y, float x) {
    // label
    SDK::K2_DrawText(c, nullptr, label,
                     {x, y + 4},
                     {0.7f, 0.65f, 0.6f, 1}, 0.7f);

    float tw  = 100;
    float tx  = x + 80;
    float pct = (*val - min) / (max - min);

    // track
    SDK::K2_DrawBox(c, {tx, y + 14}, {tw, 3}, 0,
                    {0.15f, 0.12f, 0.1f, 1});
    // fill
    SDK::K2_DrawBox(c, {tx, y + 14}, {tw * pct, 3}, 0,
                    g_Config.Menu_AccentCol);

    // value text
    char b[16];
    snprintf(b, 16, "%.1f", *val);
    SDK::K2_DrawText(c, nullptr, b,
                     {tx + tw + 6, y + 4},
                     {0.85f, 0.8f, 0.75f, 1}, 0.7f);
    y += 22;
}

void DrawMenu(UCanvas* canvas) {
    if (!g_MenuOpen || !Mem::Valid(canvas))
        return;

    g_CanvasW = (float)canvas->SizeX;
    g_CanvasH = (float)canvas->SizeY;

    float x  = g_MenuX;
    float y  = g_MenuY;
    float mh = 520;

    //bg
    SDK::K2_DrawBox(canvas, {x - 5, y - 5}, {g_MenuW + 10, mh}, 0,
                    g_Config.Menu_BgCol);
    SDK::K2_DrawBox(canvas, {x - 7, y - 7}, {g_MenuW + 14, 2}, 0,
                    g_Config.Menu_AccentCol);
    SDK::K2_DrawBox(canvas, {x - 7, y + mh - 7}, {g_MenuW + 14, 2}, 0,
                    g_Config.Menu_AccentCol);
    SDK::K2_DrawBox(canvas, {x - 5, y - 5}, {g_MenuW + 10, 28}, 0,
                    {0.06f, 0.05f, 0.04f, 0.95f});

    //title
    SDK::K2_DrawText(canvas, nullptr, "SHANDIAN",
                     {x + g_MenuW * 0.5f - 35, y + 2},
                     g_Config.Menu_AccentCol, 1.3f);
    y += 24;

    //tabs
    float tw = g_MenuW / 4;
    for (i32 i = 0; i < 4; i++) {
        FLinearColor tc = (i == g_MenuTab)
            ? g_Config.Menu_AccentCol
            : FLinearColor{0.4f, 0.35f, 0.3f, 1};
        SDK::K2_DrawText(canvas, nullptr, g_Tabs[i],
                         {x + i * tw + 10, y + 2}, tc, 0.8f);
        if (i == g_MenuTab)
            SDK::K2_DrawBox(canvas, {x + i * tw + 8, y + 20},
                            {tw - 16, 2}, 0, g_Config.Menu_AccentCol);
    }
    y += 24;
    float cx = x + 10;


    if (g_MenuTab == 0) {
        DrawCheckbox(canvas, "Speed Hack", &g_Config.SpeedHack_Enabled, y, cx);
        static float sp;
        sp = (float)g_Config.SpeedHack_Level;
        DrawSlider(canvas, "Speed 1-10", &sp, 1, 10, y, cx);
        g_Config.SpeedHack_Level = (i32)sp;

        DrawCheckbox(canvas, "FOV", &g_Config.FOV_Enabled, y, cx);
        if (g_Config.FOV_Enabled)
            DrawSlider(canvas, "FOV Angle", &g_Config.FOV_Value, 30, 180, y, cx);
        //wide view
        DrawCheckbox(canvas, "Peripheral", &g_Config.Peripheral_Enabled, y, cx);
        if (g_Config.Peripheral_Enabled)
            DrawSlider(canvas, "Peripheral", &g_Config.Peripheral_Value, 90, 180, y, cx);

        DrawCheckbox(canvas, "Fly Mode",  &g_Config.Fly_Enabled,       y, cx);
        DrawCheckbox(canvas, "Ghost Mode", &g_Config.GhostMode_Enabled, y, cx);
        DrawCheckbox(canvas, "Time of Day", &g_Config.TimeOfDay_Enabled, y, cx);

        static float tod;
        tod = g_Config.TimeOfDay_Value;
        DrawSlider(canvas, "Hour", &tod, 0, 24, y, cx);
        g_Config.TimeOfDay_Value = tod;

        DrawCheckbox(canvas, "Sky Wireframe", &g_Config.NoSkyWireframe, y, cx);
        DrawCheckbox(canvas, "No KO Blur",    &g_Config.NoKnockoutBlur, y, cx);
        DrawCheckbox(canvas, "Gamma",         &g_Config.Gamma_Enabled,  y, cx);
        if (g_Config.Gamma_Enabled)
            DrawSlider(canvas, "Gamma", &g_Config.Gamma_Value, 0.5f, 5, y, cx);

        DrawCheckbox(canvas, "Unlock FPS", &g_Config.UnlockFPS_Enabled, y, cx);
    }

    else if (g_MenuTab == 1) {
        DrawCheckbox(canvas, "Players",       &g_Config.ESP_Players,       y, cx);
        DrawCheckbox(canvas, "Sleeping",      &g_Config.ESP_Sleeping,      y, cx);
        DrawCheckbox(canvas, "Dead",          &g_Config.ESP_DeadPlayers,   y, cx);
        DrawCheckbox(canvas, "Dinos",         &g_Config.ESP_Dinos,         y, cx);
        DrawCheckbox(canvas, "Structures",    &g_Config.ESP_Structures,    y, cx);
        DrawCheckbox(canvas, "Supply Crates", &g_Config.ESP_SupplyCrates,  y, cx);
        DrawCheckbox(canvas, "Drops/Eggs",    &g_Config.ESP_DroppedItems,  y, cx);
        DrawCheckbox(canvas, "Beds",          &g_Config.ESP_Beds,          y, cx);
        DrawCheckbox(canvas, "Storage",       &g_Config.ESP_Storage,       y, cx);
        DrawCheckbox(canvas, "Metal",         &g_Config.ESP_ResourceMetal, y, cx);
        DrawCheckbox(canvas, "Crystal",       &g_Config.ESP_ResourceCrystal, y, cx);
        DrawCheckbox(canvas, "Obsidian",      &g_Config.ESP_ResourceObsidian, y, cx);
        DrawCheckbox(canvas, "Oil",           &g_Config.ESP_ResourceOil,    y, cx);
        DrawCheckbox(canvas, "Pearls",        &g_Config.ESP_ResourcePearls, y, cx);
        DrawCheckbox(canvas, "Stone",         &g_Config.ESP_ResourceStone,  y, cx);
        DrawCheckbox(canvas, "Player Names",  &g_Config.ESP_PlayerNames,   y, cx);
        DrawCheckbox(canvas, "Player Lines",  &g_Config.ESP_PlayerLines,   y, cx);
        DrawCheckbox(canvas, "Dino Lines",    &g_Config.ESP_DinoLines,     y, cx);
        DrawCheckbox(canvas, "Skeleton",      &g_Config.ESP_Bones,         y, cx);
        DrawCheckbox(canvas, "HP Bar",        &g_Config.ESP_HPBar,         y, cx);
        DrawCheckbox(canvas, "Level",         &g_Config.ESP_Level,         y, cx);
        DrawCheckbox(canvas, "Distance",      &g_Config.ESP_Distance,      y, cx);
        DrawCheckbox(canvas, "Dino HP",       &g_Config.ESP_DinoHP,        y, cx);
        DrawCheckbox(canvas, "Armor/Weps",    &g_Config.ESP_ArmorWeapons,  y, cx);
        DrawCheckbox(canvas, "Enemy Count",   &g_Config.ESP_EnemyCount,    y, cx);
        DrawSlider(canvas, "Max Dist", &g_Config.ESP_MaxDist, 1000, 50000, y, cx);
    }

    else if (g_MenuTab == 2) {
        DrawCheckbox(canvas, "Aimbot", &g_Config.Aimbot_Enabled, y, cx);
        if (g_Config.Aimbot_Enabled) {
            DrawSlider(canvas, "FOV",    &g_Config.Aimbot_FOV,    5, 360, y, cx);
            DrawSlider(canvas, "Smooth", &g_Config.Aimbot_Smooth, 1, 20,  y, cx);
            DrawCheckbox(canvas, "Visible Only", &g_Config.Aimbot_VisibleOnly, y, cx);
        }
        DrawCheckbox(canvas, "Silent Aim",   &g_Config.SilentAim_Enabled,    y, cx);
        DrawCheckbox(canvas, "Crash Enemy",   &g_Config.CrashEnemy_Enabled,   y, cx);
        DrawCheckbox(canvas, "Slow Dinos",     &g_Config.SlowDinos_Enabled,    y, cx);
        DrawCheckbox(canvas, "Fake Admin",    &g_Config.FakeAdmin_Enabled,   y, cx);
        DrawCheckbox(canvas, "Auto Fire",     &g_Config.AutoFire_Enabled,      y, cx);
        DrawCheckbox(canvas, "Auto Reload",   &g_Config.AutoReload_Enabled,    y, cx);
        DrawCheckbox(canvas, "Enemy Chams",   &g_Config.EnemyChams_Enabled,    y, cx);
        DrawCheckbox(canvas, "Tribe Chams",   &g_Config.TribeChams_Enabled,    y, cx);
        DrawCheckbox(canvas, "Ally Chams",    &g_Config.AllyChams_Enabled,     y, cx);
        DrawCheckbox(canvas, "Dino Chams",    &g_Config.DinoChams_Enabled,     y, cx);
        DrawCheckbox(canvas, "Enemy Wireframe", &g_Config.EnemyWireframe_Enabled, y, cx);
        DrawCheckbox(canvas, "No Recoil",     &g_Config.NoRecoil_Enabled,       y, cx);
        DrawCheckbox(canvas, "No Gun Shake",  &g_Config.NoGunShake_Enabled,    y, cx);
        DrawCheckbox(canvas, "No Scope Zoom", &g_Config.NoScopeZoom_Enabled,   y, cx);
        DrawCheckbox(canvas, "No Strafe",     &g_Config.NoStrafe_Enabled,       y, cx);
        DrawCheckbox(canvas, "Shoot Mt'd",     &g_Config.RunWhileFire_Enabled,   y, cx);//also allows shoot while mounted on any dino
        DrawCheckbox(canvas, "Underwater Shoot", &g_Config.UnderwaterShoot_Enabled, y, cx);
    }

    else if (g_MenuTab == 3) {
        DrawCheckbox(canvas, "Auto Armor",    &g_Config.AutoArmor_Enabled,    y, cx);
        DrawCheckbox(canvas, "Auto Brew",     &g_Config.AutoBrew_Enabled,     y, cx);
        DrawCheckbox(canvas, "Auto Stamina",  &g_Config.AutoStamina_Enabled,  y, cx);
        DrawCheckbox(canvas, "Auto Feed",     &g_Config.AutoFeed_Enabled,     y, cx);
        DrawCheckbox(canvas, "Auto Mount",    &g_Config.AutoMount_Enabled,    y, cx);
        DrawCheckbox(canvas, "Auto Loot",     &g_Config.AutoLoot_Enabled,     y, cx);
        DrawCheckbox(canvas, "Auto Pickup",   &g_Config.AutoPickup_Enabled,   y, cx);
        DrawCheckbox(canvas, "Unlock Notes",  &g_Config.UnlockNotes_Enabled,  y, cx);
        DrawCheckbox(canvas, "Force Obtain",  &g_Config.ForceObtain_Enabled,  y, cx);
        DrawCheckbox(canvas, "On Screen Only", &g_Config.DestroyOnlyOnScreen,  y, cx);
        DrawCheckbox(canvas, "Destroy Structs", &g_Config.DestroyStructs_Enabled, y, cx);
        DrawCheckbox(canvas, "Claim Dinos",   &g_Config.ClaimDinos_Enabled,     y, cx);
        DrawCheckbox(canvas, "Server Crash",  &g_Config.ServerCrash_Enabled,    y, cx);
        DrawCheckbox(canvas, "Turret Fill",   &g_Config.TurretFill_Enabled,   y, cx);

        static float tfa;
        tfa = (float)g_Config.TurretFill_Amount;
        DrawSlider(canvas, "Ammo Amt", &tfa, 1, 500, y, cx);
        g_Config.TurretFill_Amount = (i32)tfa;

        DrawCheckbox(canvas, "Turret Pin",    &g_Config.TurretPin_Enabled,    y, cx);

        static float tpv;
        tpv = (float)g_Config.TurretPin_Value;
        DrawSlider(canvas, "Pin Code", &tpv, 0, 9999, y, cx);
        g_Config.TurretPin_Value = (i32)tpv;
        DrawCheckbox(canvas, "Turret Active", &g_Config.TurretActivate_Enabled, y, cx);
        DrawCheckbox(canvas, "Minimap",       &g_Config.Minimap_Enabled,       y, cx);
        DrawCheckbox(canvas, "  Players",     &g_Config.Minimap_Players,       y, cx);
        DrawCheckbox(canvas, "  Dinos",       &g_Config.Minimap_Dinos,         y, cx);
        DrawCheckbox(canvas, "  Turrets",     &g_Config.Minimap_Turrets,       y, cx);
    }
}

}
