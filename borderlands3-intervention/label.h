#pragma once
#include "pch.h"

class Label : public Component
{
public:
	std::string Text = "";
	bool CenterHorizontal = true;
	bool CenterVertical = true;

	Label(std::string text, FVector2D location, bool centerVertical = true, bool centerHorizontal = true);

	virtual void Render(UCanvas* canvas, FVector2D baseLocation = FVector2D::ZeroVector);
	virtual void OnKeyPress(bool keyPressed[256]);
	virtual void OnMouseMove();
};