#include "pch.h"

Toggle::Toggle(FVector2D location, std::string label, bool toggled, std::function<void(bool)> callback)
	: Component(location)
{
	this->Label = label;
	this->Toggled = toggled;

	if (callback)
	{
		this->Callback = callback;
	}
}

void Toggle::Render(UCanvas* canvas, FVector2D baseLocation)
{
	this->BaseLocation = baseLocation;
	FVector2D localLocation = this->Location + baseLocation;
	float pillCornersOffset = 6.0f;
	FVector2D toggleBorderRadius = { 8.0f, 8.0f };
	FVector2D borderRadius = { 5.0f, 5.0f };

	FVector2D labelSize = FVector2D::ZeroVector;
	this->TextColor = this->MouseOver ? SELECTED_COLOR : TEXT_COLOR;
	FVector2D textLocation = { localLocation.X, localLocation.Y + borderRadius.Y };

	if (this->Label != "")
	{
		canvas->UDrawTextSCenterVertical(this->Label, textLocation, this->TextColor, false);

		labelSize = canvas->UTextSizeS(this->Label);
		labelSize.X += 20.0f;
	}

	//Draw pill
	this->ToggleClickLocation = { localLocation.X + labelSize.X, localLocation.Y };
	this->ToggleClickSize = { pillCornersOffset * 2 + borderRadius.X, borderRadius.Y * 2 };
	this->Size = { labelSize.X + pillCornersOffset * 2 + borderRadius.X * 2, borderRadius.Y * 2 };
	FVector2D location = { localLocation.X + labelSize.X + pillCornersOffset + borderRadius.X / 2, localLocation.Y };

	FVector2D pillBodyLocation = { location.X - pillCornersOffset, location.Y };
	FVector2D pillBodySize = { pillCornersOffset * 2, borderRadius.Y * 2 };

	FVector2D togglePosition = { location.X + (this->Toggled ? pillCornersOffset : -pillCornersOffset), location.Y + borderRadius.Y };

	FLinearColor color = this->Toggled ? ACCENT_COLOR : SWITCH_OFFCOLOR;

	//Draw body
	canvas->DrawPolygon({ location.X - pillCornersOffset, location.Y + borderRadius.Y }, borderRadius, NUMBER_OF_SIDES, color);
	canvas->DrawPolygon({ location.X + pillCornersOffset, location.Y + borderRadius.Y }, borderRadius, NUMBER_OF_SIDES, color);
	canvas->K2_DrawTexture(pillBodyLocation, pillBodySize, color);

	//Draw toggle
	canvas->DrawPolygon(canvas->GradientTexture0, togglePosition, toggleBorderRadius, NUMBER_OF_SIDES, White);

	this->TotalSize = { location.X + pillCornersOffset + borderRadius.X, toggleBorderRadius.Y * 2 };
}

void Toggle::OnKeyPress(bool keyPressed[256])
{
	if (keyPressed[VK_LBUTTON] && this->CursorInsideArea(this->ToggleClickLocation, this->ToggleClickSize))
	{
		this->Toggled = !this->Toggled;

		if (this->Callback)
		{
			this->Callback(this->Toggled);
		}
	}
}

void Toggle::OnMouseMove()
{
	Component::OnMouseMove();
}