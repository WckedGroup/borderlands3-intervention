#include "pch.h"

PostRenderOriginal_t PostRenderOriginal;
static int sizeX, sizeY;
std::function<void(bool)> FlyModeCallback;
std::function<void(bool)> GodModeCallback;

void Aimbot(UWorld* world, UCanvas* canvas, TArray<AActor*> actors, FVector cameraLoc, FRotator cameraRot, APlayerController* localController, AActor* localActor);

void DrawAimTrace(UWorld* world, UCanvas* canvas, APlayerController* localController, FVector cameraLocation, FRotator rotation);

void ProcessWeaponComponents(AWeapon* currentWeapon)
{
	auto recoilComponent = (UWeaponRecoilComponent*)currentWeapon->GetComponentByClass(UWeaponRecoilComponent::StaticClass());

	if (recoilComponent)
	{
		if (removeRecoil->Toggled)
			recoilComponent->CachedControlComponent->CurrentRotation = FRotator::ZeroRotator;
	}

	auto swayComponent = (UWeaponSwayComponent*)currentWeapon->GetComponentByClass(UWeaponSwayComponent::StaticClass());

	if (swayComponent)
	{
		if (removeSway->Toggled)
		{
			swayComponent->CurrentRotation = FRotator::ZeroRotator;
			swayComponent->CurrentScale = 0.0f;
			swayComponent->Scale.Value = 0.0f;
			swayComponent->Speed = 0.0f;
			swayComponent->AccuracyScale.Value = swayComponent->AccuracyScale.BaseValue * 100.0f;
		}
	}

	auto heatComponent = (UWeaponHeatComponent*)currentWeapon->GetComponentByClass(UWeaponHeatComponent::StaticClass());

	if (heatComponent)
	{
		heatComponent->HeatCoolDownRate.Value = heatComponent->HeatCoolDownRate.BaseValue * 10;
		heatComponent->HeatCoolDownDelay.Value = 0.0f;
	}

	auto fireComponent = currentWeapon->CurrentFireComponent;

	if (fireComponent)
	{
		fireComponent->ShotAmmoCost.Value = (infiniteAmmo->Toggled) ? 0 : fireComponent->ShotAmmoCost.BaseValue;

		if (removeSpread->Toggled)
		{
			fireComponent->Spread.Value = 0.0f;
			fireComponent->AccuracyImpulse.Value = fireComponent->AccuracyImpulse.BaseValue * 100.0f;
		}
		else
		{
			fireComponent->Spread.Value = fireComponent->Spread.BaseValue;
			fireComponent->AccuracyImpulse.Value = fireComponent->AccuracyImpulse.BaseValue;
		}

		if (superFireRate->CurrentValue != 1.0f)
		{
			fireComponent->FireRate.Value = fireComponent->FireRate.BaseValue * superFireRate->CurrentValue;
			fireComponent->bAutoBurst = true;
			fireComponent->BurstFireDelay.Value = 0.0f;
		}
		else
		{
			fireComponent->FireRate.Value = fireComponent->FireRate.BaseValue;
			fireComponent->BurstFireDelay.Value = fireComponent->BurstFireDelay.BaseValue;
		}

		fireComponent->Damage.Value = (superDamage->CurrentValue != 1.0f) ? fireComponent->Damage.BaseValue * superDamage->CurrentValue : fireComponent->Damage.BaseValue;
	}

	auto reloadComponent = (UWeaponReloadComponent*)currentWeapon->GetComponentByClass(UWeaponReloadComponent::StaticClass());

	if (reloadComponent)
	{
		reloadComponent->ReloadTime.Value = 0.01f;
	}

	auto clipReloadComponent = (UWeaponClipReloadComponent*)currentWeapon->GetComponentByClass(UWeaponClipReloadComponent::StaticClass());

	if (clipReloadComponent)
	{
		clipReloadComponent->TapedReloadTime.Value = 0.01f;
		clipReloadComponent->TapedReloadCompletePercent = 1.0f;
		clipReloadComponent->ReloadCompletePercent = 1.0f;
	}

	auto singleFeedReloadComponent = (UWeaponSingleFeedReloadComponent*)currentWeapon->GetComponentByClass(UWeaponSingleFeedReloadComponent::StaticClass());

	if (singleFeedReloadComponent)
	{
		singleFeedReloadComponent->bDisableInterruptedToUse = true;
		singleFeedReloadComponent->SingleFeedCompletePercent = 1.0f;
	}

	auto fireProjectileComponent = (UWeaponFireProjectileComponent*)currentWeapon->GetComponentByClass(UWeaponFireProjectileComponent::StaticClass());

	if (fireProjectileComponent)
	{
		fireProjectileComponent->ProjectileSpeedScale.Value = (instantHit->Toggled) ? fireProjectileComponent->ProjectileSpeedScale.BaseValue * 25.0f : fireProjectileComponent->ProjectileSpeedScale.BaseValue * 1.0f;

		if (infiniteRange->Toggled)
			fireProjectileComponent->Range = 20000.0f;
	}

	auto chargingComponent = (UWeaponChargeComponent*)currentWeapon->GetComponentByClass(UWeaponChargeComponent::StaticClass());

	if (chargingComponent)
	{
		chargingComponent->ChargeTime.Value = (removeChargeTime->Toggled) ? 0.0f : chargingComponent->ChargeTime.BaseValue;
	}
}

