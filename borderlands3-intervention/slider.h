#pragma once
#include "pch.h"

class Slider : public Component
{
public:
	float CurrentValue;
	float MinimunValue;
	float MaximunValue;
	float IncrementValue;
	bool ShowValue;
	std::string Label;

	Slider(std::string label, FVector2D location, FVector2D size, float currentValue, float minimunValue, float maximunValue, float incrementValue, bool showValue = true);

	template<class ValueType>
	ValueType GetValue();

	virtual void Render(UCanvas* canvas, FVector2D baseLocation = FVector2D::ZeroVector);
	virtual void OnKeyPress(bool keyPressed[256]);
	virtual void OnMouseMove();

private:
	FVector2D DownPointStart, UpPointEnd, DownPointEnd, MiddlePointStart, MiddlePointEnd, CurrentValuePoint;
	float GetPercentage();
	void SetPercentage();
	void RoundToNearestIncrement();
	float GetIncrementPercentageSize();
	int GetIncrementBarsCount(float incrementSize);
};