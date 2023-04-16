#include "pch.h"

TabContainer::TabContainer(FVector2D location, FVector2D size)
	: Component(location, size)
{
	this->TabAreaLocation = { location.X, location.Y + this->TabSize.Y + 1.0f };
}

void TabContainer::AddTab(Tab* tab)
{
	auto tabCount = this->Tabs.size();

	tab->Location = { this->Location.X + (this->TabSize.X * tabCount), this->Location.Y };
	tab->Size = this->TabSize;

	if (tabCount == 0)
	{
		this->SelectedTab = tab;
		ChangeSelectedTab(tab);
	}

	this->Tabs.push_back(tab);

	this->Size.X = this->TabSize.X * this->Tabs.size();
}

void TabContainer::ChangeSelectedTab(Tab* newSelectedTab)
{
	this->SelectedTab->TextColor = TEXT_COLOR;
	newSelectedTab->TextColor = ACCENT_COLOR;

	this->SelectedTab = newSelectedTab;
}

void TabContainer::Render(UCanvas* canvas, FVector2D baseLocation)
{
	canvas->K2_DrawTexture(this->TabAreaLocation + baseLocation, this->Size, this->BackgroundColor);
	canvas->DrawBox(this->TabAreaLocation + baseLocation, this->Size, 1.0f, this->BorderColor, false, Black);

	for (auto* tab : this->Tabs)
	{
		tab->Render(canvas, baseLocation);

		if (this->SelectedTab == tab)
		{
			for (auto* child : tab->Children)
			{
				child->Render(canvas, this->TabAreaLocation + baseLocation);
			}
		}
	}
}

void TabContainer::OnKeyPress(bool keyPressed[256])
{
	for (auto* tab : this->Tabs)
	{
		if (tab->CursorInsideArea())
		{
			ChangeSelectedTab(tab);
		}

		if (this->SelectedTab == tab)
		{
			for (auto* child : tab->Children)
			{
				child->OnKeyPress(keyPressed);
			}
		}
	}
}

void TabContainer::OnMouseMove()
{
	for (auto* tab : this->Tabs)
	{
		tab->OnMouseMove();

		if (this->SelectedTab == tab)
		{
			for (auto* child : tab->Children)
			{
				child->OnMouseMove();
			}
		}
	}
}