void PostRenderHook(UGameViewportClient* viewport, UCanvas* canvas)
{
	auto world = viewport->World;
	auto gameState = world->GameState;
	auto game = world->OwningGameInstance;

	if (gameState)
	{
		auto gameMode = gameState->AuthorityGameMode;
		auto localPlayer = game->LocalPlayers[0];
		auto localController = localPlayer->PlayerController;
		auto localCamera = localController->PlayerCameraManager;

		if (localCamera && localController)
		{
			MainMenu->OnVisibleChanged = [localController](bool visible)
			{
				localController->ClientIgnoreLookInput(visible);
				localController->SetIgnoreLookInput(visible);
			};

			MainMenu->Render(canvas);
			localController->GetViewportSize(&sizeX, &sizeY);
			canvas->UDrawTextCenteredS(UCanvas::robotoDistanceField, "Borderlands 3 Intervention V1.3 - Made by Barata - [Home] for menu", { sizeX / 2.0f, 15.0f }, ACCENT_COLOR);

			auto localPawn = localController->Pawn;

			if (localPawn)
			{
				auto level = world->PersistentLevel;

				auto cameraLoc = localCamera->GetCameraLocation();

				auto cameraRot = localCamera->GetCameraRotation();

				if (localPawn->IsA(AOakCharacter::StaticClass()))
				{
					auto localOakCharacter = (AOakCharacter*)localPawn;

					DrawAimTrace(world, canvas, localController, cameraLoc, cameraRot);

					auto damageComponent = (UDamageComponent*)localPawn->GetComponentByClass(UDamageComponent::StaticClass());

					GodModeCallback = [damageComponent](bool value)
					{
						if (damageComponent)
						{

							damageComponent->SetGodMode(value);

							if (value)
							{
								damageComponent->SelfReflectionTowardsAttacker.Value = damageComponent->SelfReflectionTowardsAttacker.BaseValue * 10.0f;
								damageComponent->SelfReflectionDamageScale.Value = damageComponent->SelfReflectionDamageScale.BaseValue * 10.0f;
								damageComponent->SelfReflectionDamageTakenScale.Value = 0.0f;
								damageComponent->SelfReflectionChance.Value = damageComponent->SelfReflectionChance.BaseValue * 10.0f;
							}
							else
							{
								damageComponent->SelfReflectionTowardsAttacker.Value = damageComponent->SelfReflectionTowardsAttacker.BaseValue;
								damageComponent->SelfReflectionDamageScale.Value = damageComponent->SelfReflectionDamageScale.BaseValue;
								damageComponent->SelfReflectionDamageTakenScale.Value = damageComponent->SelfReflectionDamageTakenScale.BaseValue;
								damageComponent->SelfReflectionChance.Value = damageComponent->SelfReflectionChance.BaseValue;
							}
						}
					};

					if (!godMode->Callback)
						godMode->Callback = GodModeCallback;

					auto movementComponent = (UOakCharacterMovementComponent*)localPawn->GetComponentByClass(UOakCharacterMovementComponent::StaticClass());

					if (movementComponent)
					{
						FlyModeCallback = [movementComponent, localOakCharacter](bool value)
						{
							if (movementComponent && localOakCharacter)
							{
								localOakCharacter->SetActorEnableCollision(!value);
								movementComponent->SetMovementMode(value ? EMovementMode::MOVE_Flying : EMovementMode::MOVE_Walking, 0);
								movementComponent->MaxFlySpeed.Value = value ? movementComponent->MaxFlySpeed.BaseValue * superSpeed->CurrentValue : movementComponent->MaxFlySpeed.BaseValue;
							}
						};

						if (!flyMode->Callback)
							flyMode->Callback = FlyModeCallback;

						if (superSpeed->CurrentValue != 1.0f)
						{
							movementComponent->MaxWalkSpeed.Value = movementComponent->MaxWalkSpeed.BaseValue * superSpeed->CurrentValue;
							movementComponent->MaxSprintSpeed.Value = movementComponent->MaxSprintSpeed.BaseValue * superSpeed->CurrentValue;
							movementComponent->MaxWalkSpeedInjured.Value = movementComponent->MaxWalkSpeedInjured.BaseValue * superSpeed->CurrentValue;
							movementComponent->MaxWalkSpeedInjuredBeingRevived.Value = movementComponent->MaxWalkSpeedInjuredBeingRevived.BaseValue * superSpeed->CurrentValue;
							movementComponent->MaxLadderDescendSpeed.Value = movementComponent->MaxLadderDescendSpeed.BaseValue * superSpeed->CurrentValue;
							movementComponent->MaxLadderAscendSpeed.Value = movementComponent->MaxLadderAscendSpeed.BaseValue * superSpeed->CurrentValue;
						}
						else
						{
							movementComponent->MaxWalkSpeed.Value = movementComponent->MaxWalkSpeed.BaseValue;
							movementComponent->MaxSprintSpeed.Value = movementComponent->MaxSprintSpeed.BaseValue;
							movementComponent->MaxWalkSpeedInjured.Value = movementComponent->MaxWalkSpeedInjured.BaseValue;
							movementComponent->MaxWalkSpeedInjuredBeingRevived.Value = movementComponent->MaxWalkSpeedInjuredBeingRevived.BaseValue;
							movementComponent->MaxLadderDescendSpeed.Value = movementComponent->MaxLadderDescendSpeed.BaseValue;
							movementComponent->MaxLadderAscendSpeed.Value = movementComponent->MaxLadderAscendSpeed.BaseValue;
						}
					}

					auto useComponent = (UUseComponent*)localPawn->GetComponentByClass(UUseComponent::StaticClass());

					if (useComponent)
					{
						useComponent->InteractDistance = 20000.0f;
						useComponent->VehicleInteractDistance = 20000.0f;
					}

					for (int weaponSlotIndex = 0; weaponSlotIndex < localOakCharacter->ActiveWeaponSlots.Num(); weaponSlotIndex++)
					{
						auto currentWeapon = localOakCharacter->GetWeapon(localOakCharacter->ActiveWeaponSlots[weaponSlotIndex]);

						if (currentWeapon)
						{
							ProcessWeaponComponents(currentWeapon);
						}
					}

					if (level)
					{
						Aimbot(world, canvas, level->Actors, cameraLoc, cameraRot, localController, localPawn);
					}
				}
			}
		}
	}

	PostRenderOriginal(viewport, canvas);
}

