#pragma once
#include "pch.h"

struct handle_data
{
	unsigned long process_id;
	HWND window_handle;
};

namespace Input
{
	extern FVector2D MousePosition;
	extern bool KeyPressed[256];

	void Initialize();
}

