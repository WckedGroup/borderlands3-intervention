#include "pch.h"

void Cursor::Render(UCanvas* canvas)
{
	auto pos1 = ColorVector2D(this->Location, Black);
	auto pos2 = ColorVector2D(this->Location.X, this->Location.Y + 20.0f, Black);
	auto pos3 = ColorVector2D(this->Location.X + 17.0f, this->Location.Y + 14.0f, Black);

	auto outlinePos1 = ColorVector2D(this->Location.X, this->Location.Y-2.0f, ACCENT_COLOR);
	auto outlinePos2 = ColorVector2D(this->Location.X - 2.0f, this->Location.Y + 22.0f, ACCENT_COLOR);
	auto outlinePos3 = ColorVector2D(this->Location.X + 19.0f, this->Location.Y + 16.0f, ACCENT_COLOR);

	canvas->UDrawTriangle(outlinePos1, outlinePos2, outlinePos3);
	canvas->UDrawTriangle(pos1, pos2, pos3);
}

void Cursor::Update(float x, float y)
{
	this->Location = { x, y };
}