void DrawItemPickup(UCanvas* canvas, FVector localPostition, APlayerController* localController, AInventoryItemPickup* item)
{
	if (item->AssociatedInventoryData)
	{
		auto categoryName = item->AssociatedInventoryData->InventoryCategory->CategoryDisplayName.Get();

		if (categoryName)
		{
			std::wstring finalCategoryName = categoryName;

			if (currencyMagnet->Toggled)
			{
				if (finalCategoryName == L"Money" || finalCategoryName == L"Eridium")
				{
					item->K2_TeleportTo(localPostition);
				}
			}

			if (drawItems->Toggled)
			{
				auto color = item->GetInventoryRarityColorOutline();

				auto origin = item->K2_GetActorLocation();
				int distance = localPostition.DistTo(origin) / 100;

				FVector2D screen;
				if (!localController->ProjectWorldLocationToScreen(origin, true, screen))
					return;

				std::wstring finalName = finalCategoryName + L" (" + std::to_wstring(distance) + L"m)";
				canvas->UDrawTextCentered(finalName, screen, color);
			}
		}
	}
}

void DrawVehicles(UCanvas* canvas, FVector localPostition, APlayerController* localController, AVehicle* vehicle)
{
	auto origin = vehicle->K2_GetActorLocation();
	int distance = localPostition.DistTo(origin) / 100;

	FVector2D screen;
	if (!localController->ProjectWorldLocationToScreen(origin, true, screen))
		return;

	auto displayName = UKismetSystemLibrary::GetDisplayNameS(vehicle);

	canvas->UDrawTextCenteredS(displayName + " (" + std::to_string(distance) + "m)", screen, VehiclesColor);
}

