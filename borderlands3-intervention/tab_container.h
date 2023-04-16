#pragma once
#include "pch.h"

class TabContainer : public Component
{
public:
	std::list<Tab*> Tabs;
	Tab* SelectedTab;

	TabContainer(FVector2D location, FVector2D size);

	void AddTab(Tab* tab);
	void ChangeSelectedTab(Tab* newSelectedTab);

	virtual void Render(UCanvas* canvas, FVector2D baseLocation = FVector2D::ZeroVector);
	virtual void OnKeyPress(bool keyPressed[256]);
	virtual void OnMouseMove();
private:

	const FVector2D TabSize { 100.0f, 30.0f };
	FVector2D TabAreaLocation;
};