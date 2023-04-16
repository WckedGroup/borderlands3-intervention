#include "pch.h"

Menu* MainMenu = new Menu("Borderlands 3 Intervention 1.3", { 500, 200 }, { 300, 250 });
Toggle* aimbotNearestAim;
Toggle* aimbotNearestPlayer;
Toggle* aimbotEnabled;
Toggle* draw2DBoxes;
Toggle* draw3DBoxes;
Toggle* drawItems;
Toggle* drawVehicles;
Toggle* drawHealthBar;
Toggle* drawNames;
Toggle* drawAimingLines;
Toggle* drawAimTrace;
Toggle* drawEnemySnapLines;
Toggle* removeRecoil;
Toggle* removeSpread;
Toggle* removeSway;
Toggle* removeChargeTime;
Slider* superFireRate;
Slider* superDamage;
Toggle* infiniteAmmo;
Toggle* instantHit;
Toggle* infiniteRange; 
Slider* superSpeed;
Toggle* instaKill;
Toggle* shatterFrozen;
Toggle* oneHit;
Toggle* flyMode;
Toggle* godMode;
Toggle* magnet;
Toggle* currencyMagnet;

void BuildMainMenu()
{
	TabContainer* tabContainer = new TabContainer(FVector2D::ZeroVector, { 300, 250 });

	Tab* espTab = new Tab("ESP", false);
	draw2DBoxes = new Toggle({ 5.0f, 5.0f }, "2D Boxes", true, [](bool value)
		{
			if (value)
			{
				draw3DBoxes->Toggled = false;
			}
		});

	draw3DBoxes = new Toggle({ 5.0f, 30.0f }, "3D Boxes", false, [](bool value)
		{
			if (value)
			{
				draw2DBoxes->Toggled = false;
			}
		});

	drawItems = new Toggle({ 5.0f, 55.0f }, "Items", true);
	drawVehicles = new Toggle({ 5.0f, 80.0f }, "Vehicles", true);
	drawHealthBar = new Toggle({ 5.0f, 105.0f }, "Health Bar", true);
	drawNames = new Toggle({ 5.0f, 130.0f }, "Names", true);
	drawEnemySnapLines = new Toggle({ 5.0f, 155.0f }, "Enemy Snap Lines", true);
	//drawAimingLines = new Toggle({ 5.0f, 180.0f }, "Enemy Aiming Lines", true);

	espTab->AddChild(draw2DBoxes);
	espTab->AddChild(draw3DBoxes);
	espTab->AddChild(drawItems);
	espTab->AddChild(drawVehicles);
	espTab->AddChild(drawHealthBar);
	espTab->AddChild(drawNames);
	//espTab->AddChild(drawAimingLines);
	espTab->AddChild(drawEnemySnapLines);

	Tab* aimbotTab = new Tab("Aimbot", false);

	aimbotNearestAim = new Toggle({ 5.0f, 5.0f }, "Nearest Aim", true, [](bool value) { aimbotNearestPlayer->Toggled = !value; });
	aimbotNearestPlayer = new Toggle({ 5.0f, 30.0f }, "Nearest Player", false, [](bool value) { aimbotNearestAim->Toggled = !value; });
	aimbotEnabled = new Toggle({ 5.0f, 55.0f }, "Always Enabled");

	aimbotTab->AddChild(aimbotNearestAim);
	aimbotTab->AddChild(aimbotNearestPlayer);
	aimbotTab->AddChild(aimbotEnabled);

	Tab* weaponTab = new Tab("Weapon", false);

	removeRecoil = new Toggle({ 5.0f, 5.0f }, "No Recoil", true);
	removeSpread = new Toggle({ 5.0f, 30.0f }, "No Spread", true);
	removeSway = new Toggle({ 5.0f, 55.0f }, "No Sway", true);
	removeChargeTime = new Toggle({ 5.0f, 80.0f }, "No Charge Time", true);
	superFireRate = new Slider("Super Fire Rate", { 5.0f, 105.0f }, {250.0f, 12.0f}, 1.0f, 1.0f, 40.0f, 1.0f);
	superDamage = new Slider("Super Damage", { 5.0f, 130.0f }, { 250.0f, 12.0f }, 1.0f, 1.0f, 40.0f, 1.0f);
	infiniteAmmo = new Toggle({ 5.0f, 155.0f }, "Infinite Ammo", true);
	instantHit = new Toggle({ 5.0f, 180.0f }, "Instant Hit", true);
	infiniteRange = new Toggle({ 5.0f, 205.0f }, "Infinite Range", true);

	weaponTab->AddChild(removeRecoil);
	weaponTab->AddChild(removeSpread);
	weaponTab->AddChild(removeSway);
	weaponTab->AddChild(removeChargeTime);
	weaponTab->AddChild(superFireRate);
	weaponTab->AddChild(superDamage);
	weaponTab->AddChild(infiniteAmmo);
	weaponTab->AddChild(instantHit);
	weaponTab->AddChild(infiniteRange);

	Tab* miscTab = new Tab("Misc", false);

	instaKill = new Toggle({ 5.0f, 5.0f }, "Kill Aura");
	shatterFrozen = new Toggle({ 5.0f, 30.0f }, "Auto Shatter", true);
	oneHit = new Toggle({ 5.0f, 55.0f }, "One Hit Kill (F7)");
	flyMode = new Toggle({ 5.0f, 80.0f }, "Fly Mode (F8)", false);
	godMode = new Toggle({ 5.0f, 105.0f }, "God Mode (F9)", false);
	magnet = new Toggle({ 5.0f, 130.0f }, "Magnet (F10)");
	currencyMagnet = new Toggle({ 5.0f, 155.0f }, "Currency Magnet", true);
	drawAimTrace = new Toggle({ 5.0f, 180.0f }, "Distance Meter", true);
	superSpeed = new Slider("Super Speed", { 5.0f, 205.0f }, { 250.0f, 12.0f }, 1.0f, 1.0f, 40.0f, 1.0f);

	miscTab->AddChild(instaKill);
	miscTab->AddChild(shatterFrozen);
	miscTab->AddChild(oneHit);
	miscTab->AddChild(flyMode);
	miscTab->AddChild(godMode);
	miscTab->AddChild(magnet);
	miscTab->AddChild(currencyMagnet);
	miscTab->AddChild(drawAimTrace);
	miscTab->AddChild(superSpeed);

	tabContainer->AddTab(espTab);
	tabContainer->AddTab(aimbotTab);
	tabContainer->AddTab(weaponTab);
	tabContainer->AddTab(miscTab);

	MainMenu->AddChild(tabContainer);
}