void DrawBones(UCanvas* canvas, FVector cameraLoc, FRotator cameraRot, APlayerController* localController, ABPChar_AI_C* aiChar, FLinearColor color)
{
	if (aiChar->Mesh)
	{
		const FMatrix comp2world = aiChar->Mesh->K2_GetComponentToWorld().ToMatrixWithScale();

		for (auto i = 0; i < aiChar->Mesh->ATransform.Num(); i++)
		{
			FVector pos;
			if (aiChar->Mesh->GetBone(i, comp2world, pos))
			{
				FVector2D screen;
				if (!localController->ProjectWorldLocationToScreen(pos, true, screen)) continue;
				auto boneName = aiChar->Mesh->GetBoneName(i);
				auto boneString = boneName.GetString();
				canvas->UDrawTextCentered(boneName.GetWString(), screen, color);
			};
		}
	}
}

void DrawHealthBar(UCanvas* canvas, FVector2D position, FVector2D size, UDamageComponent* damageComponent)
{
	FVector2D barPos = { position.X - 1.0f, position.Y + size.Y + 8.0f };

	auto healthPercentage = damageComponent->GetCurrentHealthPercent();
	auto maxShield = damageComponent->GetMaxShield();
	auto currentShield = damageComponent->GetCurrentShield();
	auto shieldPercentage = currentShield / maxShield;

	canvas->K2_DrawTexture(barPos, { size.X + 2.0f, (size.Y * 0.03f) }, Black);

	size.X -= (size.X * (1.0f - healthPercentage));

	canvas->K2_DrawTexture(barPos, { size.X + 2.0f, (size.Y * 0.03f) }, HealthBarColor);

	canvas->K2_DrawTexture({ barPos.X - 1.0f, barPos.Y - 1.0f, }, { ((size.X + 4.0f) * (shieldPercentage)), (size.Y * 0.03f) + 2.0f }, ShieldBarColor);
}

void DrawNames(UCanvas* canvas, FVector2D position, FVector2D size, int distance, UObject* object, FLinearColor color)
{
	auto displayName = UKismetSystemLibrary::GetDisplayNameS(object);

	FVector2D textPos = { position.X + size.X / 2, position.Y + size.Y + 18.0f + (size.Y * 0.03f) };

	canvas->UDrawTextCenteredS(displayName + " (" + std::to_string(distance) + "m)", textPos, color);
}

void DrawSnapLines(UCanvas* canvas, FVector2D screenPosition, FVector2D position, FVector2D size, FLinearColor color)
{
	FVector2D linePosition = { position.X + size.X / 2, position.Y + size.Y + 36.0f + (size.Y * 0.03f) };

	FVector2D finalScreenPosition = { screenPosition.X / 2.0f, screenPosition.Y };

	canvas->DrawLine(finalScreenPosition, linePosition, 1.3f, color, false, false, Black);
}

