#include "pch.h"

Component::Component(FVector2D location)
{
	this->Location = location;
}

Component::Component(FVector2D location, FVector2D size, bool visible) 
{
	this->Location = location;
	this->Size = size;
	this->Visible = visible;
}

bool Component::CursorInsideArea()
{
	return this->CursorInsideArea(this->Location + this->BaseLocation, this->TotalSize);
}

bool Component::CursorInsideArea(FVector2D location, FVector2D size)
{
	if (Input::MousePosition >= location
		&& Input::MousePosition <= (location + size))
	{
		return true;
	}

	return false;
}

bool Component::CursorInsideCircle(FVector2D location, float radius)
{
	return Input::MousePosition.DistTo(location) <= radius;
}

void Component::Render(UCanvas* canvas, FVector2D baseLocation)
{
	this->BaseLocation = baseLocation;

	this->BackgroundColor = this->MouseOver ? SELECTED_COLOR : BACKGROUND_COLOR;

	canvas->K2_DrawTexture(this->Location + baseLocation, this->Size, this->BackgroundColor);

	canvas->K2_DrawBox(this->Location + baseLocation, this->Size, 1.0f, BORDER_COLOR);
}

void Component::OnKeyPress(bool keyPressed[256])
{
	if (keyPressed[VK_LBUTTON] && this->CursorInsideArea())
	{
		this->Dragging = true;
		this->RelativeDraggingLocation = Input::MousePosition - this->Location;
	}

	if (!keyPressed[VK_LBUTTON])
	{
		this->Dragging = false;
		this->RelativeDraggingLocation = FVector2D::ZeroVector;
	}
}

void Component::OnMouseMove()
{
	this->MouseOver = this->CursorInsideArea();
	
	if (this->Dragging)
	{
		this->Location = Input::MousePosition - this->RelativeDraggingLocation;
	}
}