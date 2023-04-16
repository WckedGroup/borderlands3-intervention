#pragma once
#include "pch.h"

class Component
{
public:
	FVector2D Location { 0.0f, 0.0f };
	FVector2D BaseLocation { 0.0f, 0.0f };
	FVector2D Size { 0.0f, 0.0f };
	FVector2D RelativeDraggingLocation{ 0.0f, 0.0f };
	FVector2D TotalSize;

	FLinearColor BackgroundColor = BACKGROUND_COLOR;
	FLinearColor BorderColor = BORDER_COLOR;
	FLinearColor TextColor = TEXT_COLOR;

	Component* Parent;

	Component(FVector2D location);
	Component(FVector2D location, FVector2D size, bool visible = true);

	bool Visible = true;
	bool MouseOver = false;
	bool Dragging = false;

	bool CursorInsideArea();
	bool CursorInsideArea(FVector2D location, FVector2D size);
	bool CursorInsideCircle(FVector2D location, float radius);
	virtual void Render(UCanvas* canvas, FVector2D baseLocation = FVector2D::ZeroVector);
	virtual void OnKeyPress(bool keyPressed[256]);
	virtual void OnMouseMove();
};