#pragma once
#include "pch.h"

class Button : public Component
{
public:
	std::string Label;
	std::function<void()> Callback;
	bool SizeMatchContent = false;

	Button(std::string label, FVector2D location, FVector2D size, bool sizeMatchContent = true, std::function<void()> callback = nullptr);

	virtual void Render(UCanvas* canvas, FVector2D baseLocation = FVector2D::ZeroVector);
	virtual void OnKeyPress(bool keyPressed[256]);
	virtual void OnMouseMove();
};