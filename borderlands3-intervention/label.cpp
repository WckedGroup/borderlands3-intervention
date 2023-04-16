#include "pch.h"

Label::Label(std::string text, FVector2D location, bool centerVertical, bool centerHorizontal)
	: Component(location, FVector2D::ZeroVector)
{
	this->Text = text;
	this->CenterVertical = centerVertical;
	this->CenterHorizontal = centerHorizontal;
}

void Label::Render(UCanvas* canvas, FVector2D baseLocation)
{
	if (this->Text != "")
	{
		if (this->Size == FVector2D::ZeroVector)
		{
			this->Size = canvas->UTextSizeS(this->Text);
		}

		canvas->UDrawTextS(this->Text, this->Location + baseLocation, this->TextColor, this->CenterVertical, this->CenterHorizontal);
	}

	this->TotalSize = this->Size;
}

void Label::OnKeyPress(bool keyPressed[256])
{
	Component::OnKeyPress(keyPressed);
}

void Label::OnMouseMove()
{
	Component::OnMouseMove();
}