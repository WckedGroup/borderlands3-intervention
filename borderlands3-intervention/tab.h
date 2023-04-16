#pragma once
#include "pch.h"

class Tab : public Button
{
public:
	std::list<Component*> Children;

	Tab(std::string label, bool sizeMatchContent = true, std::function<void()> callback = nullptr);

	void AddChild(Component* child);

	virtual void Render(UCanvas* canvas, FVector2D baseLocation = FVector2D::ZeroVector);
	virtual void OnKeyPress(bool keyPressed[256]);
	virtual void OnMouseMove();
};