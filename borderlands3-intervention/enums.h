#pragma once
#include"pch.h"

// Enum Engine.EBlendMode
enum EBlendMode : uint8_t 
{
	BLEND_Opaque = 0,
	BLEND_Masked = 1,
	BLEND_Translucent = 2,
	BLEND_Additive = 3,
	BLEND_Modulate = 4,
	BLEND_AlphaComposite = 5,
	BLEND_AlphaHoldout = 6,
	BLEND_MAX = 7
};

// Enum OakGame.EHealthState
enum EHealthState : uint8_t
{
	Healthy = 0,
	Low = 1,
	VeryLow = 2,
	Injured = 3,
	Dead = 4,
	EHealthState_MAX = 5
};

// Enum OakGame.EDeathType
enum EDeathType : uint8_t
{
	None = 0,
	HealthDepleted = 1,
	Suicide = 2,
	ScriptedInstantDeath = 3,
	FellOutOfWorld = 4,
	Ragdoll = 5,
	EDeathType_MAX = 6
};

// Enum Engine.ETraceTypeQuery
enum class ETraceTypeQuery : uint8_t 
{
	TraceTypeQuery1 = 0,
	TraceTypeQuery2 = 1,
	TraceTypeQuery3 = 2,
	TraceTypeQuery4 = 3,
	TraceTypeQuery5 = 4,
	TraceTypeQuery6 = 5,
	TraceTypeQuery7 = 6,
	TraceTypeQuery8 = 7,
	TraceTypeQuery9 = 8,
	TraceTypeQuery10 = 9,
	TraceTypeQuery11 = 10,
	TraceTypeQuery12 = 11,
	TraceTypeQuery13 = 12,
	TraceTypeQuery14 = 13,
	TraceTypeQuery15 = 14,
	TraceTypeQuery16 = 15,
	TraceTypeQuery17 = 16,
	TraceTypeQuery18 = 17,
	TraceTypeQuery19 = 18,
	TraceTypeQuery20 = 19,
	TraceTypeQuery21 = 20,
	TraceTypeQuery22 = 21,
	TraceTypeQuery23 = 22,
	TraceTypeQuery24 = 23,
	TraceTypeQuery25 = 24,
	TraceTypeQuery26 = 25,
	TraceTypeQuery27 = 26,
	TraceTypeQuery28 = 27,
	TraceTypeQuery29 = 28,
	TraceTypeQuery30 = 29,
	TraceTypeQuery31 = 30,
	TraceTypeQuery32 = 31,
	TraceTypeQuery_MAX = 32,
	ETraceTypeQuery_MAX = 33
};

// Enum Engine.ECollisionChannel
enum class ECollisionChannel : uint8_t
{
	ECC_WorldStatic = 0,
	ECC_WorldDynamic = 1,
	ECC_Pawn = 2,
	ECC_Visibility = 3,
	ECC_Camera = 4,
	ECC_PhysicsBody = 5,
	ECC_Vehicle = 6,
	ECC_Destructible = 7,
	ECC_EngineTraceChannel1 = 8,
	ECC_EngineTraceChannel2 = 9,
	ECC_EngineTraceChannel3 = 10,
	ECC_EngineTraceChannel4 = 11,
	ECC_EngineTraceChannel5 = 12,
	ECC_EngineTraceChannel6 = 13,
	ECC_GameTraceChannel1 = 14,
	ECC_GameTraceChannel2 = 15,
	ECC_GameTraceChannel3 = 16,
	ECC_GameTraceChannel4 = 17,
	ECC_GameTraceChannel5 = 18,
	ECC_GameTraceChannel6 = 19,
	ECC_GameTraceChannel7 = 20,
	ECC_GameTraceChannel8 = 21,
	ECC_GameTraceChannel9 = 22,
	ECC_GameTraceChannel10 = 23,
	ECC_GameTraceChannel11 = 24,
	ECC_GameTraceChannel12 = 25,
	ECC_GameTraceChannel13 = 26,
	ECC_GameTraceChannel14 = 27,
	ECC_GameTraceChannel15 = 28,
	ECC_GameTraceChannel16 = 29,
	ECC_GameTraceChannel17 = 30,
	ECC_GameTraceChannel18 = 31,
	ECC_OverlapAll_Deprecated = 32,
	ECC_MAX = 33
};

// Enum Engine.EDrawDebugTrace
enum class EDrawDebugTrace : uint8_t 
{
	None = 0,
	ForOneFrame = 1,
	ForDuration = 2,
	Persistent = 3,
	EDrawDebugTrace_MAX = 4
};

// Enum Engine.EBoneSpaces
enum class EBoneSpaces : uint8_t
{
	WorldSpace = 0,
	ComponentSpace = 1,
	EBoneSpaces_MAX = 2
};

// Enum Engine.TextureAddress
enum class ETextureAddress : uint8_t
{
	TA_Wrap = 0,
	TA_Clamp = 1,
	TA_Mirror = 2,
	TA_MAX = 3
};

// Enum Engine.EInputEvent
enum class EInputEvent : uint8_t
{
	IE_Pressed = 0,
	IE_Released = 1,
	IE_Repeat = 2,
	IE_DoubleClick = 3,
	IE_Axis = 4,
	IE_MAX = 5
};

// Enum GbxWeapon.EWeaponChargeState
enum class EWeaponChargeState : uint8_t
{
	None = 0,
	Discharging = 1,
	Charging = 2,
	Charged = 3,
	Overcharged = 4,
	EWeaponChargeState_MAX = 5
};

// Enum Engine.EMovementMode
enum class EMovementMode : uint8_t
{
	MOVE_None = 0,
	MOVE_Walking = 1,
	MOVE_NavWalking = 2,
	MOVE_Falling = 3,
	MOVE_Swimming = 4,
	MOVE_Flying = 5,
	MOVE_Custom = 6,
	MOVE_MAX = 7
};

// Enum OakGame.EAnointedDeathState
enum class EAnointedDeathState : uint8_t
{
	PreFreeze = 0,
	Freezing = 1,
	Frozen = 2,
	ShatterKilled = 3,
	TimedOut = 4,
	EAnointedDeathState_MAX = 5
};

// Enum AIModule.ETeamAttitude
enum class ETeamAttitude : uint8_t
{
	Friendly = 0,
	Neutral = 1,
	Hostile = 2,
	ETeamAttitude_MAX = 3
};