void Draw2DBoxes(UCanvas* canvas, FVector2D screenSize, FVector localPostition, APlayerController* localController, UObject* object, UDamageComponent* damageComponent, USceneComponent* sceneComponent, FLinearColor color)
{
	FVector origin, extends;
	float sphereRadius;
	UKismetSystemLibrary::GetComponentBounds(sceneComponent, &origin, &extends, &sphereRadius);
	int distance = localPostition.DistTo(origin) / 100;

	FVector one = origin;
	one.X -= extends.X;
	one.Y -= extends.Y;
	one.Z -= extends.Z;

	FVector two = origin;
	two.X += extends.X;
	two.Y -= extends.Y;
	two.Z -= extends.Z;

	FVector three = origin;
	three.X += extends.X;
	three.Y += extends.Y;
	three.Z -= extends.Z;

	FVector four = origin;
	four.Y += extends.Y;
	four.X -= extends.X;
	four.Z -= extends.Z;

	FVector five = one;
	five.Z += extends.Z * 2;

	FVector six = two;
	six.Z += extends.Z * 2;

	FVector seven = three;
	seven.Z += extends.Z * 2;

	FVector eight = four;
	eight.Z += extends.Z * 2;

	FVector2D s1, s2, s3, s4, s5, s6, s7, s8;
	if (!localController->ProjectWorldLocationToScreen(one, false, s1))
		return;

	if (!localController->ProjectWorldLocationToScreen(two, false, s2))
		return;

	if (!localController->ProjectWorldLocationToScreen(three, false, s3))
		return;

	if (!localController->ProjectWorldLocationToScreen(four, false, s4))
		return;

	if (!localController->ProjectWorldLocationToScreen(five, false, s5))
		return;

	if (!localController->ProjectWorldLocationToScreen(six, false, s6))
		return;

	if (!localController->ProjectWorldLocationToScreen(seven, false, s7))
		return;

	if (!localController->ProjectWorldLocationToScreen(eight, false, s8))
		return;

	float xArray[] = { s1.X, s2.X, s3.X, s4.X, s5.X, s6.X, s7.X, s8.X };
	float yArray[] = { s1.Y, s2.Y, s3.Y, s4.Y, s5.Y, s6.Y, s7.Y, s8.Y };
	float minX, maxX, minY, maxY;

	std::pair<float*, float*> minMaxX = minmaxfloat(xArray, sizeof xArray / sizeof * xArray);
	std::pair<float*, float*> minMaxY = minmaxfloat(yArray, sizeof yArray / sizeof * yArray);

	minX = *minMaxX.first;
	maxX = *minMaxX.second;
	minY = *minMaxY.first;
	maxY = *minMaxY.second;

	auto finalPos = FVector2D(minX, minY);
	auto size = FVector2D(maxX - minX, maxY - minY);

	if (draw2DBoxes->Toggled)
		canvas->DrawBox(finalPos, size, 1.3f, color, true, Black);

	if (drawHealthBar->Toggled)
		DrawHealthBar(canvas, finalPos, size, damageComponent);

	if (drawNames->Toggled)
		DrawNames(canvas, finalPos, size, distance, object, color);

	if (drawEnemySnapLines->Toggled)
		DrawSnapLines(canvas, screenSize, finalPos, size, color);
}

