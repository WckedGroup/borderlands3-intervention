#pragma once
#include "pch.h"

class Menu : public Component
{
public:
	std::string Title = "";
	std::list<Component*> Children;
	std::function<void(bool)> OnVisibleChanged;

	Cursor* MainCursor = new Cursor();
	Button* CloseButton;

	FVector2D HeaderSize{ 0.0f, 20.0f };

	Menu(std::string title, FVector2D location, FVector2D size);

	void AddChild(Component* child);
	void Render(UCanvas* canvas);
	void OnKeyPress(bool keyPressed[256]);
	void OnMouseMove();
};