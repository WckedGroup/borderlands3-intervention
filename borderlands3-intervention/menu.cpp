#include "pch.h"

Menu::Menu(std::string title, FVector2D location, FVector2D size)
	: Component(location, size)
{
	this->Location = location;
	this->Size = size;
	this->Title = title;
	this->HeaderSize.X = size.X;
	this->Visible = false;

	CloseButton = new Button("X", FVector2D::ZeroVector, { 16.0f, 16.0f }, false, [this] { this->Visible = false; this->OnVisibleChanged(false); });
}

void Menu::AddChild(Component* child)
{
	auto sizeDifference = (this->Size) - (child->Location + child->Size);

	if (sizeDifference.X < 0.0f)
	{
		this->Size.X += -(sizeDifference.X);
		this->HeaderSize.X = this->Size.X;
	}

	if (sizeDifference.Y < 0.0f)
	{
		this->Size.Y += -(sizeDifference.X);
	}

	child->Parent = this;

	this->Children.push_back(child);
}

void Menu::Render(UCanvas* canvas)
{
	if (!this->Visible)
		return;

	canvas->K2_DrawTexture(this->Location, this->HeaderSize, this->BackgroundColor);
	canvas->DrawBox(this->Location, this->HeaderSize, 1.0f, this->BorderColor, false, Black);

	CloseButton->Render(canvas, { this->Location.X + this->Size.X - 20.0f, this->Location.Y + 2.0f});

	canvas->UDrawTextSCenterVertical(this->Title, { this->Location.X + 2.0f, this->Location.Y + this->HeaderSize.Y / 2 }, TEXT_COLOR, false);

	for (auto* child : this->Children)
	{
		if (!child->Visible)
			continue;

		child->Render(canvas, this->Location + FVector2D{0.0f, this->HeaderSize.Y});
	}

	this->MainCursor->Render(canvas);
}

void Menu::OnKeyPress(bool keyPressed[256])
{
	if (!this->Visible)
		return;

	CloseButton->OnKeyPress(Input::KeyPressed);

	if (Input::KeyPressed[VK_LBUTTON] && this->CursorInsideArea(this->Location, this->HeaderSize))
	{
		this->Dragging = true;
		this->RelativeDraggingLocation = Input::MousePosition - this->Location;
	}

	if (!Input::KeyPressed[VK_LBUTTON])
	{
		this->Dragging = false;
		this->RelativeDraggingLocation = FVector2D::ZeroVector;
	}

	if (!this->Dragging)
	{
		for (auto* child : this->Children)
		{
			if (!child->Visible)
				continue;

			child->OnKeyPress(Input::KeyPressed);
		}
	}
}

void Menu::OnMouseMove()
{
	this->MainCursor->Update(Input::MousePosition.X, Input::MousePosition.Y);

	if (!this->Visible)
		return;

	if (this->Dragging)
	{
		this->Location = Input::MousePosition - this->RelativeDraggingLocation;
	}
	else
	{
		CloseButton->OnMouseMove();

		for (auto* child : this->Children)
		{
			if (!child->Visible)
				continue;

			child->OnMouseMove();
		}
	}
}