void Draw3DBoxes(UCanvas* canvas, FVector localPostition, APlayerController* localController, ABPChar_AI_C* character, FLinearColor color)
{
	FVector origin, extends;
	float sphereRadius;
	UKismetSystemLibrary::GetComponentBounds(character->RootComponent, &origin, &extends, &sphereRadius);
	auto rotation = character->K2_GetActorRotation();
	rotation.Pitch = rotation.Pitch * (-1);

	FVector one = FVector::ZeroVector;
	one.X -= extends.X;
	one.Y -= extends.Y;
	one.Z -= extends.Z;

	FVector two = FVector::ZeroVector;
	two.X += extends.X;
	two.Y -= extends.Y;
	two.Z -= extends.Z;

	FVector three = FVector::ZeroVector;
	three.X += extends.X;
	three.Y += extends.Y;
	three.Z -= extends.Z;

	FVector four = FVector::ZeroVector;
	four.Y += extends.Y;
	four.X -= extends.X;
	four.Z -= extends.Z;

	FVector five = one;
	five.Z += extends.Z * 2;

	FVector six = two;
	six.Z += extends.Z * 2;

	FVector seven = three;
	seven.Z += extends.Z * 2;

	FVector eight = four;
	eight.Z += extends.Z * 2;

	FVector oneR = one, twoR = two, threeR = three, fourR = four, fiveR = five, sixR = six, sevenR = seven, eightR = eight;
	rotate(one, rotation, oneR);
	rotate(two, rotation, twoR);
	rotate(three, rotation, threeR);
	rotate(four, rotation, fourR);
	rotate(five, rotation, fiveR);
	rotate(six, rotation, sixR);
	rotate(seven, rotation, sevenR);
	rotate(eight, rotation, eightR);

	oneR = oneR + origin;
	twoR = twoR + origin;
	threeR = threeR + origin;
	fourR = fourR + origin;
	fiveR = fiveR + origin;
	sixR = sixR + origin;
	sevenR = sevenR + origin;
	eightR = eightR + origin;

	FVector2D s1, s2, s3, s4, s5, s6, s7, s8;
	if (!localController->ProjectWorldLocationToScreen(oneR, false, s1))
		return;

	if (!localController->ProjectWorldLocationToScreen(twoR, false, s2))
		return;

	if (!localController->ProjectWorldLocationToScreen(threeR, false, s3))
		return;

	if (!localController->ProjectWorldLocationToScreen(fourR, false, s4))
		return;

	if (!localController->ProjectWorldLocationToScreen(fiveR, false, s5))
		return;

	if (!localController->ProjectWorldLocationToScreen(sixR, false, s6))
		return;

	if (!localController->ProjectWorldLocationToScreen(sevenR, false, s7))
		return;

	if (!localController->ProjectWorldLocationToScreen(eightR, false, s8))
		return;

	float thickness = 1.3f;

	canvas->DrawLine(s1, s2, thickness, color, false, false, Black);
	canvas->DrawLine(s2, s3, thickness, color, false, false, Black);
	canvas->DrawLine(s3, s4, thickness, color, false, false, Black);
	canvas->DrawLine(s4, s1, thickness, color, false, false, Black);

	canvas->DrawLine(s5, s6, thickness, color, false, false, Black);
	canvas->DrawLine(s6, s7, thickness, color, false, false, Black);
	canvas->DrawLine(s7, s8, thickness, color, false, false, Black);
	canvas->DrawLine(s8, s5, thickness, color, false, false, Black);

	canvas->DrawLine(s1, s5, thickness, color, false, true, Black);
	canvas->DrawLine(s2, s6, thickness, color, false, true, Black);
	canvas->DrawLine(s3, s7, thickness, color, false, true, Black);
	canvas->DrawLine(s4, s8, thickness, color, false, true, Black);
}

void DrawAimingLine(UCanvas* canvas, APlayerController* localController, FVector startLocation, FRotator charRotation)
{
	FVector angles = charRotation.ToVector();
	FVector end = angles * 500.0f + startLocation; //5 meters

	FVector2D startScreen, endScreen;
	if (!localController->ProjectWorldLocationToScreen(startLocation, true, startScreen))
		return;

	if (!localController->ProjectWorldLocationToScreen(end, true, endScreen))
		return;

	canvas->DrawLine(startScreen, endScreen, 1.5f, AimingLineColor, false, false, Black);
}

void DrawAimTrace(UWorld* world, UCanvas* canvas, APlayerController* localController, FVector cameraLocation, FRotator rotation)
{
	FVector end = rotation.ToVector() * 30000.0f + cameraLocation; //300 meters
	FVector2D screenCenter{ (float)(sizeX / 2) + 8.0f, (float)(sizeY / 2) + 15.0f };

	FHitResult hitResult;
	memset(&hitResult, 0, sizeof(FHitResult));
	auto hit = UKismetSystemLibrary::LineTraceSingle(world, cameraLocation, end, &hitResult);

	int distance = hitResult.Distance / 100;

	//if(hitResult.Actor.IsValid())
	//	auto getActor = hitResult.Actor.Get();

	if (drawAimTrace->Toggled && hit)
		canvas->UDrawTextS("Hit (" + std::to_string(distance) + "m)", screenCenter, ACCENT_COLOR, true);
}

