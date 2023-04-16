#include "pch.h"

template<class ValueType>
ValueType Slider::GetValue()
{
	return (ValueType)this->CurrentValue;
}

//((current - min) * 100) / (max - min)
float Slider::GetPercentage()
{
	return (this->CurrentValue - this->MinimunValue) / (this->MaximunValue - this->MinimunValue);
}

void Slider::RoundToNearestIncrement()
{
	float remainder = fmod(this->CurrentValue, this->IncrementValue);

	if (remainder != 0)
	{
		this->CurrentValue = this->CurrentValue + this->IncrementValue - remainder;

		if (this->CurrentValue < this->MinimunValue)
		{
			this->CurrentValue = this->MinimunValue;
		}
	}
}

float Slider::GetIncrementPercentageSize()
{
	float incrementPercentage = this->IncrementValue / (this->MaximunValue - this->MinimunValue);

	return this->Size.X * incrementPercentage;
}

int Slider::GetIncrementBarsCount(float incrementSize)
{
	return (int)floor(this->Size.X / incrementSize);
}

//curr = min + (perc * max - min)
void Slider::SetPercentage()
{
	if (Input::MousePosition.X >= this->BaseLocation.X && Input::MousePosition.X <= (this->BaseLocation.X + this->Size.X))
	{
		float percentage = (Input::MousePosition.X - this->BaseLocation.X)/ this->Size.X;

		//this->CurrentValue = (this->MaximunValue - this->MinimunValue) * percentage;
		this->CurrentValue = this->MinimunValue + (percentage * (this->MaximunValue - this->MinimunValue));

		RoundToNearestIncrement();
	}
}

Slider::Slider(std::string label, FVector2D location, FVector2D size, float currentValue, float minimunValue, float maximunValue, float incrementValue, bool showValue)
	: Component(location, size)
{
	this->Label = label;
	this->ShowValue = showValue;
	this->CurrentValue = currentValue;
	this->MinimunValue = minimunValue;
	this->MaximunValue = maximunValue;
	this->IncrementValue = incrementValue;
}

void Slider::Render(UCanvas* canvas, FVector2D baseLocation)
{
	this->BaseLocation = this->Location + baseLocation;

	FVector2D textPositionBase = this->BaseLocation;
	FVector2D textSize = canvas->UTextSizeS(this->Label);
	this->BaseLocation.X += textSize.X + 12.0f;

	this->DownPointStart = { this->BaseLocation.X, this->BaseLocation.Y + 11.0f };
	this->UpPointEnd = { this->BaseLocation.X + this->Size.X, this->BaseLocation.Y };
	this->DownPointEnd = { this->BaseLocation.X + this->Size.X, this->BaseLocation.Y + 11.0f };
	this->MiddlePointStart = { this->BaseLocation.X, this->BaseLocation.Y + 5.0f };
	this->MiddlePointEnd = { this->BaseLocation.X + this->Size.X, this->BaseLocation.Y + 5.0f };

	//canvas->DrawLine(this->BaseLocation, this->DownPointStart, 2.0f, ACCENT_COLOR);
	canvas->DrawLine(this->MiddlePointStart, this->MiddlePointEnd, 3.0f, ACCENT_COLOR);
	//canvas->DrawLine(this->UpPointEnd, this->DownPointEnd, 2.0f, ACCENT_COLOR);

	//float incrementSize = GetIncrementPercentageSize();

	//for (int barIndex = 1; barIndex < GetIncrementBarsCount(incrementSize); barIndex++)
	//{
	//	canvas->DrawLine({ this->BaseLocation.X + (incrementSize*barIndex), this->BaseLocation.Y + 1.0f}, {this->DownPointStart.X + (incrementSize * barIndex), this->DownPointStart.Y - 1.0f }, 1.0f, ACCENT_COLOR);
	//}

	this->CurrentValuePoint.Y = this->BaseLocation.Y + 5.0f;
	this->CurrentValuePoint.X = this->BaseLocation.X + (this->Size.X * this->GetPercentage());

	canvas->DrawPolygon(this->CurrentValuePoint, { 8.0f, 8.0f }, NUMBER_OF_SIDES, White);

	FVector2D textPosition = { textPositionBase.X, textPositionBase.Y + 5.0f };

	this->TextColor = this->MouseOver ? SELECTED_COLOR : TEXT_COLOR;

	canvas->UDrawTextSCenterVertical(this->Label, textPosition, this->TextColor, false);

	textPosition.X = this->BaseLocation.X + this->Size.X + 8.0f;

	canvas->UDrawTextSCenterVertical(std::format("{:.2f}", this->CurrentValue), textPosition, this->TextColor, false);

	this->TotalSize = { this->BaseLocation.X + this->Size.X - textPositionBase.X, 11.0f };
}

void Slider::OnKeyPress(bool keyPressed[256])
{
	if (Input::KeyPressed[VK_LBUTTON] && this->CursorInsideArea(this->BaseLocation, this->Size))
	{
		this->Dragging = true;

		SetPercentage();
	}
	
	if (!Input::KeyPressed[VK_LBUTTON])
	{
		this->Dragging = false;
	}
}

void Slider::OnMouseMove()
{
	this->MouseOver = this->CursorInsideArea();

	if (this->Dragging)
	{
		SetPercentage();
	}
}