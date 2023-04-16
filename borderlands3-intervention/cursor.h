#pragma once
#include "pch.h"

class Cursor
{
public:
	FVector2D Location;

	void Render(UCanvas* canvas);
	void Update(float x, float y);
};