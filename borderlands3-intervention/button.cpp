#include "pch.h"

Button::Button(std::string label, FVector2D location, FVector2D size, bool sizeMatchContent, std::function<void()> callback)
	: Component(location, size)
{
	this->Label = label;
	this->SizeMatchContent = sizeMatchContent;

	if (callback)
	{
		this->Callback = callback;
	}
}

void Button::Render(UCanvas* canvas, FVector2D baseLocation)
{
	Component::Render(canvas, baseLocation);

	if (this->Label != "")
	{
		FVector2D textSize = FVector2D::ZeroVector;

		if (this->SizeMatchContent)
		{
			textSize = canvas->UTextSizeS(this->Label);
			this->Size = { textSize.X + 12.0f, textSize.Y + 8.0f };
		}

		FVector2D textPosition = { this->Location.X + this->Size.X / 2, this->Location.Y + this->Size.Y / 2 };

		canvas->UDrawTextCenteredS(this->Label, textPosition + baseLocation, this->TextColor, false);
	}

	this->TotalSize = this->Size;
}

void Button::OnKeyPress(bool keyPressed[256])
{
	if (keyPressed[VK_LBUTTON] && this->CursorInsideArea())
	{
		if (this->Callback)
		{
			this->Callback();
		}
	}
}

void Button::OnMouseMove()
{
	Component::OnMouseMove();
}