float GetClosestToCenterVisibleBone(USkeletalMeshComponent* mesh, UWorld* world, FVector cameraLocation, FVector2D screenCenter, APlayerController* localController, bool closestToCenter, FVector& outPoint)
{
	const FMatrix comp2world = mesh->K2_GetComponentToWorld().ToMatrixWithScale();

	FVector closestPoint, boneOut;
	FVector2D s1, closestScreenPoint{ 0.0f, 0.0f };
	float closestDistance = 9999999999999.0f, tempDistance;

	for (int boneIndex = 0; boneIndex < mesh->ATransform.Num(); boneIndex++)
	{
		if (mesh->GetBone(boneIndex, comp2world, boneOut))
		{
			struct FHitResult hitResult;
			memset(&hitResult, 0, sizeof(FHitResult));
			auto hit = UKismetSystemLibrary::LineTraceSingle(world, cameraLocation, boneOut, &hitResult);

			if (hit)
				continue;

			if (closestToCenter)
			{
				if (!localController->ProjectWorldLocationToScreen(boneOut, true, s1))
					continue;

				tempDistance = s1.DistTo(screenCenter);

				if (closestScreenPoint.DistTo(screenCenter) > tempDistance)
				{
					closestPoint = boneOut;
					closestScreenPoint = s1;
					closestDistance = tempDistance;
				}
			}
			else
			{
				tempDistance = boneOut.DistTo(cameraLocation);

				if (closestPoint.DistTo(cameraLocation) > tempDistance)
				{
					closestPoint = boneOut;
					closestDistance = tempDistance;
				}
			}

		}
	}

	outPoint = closestPoint;

	return closestDistance;
}

