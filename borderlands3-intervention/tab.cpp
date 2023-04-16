#include "pch.h"

Tab::Tab(std::string label, bool sizeMatchContent, std::function<void()> callback)
	: Button(label, FVector2D::ZeroVector, FVector2D::ZeroVector, sizeMatchContent, callback)
{

}

void Tab::AddChild(Component* child)
{
	child->Location = this->Location + child->Location;
	
	child->Parent = this;

	this->Children.push_back(child);
}

void Tab::Render(UCanvas* canvas, FVector2D baseLocation)
{
	Button::Render(canvas, baseLocation);
}

void Tab::OnKeyPress(bool keyPressed[256])
{
	Button::OnKeyPress(keyPressed);
}

void Tab::OnMouseMove()
{
	Button::OnMouseMove();
}