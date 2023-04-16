#pragma once
#include "pch.h"

typedef void(__thiscall* PostRenderOriginal_t)(UGameViewportClient*, UCanvas*);
extern PostRenderOriginal_t PostRenderOriginal;
extern Toggle* toggle;

void PostRenderHook(UGameViewportClient* viewport, UCanvas* canvas);

extern std::function<void(bool)> flyModeCallback;
extern std::function<void(bool)> godModeCallback;
extern std::function<void(bool)> showMenuCallback;