void Aimbot(UWorld* world, UCanvas* canvas, TArray<AActor*> actors, FVector cameraLoc, FRotator cameraRot, APlayerController* localController, AActor* localActor)
{
	AActor* closestActor = nullptr;
	FVector closestPoint;
	FVector2D s1, closestScreenPoint{ 0.0f, 0.0f };
	static FVector2D screenCenter{ (float)(sizeX / 2), (float)(sizeY / 2) };
	static FVector2D screenSize{ (float)(sizeX), (float)(sizeY) };
	std::unordered_set <ValidTarget*> validTargets;
	float closestDistance = 9999999999.0f;

	for (int i = 0; i < actors.Num(); i++)
	{
		auto currentActor = actors[i];

		if (currentActor)
		{
			auto targetVector = cameraRot.ToVector() * 1000.0f + cameraLoc;

			if (currentActor->IsA(AVehicle::StaticClass()) && drawVehicles->Toggled)
			{
				DrawVehicles(canvas, cameraLoc, localController, (AVehicle*)currentActor);
			}

			if (currentActor->IsA(AIO_Mayhem2_ArcaneEncanter_C::StaticClass()))
			{
				auto mayhem2 = (AIO_Mayhem2_ArcaneEncanter_C*)currentActor;

				if (mayhem2->OakDamage->GetCurrentHealth() > 0.0f)
				{
					Draw2DBoxes(canvas, screenSize, cameraLoc, localController, currentActor, mayhem2->OakDamage, mayhem2->RootComponent, EnemyColor);

					if (magnet->Toggled)
					{
						currentActor->K2_TeleportTo(targetVector);
					}

					if (oneHit->Toggled)
					{
						mayhem2->OakDamage->SetCurrentShield(0.0f);
						mayhem2->OakDamage->SetCurrentHealth(1.0f);
					}

					FVector origin, extends;
					float sphereRadius;
					UKismetSystemLibrary::GetComponentBounds(currentActor->RootComponent, &origin, &extends, &sphereRadius);

					origin.X = origin.X + extends.X / 2;
					origin.Y = origin.Y + extends.Y / 2;
					origin.Z = origin.Z + extends.Z / 2;

					struct FHitResult hitResult;
					memset(&hitResult, 0, sizeof(FHitResult));
					auto hit = UKismetSystemLibrary::LineTraceSingle(world, cameraLoc, origin, &hitResult);

					if (!hit)
					{
						auto target = new ValidTarget();

						target->Point = origin;

						validTargets.insert(target);
					}
				}
			}

			if (currentActor->IsA(AInventoryItemPickup::StaticClass()))
			{
				DrawItemPickup(canvas, cameraLoc, localController, (AInventoryItemPickup*)currentActor);
			}

			if (currentActor->IsA(ABPChar_AI_C::StaticClass()))
			{
				auto aiChar = (ABPChar_AI_C*)currentActor;

				auto balanceComponent = (UAIBalanceStateComponent*)currentActor->GetComponentByClass(UAIBalanceStateComponent::StaticClass());

				if (balanceComponent)
				{
					if (balanceComponent->bIsAnointed)
					{
						if (balanceComponent->AnointedDeathState == EAnointedDeathState::Frozen && shatterFrozen->Toggled)
						{
							balanceComponent->ShatterFrozenAnointed();
						}

						if (balanceComponent->AnointedDeathState != EAnointedDeathState::PreFreeze)
						{
							continue;
						}
					}
				}

				if (aiChar->DamageComponent->GetCurrentHealth() <= 0.0f)
					continue;

				/*auto localTeamComponent = (UTeamComponent*)localActor->GetComponentByClass(UTeamComponent::StaticClass());*/

				auto entityColor = FriendlyColor;

				bool isHostile = false;

				/*if (localTeamComponent)
				{
					isHostile = localTeamComponent->IsHostile(currentActor);

					if (isHostile)
					{
						entityColor = EnemyColor;
					}
				}*/

				auto localTeam = ((AOakCharacter*)localActor)->GetTeam();
				auto currentTeam = aiChar->GetTeam();

				isHostile = localTeam->GetAttitudeToward(currentTeam) == ETeamAttitude::Hostile;

				if (isHostile)
				{
					entityColor = EnemyColor;
				}

				Draw2DBoxes(canvas, screenSize, cameraLoc, localController, aiChar, aiChar->DamageComponent, aiChar->RootComponent, entityColor);

				if (draw3DBoxes->Toggled)
					Draw3DBoxes(canvas, cameraLoc, localController, aiChar, entityColor);

				if (!isHostile)
					continue;

				if (oneHit->Toggled)
				{
					aiChar->DamageComponent->SetCurrentShield(0.0f);
					aiChar->DamageComponent->SetCurrentHealth(1.0f);
				}

				if (instaKill->Toggled)
				{
					aiChar->SetDead(EDeathType::HealthDepleted);
				};

				if (magnet->Toggled)
				{
					auto targetVector = cameraRot.ToVector() * 1000.0f + cameraLoc;

					aiChar->K2_TeleportTo(targetVector);
				}

				if (aiChar->Mesh)
				{
					auto target = new ValidTarget();
					target->Mesh = aiChar->Mesh;

					validTargets.insert(target);
				}
			}
		}
	}

	FVector pointOut;

	for (auto target : validTargets)
	{
		float distance = 9999.0f;

		if (target->Mesh)
		{
			distance = GetClosestToCenterVisibleBone(target->Mesh, world, cameraLoc, screenCenter, localController, aimbotNearestAim->Toggled, pointOut);
		}
		else
		{
			if (aimbotNearestAim->Toggled)
			{
				if (!localController->ProjectWorldLocationToScreen(target->Point, true, s1))
					continue;

				distance = s1.DistTo(screenCenter);
				pointOut = target->Point;
			}
			else
			{
				distance = target->Point.DistTo(cameraLoc);
				pointOut = target->Point;
			}
		}

		if (closestDistance > distance)
		{
			closestDistance = distance;
			closestPoint = pointOut;
		}

		delete target;
		target = NULL;
	}

	if ((localController->IsInputKeyDown(FKey("LeftAlt")) || aimbotEnabled->Toggled) && closestPoint.X != 0.0f && closestPoint.Y != 0.0f && closestPoint.Z != 0.0f)
	{
		auto delta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLoc, closestPoint), cameraRot);

		localController->AddPitchInput(delta.Pitch);
		localController->AddYawInput(delta.Yaw);

		canvas->K2_DrawBox({ screenCenter.X - 10.f, screenCenter.Y - 10.f }, { 20.0f, 20.0f }, 2.0f, AimbotSquareColor);

		UTestLibrary::SimulateInputKey(localController, FKey("LeftMouseButton"), EInputEvent::IE_Pressed, 1.0f);
		UTestLibrary::SimulateInputKey(localController, FKey("LeftMouseButton"), EInputEvent::IE_Released, 1.0f);
	}
}