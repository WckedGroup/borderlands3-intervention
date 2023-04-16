#pragma once
#include "pch.h"

class Toggle : public Component
{
public:
	bool Toggled = false;
	std::string Label = "";
	std::function<void(bool)> Callback;

	Toggle(FVector2D location, std::string label, bool toggled = false, std::function<void(bool)> callback = nullptr);

	virtual void Render(UCanvas* canvas, FVector2D baseLocation = FVector2D::ZeroVector);
	virtual void OnKeyPress(bool keyPressed[256]);
	virtual void OnMouseMove();
private:
	FVector2D ToggleClickLocation;
	FVector2D ToggleClickSize;
};