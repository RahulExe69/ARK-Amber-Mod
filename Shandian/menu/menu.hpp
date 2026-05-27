#pragma once
#include "../cheat/config.hpp"

namespace Shandian {

//globals
inline bool g_MenuOpen = true;
inline i32  g_MenuTab  = 0;
inline float g_MenuX = 50, g_MenuY = 80, g_MenuW = 260;
static const char* g_Tabs[] = {"General", "ESP", "PvP", "Misc"};

//colour helpers
inline FLinearColor HSVtoRGB(float h, float s, float v) {
    h = fmodf(h, 360.f);
    if (h < 0) h += 360;
    float c = v * s;
    float x = c * (1 - fabsf(fmodf(h / 60, 2) - 1));
    float m = v - c;
    float r = 0, g = 0, b = 0;
    if      (h < 60)  { r = c; g = x; }
    else if (h < 120) { r = x; g = c; }
    else if (h < 180) { g = c; b = x; }
    else if (h < 240) { g = x; b = c; }
    else if (h < 300) { r = x; b = c; }
    else              { r = c; b = x; }
    return {r + m, g + m, b + m, 1};
}

inline bool InRect(float px, float py,
                   float rx, float ry,
                   float rw, float rh) {
    return px >= rx && px <= rx + rw &&
           py >= ry && py <= ry + rh;
}

// draw funcs
void HandleTouch(float tx, float ty);
void DrawCheckbox(UCanvas* c, const char* label, bool* val,
                  float& y, float x);
void DrawSlider(UCanvas* c, const char* label, float* val,
                float min, float max, float& y, float x);
void DrawMenu(UCanvas* canvas);

}
