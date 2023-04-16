#pragma once
#include "pch.h"

inline void ProcessEvent(void* obj, void* function, void* parms)
{
	auto vtable = *reinterpret_cast<void***>(obj);
	reinterpret_cast<void(*)(void*, void*, void*)>(vtable[0x41])(obj, function, parms);
}

class UEngine;
class UObject;
class UWorld;
class UKismetSystemLibrary;

extern UEngine* GEngine;

template<class TEnum>
class TEnumAsByte
{
public:
	constexpr TEnumAsByte() noexcept {}

	constexpr TEnumAsByte(TEnum _value) noexcept :
		value(static_cast<uint8_t>(_value))
	{
	}

	constexpr explicit TEnumAsByte(int32_t _value) noexcept :
		value(static_cast<uint8_t>(_value))
	{
	}

	constexpr explicit TEnumAsByte(uint8_t _value) noexcept :
		value(_value)
	{
	}

	constexpr operator TEnum() const noexcept
	{
		return (TEnum)value;
	}

	constexpr TEnum GetValue() const noexcept
	{
		return (TEnum)value;
	}

private:
	std::uint8_t value;
};

class FScriptInterface
{
private:
	UObject* ObjectPointer;
	void* InterfacePointer;

public:
	inline UObject* GetObject() const
	{
		return ObjectPointer;
	}

	inline UObject*& GetObjectRef()
	{
		return ObjectPointer;
	}

	inline void* GetInterface() const
	{
		return ObjectPointer != nullptr ? InterfacePointer : nullptr;
	}
};

template<class InterfaceType>
class TScriptInterface : public FScriptInterface
{
public:
	inline InterfaceType* operator->() const
	{
		return (InterfaceType*)GetInterface();
	}

	inline InterfaceType& operator*() const
	{
		return *((InterfaceType*)GetInterface());
	}

	inline operator bool() const
	{
		return GetInterface() != nullptr;
	}
};

template<class T>
class TArray
{
	friend class FString;

public:
	constexpr TArray() noexcept
	{
		Data = nullptr;
		Count = Max = 0;
	};

	constexpr auto Num() const noexcept
	{
		return Count;
	};

	constexpr auto& operator[](std::int32_t i) noexcept
	{
		return Data[i];
	};

	constexpr const auto& operator[](std::int32_t i) const noexcept
	{
		return Data[i];
	};

	constexpr auto IsValidIndex(std::int32_t i) const noexcept
	{
		return i < Num();
	}

	T* Data;
	std::int32_t Count;
	std::int32_t Max;
};

class UObject;

class FUObjectItem
{
	enum class EInternalObjectFlags : std::int32_t
	{
		None = 0,
		ReachableInCluster = 1 << 23,
		ClusterRoot = 1 << 24,
		Native = 1 << 25,
		Async = 1 << 26,
		AsyncLoading = 1 << 27,
		Unreachable = 1 << 28,
		PendingKill = 1 << 29,
		RootSet = 1 << 30,
		GarbageCollectionKeepFlags = Native | Async | AsyncLoading,
		AllFlags = ReachableInCluster | ClusterRoot | Native | Async | AsyncLoading | Unreachable | PendingKill | RootSet,
	};

public:
	class UObject* Object;
	std::int32_t Flags;
	std::int32_t ClusterRootIndex;
	std::int32_t SerialNumber;
	char pad_0014[4];

	[[nodiscard]] constexpr auto IsUnreachable() const noexcept
	{
		return !!(Flags & static_cast<std::underlying_type_t<EInternalObjectFlags>>(EInternalObjectFlags::Unreachable));
	}

	[[nodiscard]] constexpr auto IsPendingKill() const noexcept
	{
		return !!(Flags & static_cast<std::underlying_type_t<EInternalObjectFlags>>(EInternalObjectFlags::PendingKill));
	}
};

class FChunkedFixedUObjectArray
{
public:
	enum
	{
		NumElementsPerChunk = 64 * 1024
	};

	constexpr auto Num() const noexcept
	{
		return NumElements;
	}

	constexpr auto GetObjectPtr(std::size_t Index) const noexcept
	{
		const auto ChunkIndex = Index / NumElementsPerChunk;
		const auto WithinChunkIndex = Index % NumElementsPerChunk;
		return Objects[ChunkIndex] + WithinChunkIndex;
	}

	constexpr auto* GetByIndex(std::size_t Index) const noexcept
	{
		return GetObjectPtr(Index);
	}

	UObject* FindObject(const std::string& name) const;

private:
	class FUObjectItem** Objects;
	class FUObjectItem* PreAllocatedObjects;
	std::int32_t MaxElements;
	std::int32_t NumElements;
	std::int32_t MaxChunks;
	std::int32_t NumChunks;
};

class FUObjectArray
{
public:
	std::int32_t ObjFirstGCIndex;
	std::int32_t ObjLastNonGCIndex;
	std::int32_t MaxObjectsNotConsideredByGC;
	bool OpenForDisregardForGC;
	FChunkedFixedUObjectArray ObjObjects;
};

class FWeakObjectPtr
{
public:
	[[nodiscard]] auto IsValid() const noexcept -> bool;
	[[nodiscard]] auto Get() const noexcept -> class UObject*;

	std::int32_t ObjectIndex;
	std::int32_t ObjectSerialNumber;
};

template<class T, class TWeakObjectPtrBase = FWeakObjectPtr>
struct TWeakObjectPtr : private TWeakObjectPtrBase
{
public:
	constexpr auto Get() const noexcept -> T*
	{
		return reinterpret_cast<T*>(TWeakObjectPtrBase::Get());
	}

	[[nodiscard]] constexpr auto& operator*() const noexcept
	{
		return *Get();
	}

	[[nodiscard]] constexpr auto operator->() const noexcept
	{
		return Get();
	}

	[[nodiscard]] constexpr auto IsValid() const noexcept
	{
		return TWeakObjectPtrBase::IsValid();
	}
};

class FNameEntry
{
public:
	constexpr auto GetAnsiName() const noexcept
	{
		return AnsiName;
	}

	constexpr auto GetWideName() const noexcept
	{
		return WideName;
	}

	inline bool IsWide() const
	{
		return (Index & 0x1);
	}

	inline std::wstring ToWString() const
	{
		std::wstring value;

		if (IsWide()) {
			std::wstring ws(GetWideName());
			value = ws;
		}
		else {
			std::string string = GetAnsiName();
			std::wstring ws(string.begin(), string.end());
			value = ws;
		}

		auto pos = value.rfind('/');
		if (pos != std::string::npos)
		{
			value = value.substr(pos + 1);
		}

		return value;
	}

	inline std::string ToString() const
	{
		std::string value;

		if (IsWide()) {
			std::wstring ws(GetWideName());
			std::string out(ws.begin(), ws.end());
			value = out;
		}
		else {
			value = GetAnsiName();
		}

		auto pos = value.rfind('/');
		if (pos != std::string::npos)
		{
			value = value.substr(pos + 1);
		}

		return value;
	}

private:
	std::int32_t Index; //0x0000
	char pad_0004[4]; //0x0004
	class FNameEntry* HashNext; //0x0008

	union
	{
		char AnsiName[1024];
		wchar_t WideName[1024];
	};
};

class TNameEntryArray
{
public:
	constexpr auto Num() const noexcept
	{
		return NumElements;
	}

	constexpr auto IsValidIndex(int Index) const noexcept
	{
		return Index < Num() && Index >= 0;
	}

	auto* operator[](int Index) const noexcept
	{
		return GetItemPtr(Index);
	}

	auto GetItem(int Index) const noexcept
	{
		return GetItemPtr(Index);
	}

private:
	enum
	{
		ElementsPerChunk = 16 * 1024,
		ChunkTableSize = (2 * 1024 * 1024 + ElementsPerChunk - 1) / ElementsPerChunk
	};

	FNameEntry* GetItemPtr(size_t Index) const noexcept
	{
		const auto ChunkIndex = Index / ElementsPerChunk;
		const auto WithinChunkIndex = Index % ElementsPerChunk;
		const auto Chunk = Chunks[ChunkIndex];
		return *(FNameEntry**)(Chunk + WithinChunkIndex);
	}

	FNameEntry** Chunks[ChunkTableSize];
	std::int32_t NumElements;
	std::int32_t NumChunks;
};

class FTextData
{
public:
	char UnknownData[0x28];
	wchar_t* Name;
	__int32 Length;
};

struct FText
{
	FTextData* Data;
	char UnknownData[0x10];

	wchar_t* Get() const {
		if (Data)
			return Data->Name;

		return nullptr;
	}
};

class FName
{
public:
	static inline TNameEntryArray* GNames = nullptr;
	std::int32_t ComparisonIndex;
	std::int32_t Number;

	inline FName()
		: ComparisonIndex(0),
		Number(0)
	{
	};

	inline FName(int32_t i)
		: ComparisonIndex(i),
		Number(0)
	{
	};

	FName(const char* nameToFind)
		: ComparisonIndex(0),
		Number(0)
	{
		static std::unordered_set<int> cache;

		for (auto i : cache)
		{
			auto currentName = GNames->GetItem(i);

			if (currentName != nullptr)
			{
				if (!std::strcmp(currentName->GetAnsiName(), nameToFind))
				{
					ComparisonIndex = i;

					return;
				}
			}
		}

		for (auto i = 0; i < GNames->Num(); ++i)
		{
			auto currentName = GNames->GetItem(i);

			if (currentName != nullptr)
			{
				if (!std::strcmp(currentName->GetAnsiName(), nameToFind))
				{
					cache.insert(i);

					ComparisonIndex = i;

					return;
				}
			}
		}
	};

	static TNameEntryArray* GetGlobalNames() noexcept
	{
		return GNames;
	};

	std::string GetString() const noexcept
	{
		auto name = GNames->GetItem(ComparisonIndex);

		return name->ToString();
	};

	std::wstring GetWString() const noexcept
	{
		auto name = GNames->GetItem(ComparisonIndex);

		return name->ToWString();
	};

	constexpr auto operator==(const FName& other) const noexcept
	{
		return ComparisonIndex == other.ComparisonIndex;
	};
};

class UClass;

class UObject
{
public:
	static inline FChunkedFixedUObjectArray* GObjects = nullptr;
	void** VTable;//0x00
	uint32_t ObjectFlags;//0x08
	uint32_t InternalIndex;//0x0C
	UClass* ClassPrivate;
	FName NamePrivate;
	UObject* OuterPrivate;

	std::string GetFullName() const;

	template<typename T>
	static T* FindObject(const std::string& name)
	{
		for (int i = 0; i < GObjects->Num(); ++i)
		{
			auto object = GObjects->GetByIndex(i)->Object;

			if (object == nullptr)
			{
				continue;
			}

			if (object->GetFullName() == name)
			{
				return static_cast<T*>(object);
			}
		}
		return nullptr;
	}

	template<typename T>
	static T* GetObjectByIndex(std::size_t Index)
	{
		for (int i = 0; i < GObjects->Num(); ++i)
		{
			auto object = GObjects->GetByIndex(i);

			return static_cast<T*>(object);
		}
		return nullptr;
	}

	bool IsA(UClass* cmp) const;
};

class UField : public UObject
{
public:
	UField* Next;
};

class UStruct : public UField
{
public:
	UStruct* SuperStruct; // 0x30(0x8)
	UField* Children; // 0x38(0x08)
	uint32_t PropertiesSize;// 0x40(0x04)
	char pad_0044[0x44]; // 0x54(0x88)
};

class UClass : public UStruct
{
public:
	char pad_00B0[0x180]; // 0xb0(0x180)
};

class FString : public TArray<wchar_t>
{
public:
	constexpr FString() noexcept {};

	constexpr FString(const wchar_t* other) noexcept
	{
		Max = Count = *other ? static_cast<std::int32_t>(std::wcslen(other)) + 1 : 0;

		if (Count)
			Data = const_cast<wchar_t*>(other);
	};

	constexpr auto IsValid() const noexcept
	{
		return Data != nullptr;
	}

	constexpr auto c_str() const noexcept
	{
		return Data;
	}

	std::string ToString() const noexcept
	{
		const auto length = std::wcslen(Data);
		std::string str(length, '\0');
		std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);
		return str;
	}
};

struct FGbxAttributeFloat
{
	PAD(0x4);
	float Value;		// 0x0004(0x0004) (ZeroConstructor, Transient, IsPlainOldData)
	float BaseValue;	// 0x0008(0x0004) (Edit, ZeroConstructor, IsPlainOldData, RepSkip, RepNotify, Interp, NonTransactional, EditorOnly, NoDestructor, AutoWeak, ContainsInstancedReference, AssetRegistrySearchable, SimpleDisplay, AdvancedDisplay, Protected, BlueprintCallable, BlueprintAuthorityOnly, TextExportTransient, NonPIEDuplicateTransient, ExposeOnSpawn, PersistentInstance, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic, NativeAccessSpecifierProtected, NativeAccessSpecifierPrivate)
};

struct FGbxAttributeInteger
{
	PAD(0x4);
	int Value;		// 0x0004(0x0004) (ZeroConstructor, Transient, IsPlainOldData)
	int BaseValue;	// 0x0008(0x0004) (Edit, ZeroConstructor, IsPlainOldData, RepSkip, RepNotify, Interp, NonTransactional, EditorOnly, NoDestructor, AutoWeak, ContainsInstancedReference, AssetRegistrySearchable, SimpleDisplay, AdvancedDisplay, Protected, BlueprintCallable, BlueprintAuthorityOnly, TextExportTransient, NonPIEDuplicateTransient, ExposeOnSpawn, PersistentInstance, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic, NativeAccessSpecifierProtected, NativeAccessSpecifierPrivate)
};

// Class Engine.Texture
// 0x0090 (0x00B8 - 0x0028)
class UTexture : public UObject
{
public:
	PAD(0x90);

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class Engine.Texture");

		return ptr;
	}
};

// Class Engine.TextureStreaming
// 0x0040 (0x00F8 - 0x00B8)
class UTextureStreaming : public UTexture
{
public:
	PAD(0x40);

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class Engine.TextureStreaming");

		return ptr;
	}
};

// Class Engine.Texture2D
// 0x0008 (0x0100 - 0x00F8)
class UTexture2D : public UTextureStreaming
{
public:
	TEnumAsByte<ETextureAddress> AddressX;	// 0x00F8(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	TEnumAsByte<ETextureAddress> AddressY;	// 0x00F9(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	PAD(0x6);								// 0x00FA(0x0006) MISSED OFFSET

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class Engine.Texture2D");

		return ptr;
	}

	int Blueprint_GetSizeY()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.Texture2D.Blueprint_GetSizeY");

		int ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);

		return ReturnValue;
	}

	int Blueprint_GetSizeX()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.Texture2D.Blueprint_GetSizeX");

		int ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);

		return ReturnValue;
	}
};

// Class Engine.Font
// 0x01A8 (0x01D0 - 0x0028)
class UFont : public UObject
{
public:
	PAD(0x30);						// 0x0028(0x0008) MISSED OFFSET
	int IsRemapped;                 // 0x0058(0x0004) (ZeroConstructor, IsPlainOldData)
	float EmScale;					// 0x005C(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	float Ascent;					// 0x0060(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	float Descent;					// 0x0064(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	float Leading;					// 0x0068(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	int Kerning;					// 0x006C(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	PAD(0xB0);						// 0x0070(0x00B0) (Edit)
	int NumCharacters;				// 0x0120(0x0004) (ZeroConstructor, Transient, IsPlainOldData)
	PAD(0x4);						// 0x0124(0x0004) MISSED OFFSET
	TArray<int> MaxCharHeight;		// 0x0128(0x0010) (ZeroConstructor, Transient)
	float ScalingFactor;			// 0x0138(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	int LegacyFontSize;				// 0x013C(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	class FName LegacyFontName;	// 0x0140(0x0008) (Edit, ZeroConstructor, IsPlainOldData)
	PAD(0x88);						// 0x0148(0x0038)
};

// Class Engine.ActorComponent
// 0x0150 (0x0178 - 0x0028)
class UActorComponent : public UObject
{
public:
	PAD(0x150);

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class Engine.ActorComponent");

		return ptr;
	}

	class AActor* GetOwner()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.ActorComponent.GetOwner");

		AActor* ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);

		return ReturnValue;
	}
};

// Class OakGame.AIBalanceStateComponent
// 0x0160 (0x0338 - 0x01D8)
class UAIBalanceStateComponent : public UActorComponent
{
public:
	PAD(0x178);								// 0x01D8(0x0178) (Edit, ZeroConstructor)
	bool bIsAnointed;						// 0x02F0(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	PAD(0x37);								// 0x02F1(0x0007) MISSED OFFSET
	EAnointedDeathState AnointedDeathState;	// 0x0328(0x0001) (ZeroConstructor, Transient, IsPlainOldData)
	PAD(0xF);								// 0x0329(0x000F) MISSED OFFSET

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindObject<UClass>("Class OakGame.AIBalanceStateComponent");

		return ptr;
	}

	void ShatterFrozenAnointed()
	{
		static auto fn = UObject::FindObject<UObject>("Function OakGame.AIBalanceStateComponent.ShatterFrozenAnointed");

		struct {} param;

		ProcessEvent(this, fn, &param);
	}

};

// Class Engine.SceneComponent
// 0x0158 (0x02D0 - 0x0178)
class USceneComponent : public UActorComponent
{
public:
	PAD(0x74);							// 0x0178(0x0008) MISSED OFFSET
	class FVector RelativeLocation;	// 0x01EC(0x000C) (Edit, BlueprintVisible, BlueprintReadOnly, Net, IsPlainOldData)
	class FRotator RelativeRotation;	// 0x01F8(0x000C) (Edit, BlueprintVisible, BlueprintReadOnly, Net, IsPlainOldData)
	class FVector RelativeScale3D;		// 0x0204(0x000C) (Edit, BlueprintVisible, BlueprintReadOnly, Net, IsPlainOldData)
	PAD(0x30);							// 0x0210(0x0030) MISSED OFFSET
	class FVector ComponentVelocity;	// 0x0240(0x000C) (Transient, IsPlainOldData)
	PAD(0x84);							// 0x024C(0x0001) (Transient)

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class Engine.SceneComponent");

		return ptr;
	}
};

// Class GbxWeapon.GbxAmmoTypeData
// 0x0040 (0x0070 - 0x0030)
class UGbxAmmoTypeData
{
public:
	PAD(0x30);
	struct FText DisplayName;				// 0x0030(0x0028) (Edit, DisableEditOnInstance)
	class UTexture2D* DisplayIcon;			// 0x0048(0x0008) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData)
	class FString ScaleformDisplayFrameID;	// 0x0050(0x0010) (Edit, ZeroConstructor, DisableEditOnInstance)
	PAD(0x8);								// 0x0060(0x0008) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData)
	float RegenerationRate;					// 0x0068(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData)
	PAD(0x4);								// 0x006C(0x0004) MISSED OFFSET
};


// Class GbxWeapon.WeaponAmmoComponent
// 0x0038 (0x01B0 - 0x0178)
class UWeaponAmmoComponent : public UActorComponent
{
public:
	PAD(0x8);								// 0x0178(0x0008) MISSED OFFSET
	class UGbxAmmoTypeData* AmmoTypeData;	// 0x0180(0x0008) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData)
	PAD(0x20);								// 0x0188(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	class AWeapon* WeaponPrivate;			// 0x01A8(0x0008) (ZeroConstructor, Transient, IsPlainOldData)

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class GbxWeapon.WeaponAmmoComponent");

		return ptr;
	}
};

// Class Engine.Actor
// 0x0430 (0x0458 - 0x0028)
class AActor : public UObject
{
public:
	PAD(0x0140);												// 0x0028(0x0268)
	class USceneComponent* RootComponent;						// 0x0168(0x0008)
	PAD(0x01A0);												// 0x0170(0x0268)
	TArray<class UActorComponent*> InstanceComponents;			// 0x0310(0x0010) (ExportObject, ZeroConstructor)
	TArray<class UActorComponent*> BlueprintCreatedComponents;	// 0x0320(0x0010) (ExportObject, ZeroConstructor)
	PAD(0x0128);												// 0x0330(0x0100)

	class FRotator K2_GetActorRotation()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.Actor.K2_GetActorRotation");

		FRotator rotation;
		ProcessEvent(this, fn, &rotation);

		return rotation;
	};

	class FVector K2_GetActorLocation()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.Actor.K2_GetActorLocation");

		FVector location;
		ProcessEvent(this, fn, &location);

		return location;
	}

	bool K2_TeleportTo(const class FVector& DestLocation)
	{
		static UObject* fn = UObject::FindObject<UObject>("Function Engine.Actor.K2_TeleportTo");

		struct {
			FVector DestLocation;
			FRotator DestRotation;
			bool ReturnValue;
		} params;

		params = { DestLocation , K2_GetActorRotation(), false };

		ProcessEvent(this, fn, &params);

		return params.ReturnValue;
	}

	void SetActorEnableCollision(bool enableCollision)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.Actor.SetActorEnableCollision");

		bool bNewActorEnableCollision = enableCollision;

		ProcessEvent(this, fn, &bNewActorEnableCollision);
	}

	bool GetActorEnableCollision()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.Actor.GetActorEnableCollision");

		bool ReturnValue = false;

		ProcessEvent(this, fn, &ReturnValue);

		return ReturnValue;
	}

	class UActorComponent* GetComponentByClass(class UClass* ComponentClass)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.Actor.GetComponentByClass");

		struct
		{
			UClass* ComponentClass;
			UActorComponent* ReturnValue;
		} params;

		params.ComponentClass = ComponentClass;

		ProcessEvent(this, fn, &params);

		return params.ReturnValue;
	}

	void DisableInput(class APlayerController* PlayerController)
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.Actor.DisableInput");

		struct
		{
			APlayerController* PlayerController;
		} params;

		ProcessEvent(this, fn, &params);
	}

	void EnableInput(class APlayerController* PlayerController)
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.Actor.EnableInput");

		struct
		{
			APlayerController* PlayerController;
		} params;

		ProcessEvent(this, fn, &params);
	}
};

// ScriptStruct Engine.HitResult
// 0x0090
struct FHitResult
{
	unsigned char bBlockingHit : 1;			// 0x0000(0x0001)
	unsigned char bStartPenetrating : 1;	// 0x0000(0x0001)
	PAD(0x3);		 						// 0x0001(0x0003) MISSED OFFSET
	int FaceIndex;							// 0x0004(0x0004) (ZeroConstructor, IsPlainOldData)
	unsigned char bLagCompensated : 1;		// 0x0008(0x0001)
	PAD(0x3);		 						// 0x0009(0x0003) MISSED OFFSET
	float Time;								// 0x000C(0x0004) (ZeroConstructor, IsPlainOldData)
	float Distance;							// 0x0010(0x0004) (ZeroConstructor, IsPlainOldData)
	class FVector Location;				// 0x0014(0x000C)
	class FVector ImpactPoint;				// 0x0020(0x000C)
	class FVector Normal;					// 0x002C(0x000C)
	class FVector ImpactNormal;			// 0x0038(0x000C)
	class FVector TraceStart;				// 0x0044(0x000C)
	class FVector TraceEnd;				// 0x0050(0x000C)                                         
	float PenetrationDepth;					// 0x005C(0x0004) (ZeroConstructor, IsPlainOldData)
	int Item;								// 0x0060(0x0004) (ZeroConstructor, IsPlainOldData)
	void* Physics;
	TWeakObjectPtr<class AActor>                       Actor;                                                    // 0x006C(0x0008) (ZeroConstructor, IsPlainOldData)
	TWeakObjectPtr<class UPrimitiveComponent>          Component;                                                // 0x0074(0x0008) (ExportObject, ZeroConstructor, InstancedReference, IsPlainOldData)
	PAD(0x4);								// 0x007C(0x0004) MISSED OFFSET
	//TWeakObjectPtr<class UPhysicalMaterial>            PhysMaterial;                                             // 0x0064(0x0008) (ZeroConstructor, IsPlainOldData)
	class FName BoneName;					// 0x0080(0x0008) (ZeroConstructor, IsPlainOldData)
	class FName MyBoneName;				// 0x0088(0x0008) (ZeroConstructor, IsPlainOldData)
};

// Class OakGame.InteractiveObject
// 0x00F8 (0x0550 - 0x0458)
class AInteractiveObject : public AActor
{
public:
	PAD(0xC0);													// 0x0458(0x0060) MISSED OFFSET
	class USkeletalMeshComponent* PrimarySkeletalMeshComponent;	// 0x0518(0x0008) (ExportObject, ZeroConstructor, Transient, InstancedReference, IsPlainOldData)
	PAD(0x030);													// 0x0520(0x0060) MISSED OFFSET
};

// BlueprintGeneratedClass IO_Mayhem2_ArcaneEncanter.IO_Mayhem2_ArcaneEncanter_C
// 0x0118 (0x0668 - 0x0550)
class AIO_Mayhem2_ArcaneEncanter_C : public AInteractiveObject
{
public:
	PAD(0x20);								// 0x0550(0x0008) (Transient, DuplicateTransient)
	class USceneComponent* Sphere;			// 0x0570(0x0008) (Transient, DuplicateTransient)
	PAD(0x8);								// 0x0578(0x0008) (Transient, DuplicateTransient)
	class UDamageComponent* OakDamage;		// 0x0580(0x0008) (BlueprintVisible, ZeroConstructor, InstancedReference, IsPlainOldData)
	PAD(0x40);								// 0x0588(0x0008) (BlueprintVisible, ZeroConstructor, InstancedReference, IsPlainOldData)
	float Duration;							// 0x05C8(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	PAD(0x9C);								// 0x05CC(0x0004) MISSED OFFSET

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindObject<UClass>("BlueprintGeneratedClass IO_Mayhem2_ArcaneEncanter.IO_Mayhem2_ArcaneEncanter_C");

		return ptr;
	}

	void Explode()
	{
		static auto fn = UObject::FindObject<UObject>("Function IO_Mayhem2_ArcaneEncanter.IO_Mayhem2_ArcaneEncanter_C.Explode");

		struct {} param;

		ProcessEvent(this, fn, &param);
	}

};

// Class GbxInventory.InventoryCategoryData
// 0x0038 (0x0070 - 0x0038)
class UInventoryCategoryData
{
public:
	PAD(0x3A);                                  // 0x0038(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	bool SharedWithAllPlayers;					// 0x003A(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	bool bSharedPlayersMustBeOnFriendlyTeam;	// 0x003B(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	bool bLevelSyncPickupCurrency;				// 0x003C(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	bool ManualActivation;						// 0x003D(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	PAD(0x2);                                   // 0x003E(0x0002) MISSED OFFSET
	struct FText CategoryDisplayName;			// 0x0040(0x0028) (Edit)
	bool bCanInspectItem;						// 0x0058(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	bool bCanInspectManufacturer;				// 0x0059(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	bool bCanInspectElementalEffects;			// 0x005A(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	bool bCanInspectParts;						// 0x005B(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	bool bCanChangeSkins;						// 0x005C(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	bool bUseEquipAudioFromWeaponType;			// 0x005D(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	bool bDoNotUseCosmeticLabelInBackpackCells;	// 0x005E(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	PAD(0x11);                                  // 0x005F(0x0001) MISSED OFFSET
};

// Class GbxInventory.InventoryData
// 0x02D0 (0x0300 - 0x0030)
class UInventoryData
{
public:
	PAD(0x40);											// 0x0030(0x0008) MISSED OFFSET
	struct FText InventoryName;							// 0x0040(0x0028) (Edit, DisableEditOnTemplate)
	PAD(0x180);                                         // 0x0058(0x0010) (Edit, ZeroConstructor, DisableEditOnTemplate)
	class UInventoryCategoryData* InventoryCategory;	// 0x01D8(0x0008) (Edit, ZeroConstructor, IsPlainOldData)
	PAD(0x120);											// 0x01E0(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
};

// Class GbxInventory.InventoryItemPickup
// 0x0190 (0x05E8 - 0x0458)
class AInventoryItemPickup : public AActor
{
public:
	PAD(0x30);												// 0x0458(0x0008) (Edit, ZeroConstructor, IsPlainOldData)
	class USkeletalMeshComponent* PickupSkelMesh;			// 0x0488(0x0008) (BlueprintVisible, ExportObject, BlueprintReadOnly, ZeroConstructor, Transient, InstancedReference, IsPlainOldData)
	PAD(0x10);												// 0x0490(0x0010) (ExportObject, ZeroConstructor, Transient)
	int Quantity;											// 0x04A0(0x0004) (ZeroConstructor, Transient, IsPlainOldData)
	unsigned char bCanOnlyBePickedUpByOwnerController : 1;	// 0x04A4(0x0001) (BlueprintVisible, BlueprintReadOnly, Net, Transient)
	unsigned char bHighDetail : 1;							// 0x04A4(0x0001) (Edit)
	unsigned char bSharedWithAllPlayers : 1;				// 0x04A4(0x0001) (Edit)
	unsigned char bSharedPlayersMustBeOnFriendlyTeam : 1;	// 0x04A4(0x0001) (Edit)
	unsigned char bReplicatePickupActor : 1;				// 0x04A4(0x0001) (Edit)
	unsigned char StaticPickup : 1;							// 0x04A4(0x0001) (Edit)
	unsigned char bUseInventoryDataPickupActionType : 1;	// 0x04A4(0x0001) (Edit)
	PAD(0xB);												// 0x04A5(0x0003) MISSED OFFSET
	float MaxDrawDistance;									// 0x04B0(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	PAD(0xC);												// 0x04B4(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	class UInventoryCategoryData* PickupCategory;			// 0x04C0(0x0008) (BlueprintVisible, BlueprintReadOnly, Net, ZeroConstructor, Transient, IsPlainOldData)
	PAD(0x40);												// 0x04C8(0x0010) (ZeroConstructor, InstancedReference, BlueprintAssignable)
	class FVector StaticLocation;							// 0x0508(0x000C) (IsPlainOldData)
	class FRotator StaticRotation;							// 0x0514(0x000C) (IsPlainOldData)
	bool NotAddedToInventory;								// 0x0520(0x0001) (ZeroConstructor, Transient, IsPlainOldData)
	PAD(0x3);												// 0x0521(0x0003) MISSED OFFSET
	float PickupSphereRadius;								// 0x0524(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	float PickupSphereRadiusHideScale;						// 0x0528(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	PAD(0xC);												// 0x052C(0x0008) (ExportObject, ZeroConstructor, Transient, InstancedReference, IsPlainOldData)
	class UInventoryData* AssociatedInventoryData;			// 0x0538(0x0008) (ZeroConstructor, Transient, IsPlainOldData)
	PAD(0x68);												// 0x0540(0x0028) (BlueprintVisible, BlueprintReadOnly, Transient)
	unsigned char bIsActive : 1;							// 0x05A8(0x0001) (BlueprintVisible, BlueprintReadOnly, Net)
	PAD(0x3F);												// 0x05A9(0x0003) MISSED OFFSET

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class GbxInventory.InventoryItemPickup");

		return ptr;
	}

	FLinearColor GetInventoryRarityColorOutline()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function GbxInventory.InventoryItemPickup.GetInventoryRarityColorOutline");

		FLinearColor ReturnValue;

		ProcessEvent(this, fn, &ReturnValue);

		return ReturnValue;
	}

	FLinearColor GetInventoryRarityColorFX()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function GbxInventory.InventoryItemPickup.GetInventoryRarityColorFX");

		FLinearColor ReturnValue;

		ProcessEvent(this, fn, &ReturnValue);

		return ReturnValue;
	}
};

// Class GbxInventory.DroppedInventoryItemPickup
// 0x0078 (0x0660 - 0x05E8)
class ADroppedInventoryItemPickup : public AInventoryItemPickup
{
public:
	PAD(0x24);				// 0x05E8(0x000C) (Transient)
	int DroppedQuantity;	// 0x060C(0x0004) (Net, ZeroConstructor, Transient, IsPlainOldData)
	PAD(0x50);				// 0x0610(0x0004) (Net, ZeroConstructor, Transient, IsPlainOldData)
};

// Class GbxInventory.InventoryRarityData
// 0x0118 (0x0148 - 0x0030)
class UInventoryRarityData
{
public:
	PAD(0x30);								// 0x0000(0x0030) (Edit, IsPlainOldData)
	class FLinearColor RarityColorFX;		// 0x0030(0x0010) (Edit, IsPlainOldData)
	class FLinearColor RarityColorOutline;	// 0x0040(0x0010) (Edit, IsPlainOldData)
	int RarityOutlineDepthStencilValue;		// 0x0050(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	bool DisplayRarityOutline;				// 0x0054(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	PAD(0x1B);								// 0x0055(0x0003) MISSED OFFSET
	struct FText RarityDisplayname;			// 0x0070(0x0028) (Edit)
	int RaritySortValue;					// 0x0088(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	PAD(0x4);								// 0x008C(0x0004) MISSED OFFSET
	float RarityLootBeamHeight;				// 0x00A0(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	PAD(0xA4);								// 0x00A4(0x0004) MISSED OFFSET
};

// Class OakGame.OakInventoryItemPickup
// 0x00C8 (0x0728 - 0x0660)
class AOakInventoryItemPickup : public ADroppedInventoryItemPickup
{
public:
	PAD(0xB0);													// 0x0660(0x0018) MISSED OFFSET
	class UInventoryRarityData* AssociatedInventoryRarityData;	// 0x0710(0x0008) (ZeroConstructor, Transient, IsPlainOldData)
	PAD(0x10);													// 0x0718(0x0008) (ExportObject, ZeroConstructor, Transient, InstancedReference, IsPlainOldData)

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class OakGame.OakInventoryItemPickup");

		return ptr;
	}
};

// BlueprintGeneratedClass BP_OakInventoryItemPickup.BP_OakInventoryItemPickup_C
// 0x0008 (0x0730 - 0x0728)
class ABP_OakInventoryItemPickup_C : public AOakInventoryItemPickup
{
public:
	PAD(0x8);	// 0x0728(0x0008) (BlueprintVisible, ZeroConstructor, InstancedReference, IsPlainOldData)

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("BlueprintGeneratedClass BP_OakInventoryItemPickup.BP_OakInventoryItemPickup_C");

		return ptr;
	}
};

// BlueprintGeneratedClass BP_OakWeaponPickup.BP_OakWeaponPickup_C
// 0x0000 (0x0730 - 0x0730)
class ABP_OakWeaponPickup_C : public ABP_OakInventoryItemPickup_C
{
public:

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("BlueprintGeneratedClass BP_OakWeaponPickup.BP_OakWeaponPickup_C");

		return ptr;
	}

};

// BlueprintGeneratedClass BP_OakConsumableItemPickup.BP_OakConsumableItemPickup_C
// 0x0008 (0x0738 - 0x0730)
class ABP_OakConsumableItemPickup_C : public ABP_OakInventoryItemPickup_C
{
public:
	PAD(0x8);	// 0x0730(0x0008) (Transient, DuplicateTransient)

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("BlueprintGeneratedClass BP_OakConsumableItemPickup.BP_OakConsumableItemPickup_C");

		return ptr;
	}
};

// Class Engine.PrimitiveComponent
// 0x0420 (0x06F0 - 0x02D0)
class UPrimitiveComponent : public USceneComponent
{
public:
	PAD(0x0420);
};

// Class Engine.ModelComponent
// 0x0040 (0x0730 - 0x06F0)
class UModelComponent : public UPrimitiveComponent
{
public:
	PAD(0x40);
};

// Class Engine.Level
// 0x02F8 (0x0320 - 0x0028)
class ULevel : public UObject
{
public:
	PAD(0x78);										// 0x0028(0x0078)
	TArray<class AActor*> Actors;					// 0x00A0(0x0010)
	TArray<class UModelComponent*> ModelComponents;	// 0x00B0(0x0078)
	PAD(0x260);										// 0x00C0(0x0270)
};

// Class Engine.PlayerState
// 0x00D0 (0x0528 - 0x0458)
class APlayerState// : public AInfo
{
public:
	PAD(0x0458);
	float Score;						// 0x0458(0x0004) (BlueprintVisible, BlueprintReadOnly, Net, ZeroConstructor, IsPlainOldData)
	unsigned char Ping;					// 0x045C(0x0001) (BlueprintVisible, BlueprintReadOnly, Net, ZeroConstructor, IsPlainOldData)
	PAD(0x3);							// 0x045D(0x0003) MISSED OFFSET
	class FString PlayerName;			// 0x0460(0x0010) (BlueprintVisible, BlueprintReadOnly, ZeroConstructor)
	PAD(0x088);							// 0x0470(0x0010) MISSED OFFSET
	class FString PlayerNamePrivate;	// 0x04F8(0x0010) (Net, ZeroConstructor)
	PAD(0x020);							// 0x0508(0x0010) MISSED OFFSET
};

// Class Engine.GameModeBase
// 0x00A8 (0x0500 - 0x0458)
class AGameModeBase
{
public:
	PAD(0x458);						// 0x0000
	class FString OptionsString;	// 0x0458(0x0010) (BlueprintVisible, BlueprintReadOnly, ZeroConstructor)
	PAD(0x20);						// 0x0468(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	class UClass* HUDClass;			// 0x0488(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	PAD(0x70);						// 0x0490(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
};

// Class Engine.GameMode
// 0x0048 (0x0548 - 0x0500)
class AGameMode : public AGameModeBase
{
public:
	class FName MatchState;            // 0x0500(0x0008) (ZeroConstructor, Transient, IsPlainOldData)
	unsigned char bDelayedStart : 1;	// 0x0508(0x0001) (Edit, BlueprintVisible, BlueprintReadOnly)
	PAD(0x3);							// 0x0509(0x0003) MISSED OFFSET
	int NumSpectators;                  // 0x050C(0x0004) (BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	int NumPlayers;                     // 0x0510(0x0004) (BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	int NumBots;                        // 0x0514(0x0004) (BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	float MinRespawnDelay;              // 0x0518(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	int NumTravellingPlayers;           // 0x051C(0x0004) (BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	PAD(0x1);                           // 0x0520(0x0008) (ZeroConstructor, IsPlainOldData)
};

// Class Engine.GameStateBase
// 0x00C8 (0x0520 - 0x0458)
class AGameStateBase// : public AInfo
{
public:
	PAD(0x460);									// 0x0458(0x0008)
	class AGameModeBase* AuthorityGameMode;		// 0x0460(0x0008)
	PAD(0x8);									// 0x0468(0x0008)
	TArray<class APlayerState*> PlayerArray;	// 0x0470(0x0010)
	PAD(0xA0);									// 0x0480(0x00A0)
};

// Class Engine.Pawn
// 0x0068 (0x04C0 - 0x0458)
class APawn : public AActor
{
public:
	PAD(0x0068); //0x0458
};

// Class Engine.SkeletalMeshComponent
// 0x04F0 (0x0F20 - 0x0A30)
class USkeletalMeshComponent
{
public:
	PAD(0x728);													// 0x0A30(0x0008) MISSED OFFSET	
	TArray<class FTransform> ATransform;
	TArray<class FTransform> BTransform;
	TArray<class FTransform> CTransform;
	TArray<class FTransform> DTransform;						// 0x0758(0x0010) MISSED OFFSET
	PAD(0x320);
	TArray<class FTransform> ETransform;						// 0x0A88(0x0010) MISSED OFFSET
	PAD(0x38);													// 0x0A98(0x0038) MISSED OFFSET
	TArray<class FTransform> CachedBoneSpaceTransforms;		// 0x0AD0(0x0010) (ZeroConstructor, Transient)
	TArray<class FTransform> CachedComponentSpaceTransforms;	// 0x0AE0(0x0010) (ZeroConstructor, Transient)
	PAD(0x430);													// 0x0AF0(0x0430) MISSED OFFSET

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class Engine.SkeletalMeshComponent");

		return ptr;
	}

	FName GetBoneName(int boneIndex)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.SkinnedMeshComponent.GetBoneName");

		struct {
			int BoneIndex;
			PAD(0x4);
			FName ReturnValue;
		} params;

		params = { boneIndex };

		ProcessEvent(this, fn, &params);

		return params.ReturnValue;
	}

	int GetBoneIndex(const class FName& BoneName)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.SkinnedMeshComponent.GetBoneIndex");

		struct {
			FName BoneName;
			int ReturnValue;
		} params;

		params = { BoneName };

		ProcessEvent(this, fn, &params);

		return params.ReturnValue;
	}

	FTransform K2_GetComponentToWorld()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.SceneComponent.K2_GetComponentToWorld");

		FTransform CompToWorld;
		ProcessEvent(this, fn, &CompToWorld);
		return CompToWorld;
	}

	bool GetBoneByName(FName boneName, const FMatrix& componentToWorld, FVector& pos)
	{
		auto boneIndex = GetBoneIndex(boneName);

		if (boneIndex >= ATransform.Num())
			return false;

		const auto& bone = ATransform[boneIndex];
		auto boneMatrix = bone.ToMatrixWithScale();
		auto world = boneMatrix * componentToWorld;

		pos =
		{
			world.M[3][0],
			world.M[3][1],
			world.M[3][2]
		};

		return true;
	}

	bool GetBone(const uint32_t id, const FMatrix& componentToWorld, FVector& pos)
	{
		if (id >= ATransform.Num())
			return false;

		const auto& bone = ATransform[id];
		auto boneMatrix = bone.ToMatrixWithScale();
		auto world = boneMatrix * componentToWorld;

		pos =
		{
			world.M[3][0],
			world.M[3][1],
			world.M[3][2]
		};

		return true;
	}
};

// ScriptStruct GbxGameSystemCore.DamageInfo
// 0x0058
struct FDamageInfo
{
	float Damage;							// 0x0000(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, RepSkip, RepNotify, Interp, NonTransactional, EditorOnly, NoDestructor, AutoWeak, ContainsInstancedReference, AssetRegistrySearchable, SimpleDisplay, AdvancedDisplay, Protected, BlueprintCallable, BlueprintAuthorityOnly, TextExportTransient, NonPIEDuplicateTransient, ExposeOnSpawn, PersistentInstance, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic, NativeAccessSpecifierProtected, NativeAccessSpecifierPrivate)
	PAD(0x14);								// 0x0004(0x0004) MISSED OFFSET
	float DamageRadius;						// 0x0018(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float ImpactForce;						// 0x001C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	PAD(0x28);								// 0x0020(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, RepSkip, RepNotify, Interp, NonTransactional, EditorOnly, NoDestructor, AutoWeak, ContainsInstancedReference, AssetRegistrySearchable, SimpleDisplay, AdvancedDisplay, Protected, BlueprintCallable, BlueprintAuthorityOnly, TextExportTransient, NonPIEDuplicateTransient, ExposeOnSpawn, PersistentInstance, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic, NativeAccessSpecifierProtected, NativeAccessSpecifierPrivate)
	bool bApplyCriticalHitModsToAoEDamage;	// 0x0048(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, RepSkip, RepNotify, Interp, NonTransactional, EditorOnly, NoDestructor, AutoWeak, ContainsInstancedReference, AssetRegistrySearchable, SimpleDisplay, AdvancedDisplay, Protected, BlueprintCallable, BlueprintAuthorityOnly, TextExportTransient, NonPIEDuplicateTransient, ExposeOnSpawn, PersistentInstance, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic, NativeAccessSpecifierProtected, NativeAccessSpecifierPrivate)
	PAD(0xF);								// 0x0049(0x0003) MISSED OFFSET
};

// Class Engine.MovementComponent
// 0x0048 (0x01C0 - 0x0178)
class UMovementComponent : public UActorComponent
{
public:
	class USceneComponent* UpdatedComponent;	// 0x0178(0x0008) (BlueprintVisible, ExportObject, BlueprintReadOnly, ZeroConstructor, Transient, InstancedReference, DuplicateTransient, IsPlainOldData)
	PAD(0xC);									// 0x0180(0x0008) (BlueprintVisible, ExportObject, BlueprintReadOnly, ZeroConstructor, Transient, InstancedReference, DuplicateTransient, IsPlainOldData)
	class FVector Velocity;					// 0x018C(0x000C) (Edit, BlueprintVisible, IsPlainOldData)
	PAD(0x28);									// 0x0198(0x0001) (Edit, BlueprintVisible, BlueprintReadOnly)

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class Engine.MovementComponent");

		return ptr;
	}
};

struct FMovementProperties
{
	unsigned char                                      bCanCrouch : 1;                                           // 0x0000(0x0001) (Edit, BlueprintVisible)
	unsigned char                                      bCanJump : 1;                                             // 0x0000(0x0001) (Edit, BlueprintVisible)
	unsigned char                                      bCanWalk : 1;                                             // 0x0000(0x0001) (Edit, BlueprintVisible)
	unsigned char                                      bCanSwim : 1;                                             // 0x0000(0x0001) (Edit, BlueprintVisible)
	unsigned char                                      bCanFly : 1;                                              // 0x0000(0x0001) (Edit, BlueprintVisible)
	unsigned char                                      UnknownData00[0x3];                                       // 0x0001(0x0003) MISSED OFFSET
};

// Class Engine.NavMovementComponent
// 0x0048 (0x0208 - 0x01C0)
class UNavMovementComponent : public UMovementComponent
{
public:
	PAD(0x40);									// 0x01C0(0x0030) (Edit, BlueprintVisible)
	struct FMovementProperties MovementState;	// 0x0200(0x0004)
	PAD(0x4);									// 0x0204(0x0004) MISSED OFFSET
};

// Class Engine.PawnMovementComponent
// 0x0008 (0x0210 - 0x0208)
class UPawnMovementComponent : public UNavMovementComponent
{
public:
	class APawn* PawnOwner;                                                // 0x0208(0x0008) (ZeroConstructor, Transient, DuplicateTransient, IsPlainOldData)

	class FVector K2_GetInputVector()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.PawnMovementComponent.K2_GetInputVector");

		FVector params;

		ProcessEvent(this, fn, &params);

		return params;
	}

	void AddInputVector(const class FVector& WorldVector, bool bForce)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.PawnMovementComponent.AddInputVector");

		struct {
			FVector WorldVector;
			bool bForce;
		} params;

		params = { WorldVector, bForce };

		ProcessEvent(this, fn, &params);
	}
};


// Class Engine.ProjectileMovementComponent
// 0x0098 (0x0258 - 0x01C0)
class UProjectileMovementComponent : public UMovementComponent
{
public:
	PAD(0x1C0);
	float          InitialSpeed;                            // 0x01C0(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float          MaxSpeed;                                // 0x01C4(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	unsigned char  bRotationFollowsVelocity : 1;            // 0x01C8(0x0001) (Edit, BlueprintVisible)
	unsigned char  bShouldBounce : 1;                       // 0x01C8(0x0001) (Edit, BlueprintVisible)
	unsigned char  bInitialVelocityInLocalSpace : 1;        // 0x01C8(0x0001) (Edit, BlueprintVisible)
	unsigned char  bForceSubStepping : 1;                   // 0x01C8(0x0001) (Edit, BlueprintVisible)
	unsigned char  bIsHomingProjectile : 1;                 // 0x01C8(0x0001) (Edit, BlueprintVisible)
	unsigned char  bBounceAngleAffectsFriction : 1;         // 0x01C8(0x0001) (Edit, BlueprintVisible)
	unsigned char  bIsSliding : 1;                          // 0x01C8(0x0001) (Edit, BlueprintVisible, BlueprintReadOnly, DisableEditOnTemplate, EditConst)
	unsigned char  UnknownData00[0x3];                      // 0x01C9(0x0003) MISSED OFFSET
	float          PreviousHitTime;                         // 0x01CC(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, DisableEditOnTemplate, EditConst, IsPlainOldData)
	class FVector PreviousHitNormal;                       // 0x01D0(0x000C) (Edit, BlueprintVisible, BlueprintReadOnly, DisableEditOnTemplate, EditConst, IsPlainOldData)
	float          ProjectileGravityScale;                  // 0x01DC(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float          Buoyancy;                                // 0x01E0(0x0004) (ZeroConstructor, IsPlainOldData)
	float          Bounciness;                              // 0x01E4(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float          Friction;                                // 0x01E8(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float          BounceVelocityStopSimulatingThreshold;   // 0x01EC(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	PAD(0x68);												// 0x01F0(0x0010) (ZeroConstructor, InstancedReference, BlueprintAssignable)
};

// Class GbxWeapon.GbxProjectileMovementComponent
// 0x0070 (0x02C8 - 0x0258)
class UGbxProjectileMovementComponent : public UProjectileMovementComponent
{
public:
	PAD(0x70);
};

// Class GbxGameSystemCore.UseComponent
// 0x0428 (0x05A0 - 0x0178)
class UUseComponent : public UActorComponent
{
public:
	PAD(0x50);								// 0x0178(0x0010) (ZeroConstructor, InstancedReference, BlueprintAssignable)
	float ViewDistance;                     // 0x01C8(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	float InteractDistance;                 // 0x01CC(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	float LeaveInteractionDistance;         // 0x01D0(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	float VehicleInteractDistance;          // 0x01D4(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	float VehicleLeaveInteractionDistance;	// 0x01D8(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	PAD(0x3C4);								// 0x01DC(0x0004) MISSED OFFSET

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class GbxGameSystemCore.UseComponent");

		return ptr;
	}
};

// Class GbxGameSystemCore.ExplosionComponent
// 0x0060 (0x01D8 - 0x0178)
class UExplosionComponent : public UActorComponent
{
public:
	struct FGbxAttributeFloat ExplosionRadius;	// 0x0178(0x000C) (Edit, BlueprintVisible)
	PAD(0x4);									// 0x0184(0x0004) MISSED OFFSET
};

// Class Engine.CharacterMovementComponent
// 0x0680 (0x0890 - 0x0210)
class UCharacterMovementComponent : public UPawnMovementComponent
{
public:
	PAD(0x64);							// 0x0210(0x0010) MISSED OFFSET
	struct FGbxAttributeFloat                          MaxWalkSpeed;                                             // 0x0274(0x000C) (Edit, BlueprintVisible, Net)
	struct FGbxAttributeFloat                          MaxWalkSpeedCrouched;                                     // 0x0280(0x000C) (Edit, BlueprintVisible, Net)
	struct FGbxAttributeFloat                          MaxSwimSpeed;                                             // 0x028C(0x000C) (Edit, BlueprintVisible, Net)
	struct FGbxAttributeFloat                          MaxFlySpeed;                                              // 0x0298(0x000C) (Edit, BlueprintVisible, Net)
	struct FGbxAttributeFloat                          MaxCustomMovementSpeed;                                   // 0x02A4(0x000C) (Edit, BlueprintVisible, Net)
	float                                              MaxAcceleration;                                          // 0x02B0(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	PAD(0x9C);                                       // 0x02B4(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	class FVector                                     Acceleration;                                             // 0x0350(0x000C) (IsPlainOldData)
	PAD(0x4E4);                                       // 0x035C(0x000C) (IsPlainOldData)
	float                                              WalkableLedgeAngle;                                       // 0x0840(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	float                                              WalkableLedgeZ;                                           // 0x0844(0x0004) (Edit, ZeroConstructor, EditConst, IsPlainOldData)
	PAD(0x48);                                      // 0x0848(0x0047) MISSED OFFSET

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class Engine.CharacterMovementComponent");

		return ptr;
	}

	void SetMovementMode(TEnumAsByte<EMovementMode> NewMovementMode, unsigned char NewCustomMode)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.CharacterMovementComponent.SetMovementMode");

		struct {
			TEnumAsByte<EMovementMode> NewMovementMode;
			unsigned char NewCustomMode;
		} params;

		params = { NewMovementMode, NewCustomMode };

		ProcessEvent(this, fn, &params);
	}
};

// ScriptStruct OakGame.CharacterJumpGoalSetting
// 0x000C
struct FCharacterJumpGoalSetting
{
	float InitialZVelocity;                                         // 0x0000(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float GoalHeight;                                               // 0x0004(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	unsigned char bUseInitialZVelocity : 1;                                 // 0x0008(0x0001) (Edit, BlueprintVisible)
	unsigned char bUseGoalHeight : 1;                                       // 0x0008(0x0001) (Edit, BlueprintVisible)
	PAD(0x3);                                       // 0x0009(0x0003) MISSED OFFSET
};

// Class GbxGameSystemCore.GbxCharacterMovementComponent
// 0x1DF0 (0x2680 - 0x0890)
class UGbxCharacterMovementComponent : public UCharacterMovementComponent
{
public:
	PAD(0x8AC);                                     // 0x0890(0x0748) MISSED OFFSET
	struct FGbxAttributeFloat                          MaxLadderAscendSpeed;                                     // 0x113C(0x000C) (Edit, BlueprintVisible, Net)
	struct FGbxAttributeFloat                          MaxLadderDescendSpeed;                                    // 0x1148(0x000C) (Edit, BlueprintVisible, Net)
	PAD(0xFB0);                                    // 0x1154(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              AirFriction;                                              // 0x2104(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	PAD(0x578);                           // 0x2108(0x0004) (ZeroConstructor, Transient, IsPlainOldData)
};

// Class GbxNav.GbxNavCharacterMovementComponent
// 0x0210 (0x2890 - 0x2680)
class UGbxNavCharacterMovementComponent : public UGbxCharacterMovementComponent
{
public:
	PAD(0x210);
};


// Class OakGame.OakCharacterMovementComponent
// 0x0700 (0x2F90 - 0x2890)
class UOakCharacterMovementComponent : public UGbxNavCharacterMovementComponent
{
public:
	float                                              MaxInheritedSpeedFromSimulatingPhysicsMovementBase;       // 0x2890(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	bool                                               bUseJumpGoals;                                            // 0x2894(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData00[0x3];                                       // 0x2895(0x0003) MISSED OFFSET
	float                                              SprintingJumpMaxSpeedPct;                                 // 0x2898(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              SprintingJumpHorizontalSpeedScale;                        // 0x289C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	struct FCharacterJumpGoalSetting                   JumpGoal_Default;                                         // 0x28A0(0x000C) (Edit, BlueprintVisible)
	struct FCharacterJumpGoalSetting                   JumpGoal_Sprinting;                                       // 0x28AC(0x000C) (Edit, BlueprintVisible)
	float                                              JumpHorizontalSpeedScaleWhenNoAcceleration;               // 0x28B8(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              FallDelayTime;                                            // 0x28BC(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	float                                              FallDelayGravityScale;                                    // 0x28C0(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	float                                              JumpQueueTime;                                            // 0x28C4(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData01[0x4];                                       // 0x28C8(0x0004) MISSED OFFSET
	struct FGbxAttributeFloat                          MaxSprintSpeed;                                           // 0x28CC(0x000C) (Edit, BlueprintVisible, Net)
	struct FGbxAttributeFloat                          MaxInjuredSprintSpeed;                                    // 0x28D8(0x000C) (Edit, BlueprintVisible, Net)
	struct FGbxAttributeFloat                          MaxWalkSpeedInjured;                                      // 0x28E4(0x000C) (Edit, BlueprintVisible, Net)
	struct FGbxAttributeFloat                          MaxWalkSpeedInjuredBeingRevived;                          // 0x28F0(0x000C) (Edit, BlueprintVisible, Net)
	float                                              MaxSprintAngle;                                           // 0x28FC(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              SprintAnalogInputThreshold;                               // 0x2900(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	PAD(0x68C);                      // 0x2904(0x0001) (Edit, BlueprintVisible)

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class OakGame.OakCharacterMovementComponent");

		return ptr;
	}
};

// Class GbxWeapon.RecoilControlComponent
// 0x0060 (0x01D8 - 0x0178)
class URecoilControlComponent : public UActorComponent
{
public:
	PAD(0x24);									// 0x0178(0x0010) (ZeroConstructor, InstancedReference, BlueprintAssignable)
	class FRotator CurrentRotation;			// 0x019C(0x000C) (Transient, IsPlainOldData)
	class FRotator TargetRotation;				// 0x01A8(0x000C) (Transient, IsPlainOldData)
	class FRotator InputPassthroughThreshold;	// 0x01B4(0x000C) (Edit, BlueprintVisible, BlueprintReadOnly, IsPlainOldData)
	PAD(0x18);									// 0x01C0(0x0018) MISSED OFFSET

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class GbxWeapon.RecoilControlComponent");

		return ptr;
	}
};

// Class GbxWeapon.WeaponReloadComponent
// 0x0060 (0x01D8 - 0x0178)
class UWeaponReloadComponent : public UActorComponent
{
public:
	PAD(0x48);								// 0x0178(0x0010) MISSED OFFSET
	struct FGbxAttributeFloat ReloadTime;	// 0x01C0(0x000C) (Edit, BlueprintVisible, BlueprintReadOnly, Net)
	PAD(0xC);								// 0x01CC(0x0004) MISSED OFFSET

	static UClass* StaticClass()
	{
		static UClass* ptr = UObject::FindObject<UClass>("Class GbxWeapon.WeaponReloadComponent");

		return ptr;
	}
};

// Class GbxWeapon.WeaponRecoilComponent
// 0x0020 (0x0198 - 0x0178)
class UWeaponRecoilComponent : public UActorComponent
{
public:
	PAD(0x10);                                      // 0x0178(0x0010) MISSED OFFSET
	class URecoilControlComponent* CachedControlComponent;                                   // 0x0188(0x0008) (ExportObject, ZeroConstructor, Transient, InstancedReference, IsPlainOldData)
	class AWeapon* WeaponPrivate;                                            // 0x0190(0x0008) (ZeroConstructor, Transient, IsPlainOldData)

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class GbxWeapon.WeaponRecoilComponent");

		return ptr;
	}
};

// Class OakGame.SwayPatternData
// 0x0018 (0x0048 - 0x0030)
class USwayPatternData
{
public:
	PAD(0x30);
	TArray<class FVector2D>                           Samples;                                                  // 0x0030(0x0010) (Edit, ZeroConstructor, DisableEditOnInstance, EditConst)
	int                                                NumberOfOctaves;                                          // 0x0040(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData)
	int                                                NumberOfSamplesToGenerate;                                // 0x0044(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData)
};

// Class OakGame.WeaponSwayComponent
// 0x00F0 (0x0268 - 0x0178)
class UWeaponSwayComponent : public UActorComponent
{
public:
	PAD(0x10);                                        // 0x0178(0x0010) MISSED OFFSET
	class USwayPatternData* Pattern;                                                  // 0x0188(0x0008) (Edit, ZeroConstructor, IsPlainOldData)
	struct FGbxAttributeFloat                          Scale;                                                    // 0x0190(0x000C) (Edit, Net)
	struct FGbxAttributeFloat                          AccuracyScale;                                            // 0x019C(0x000C) (Edit, Net)
	float                                              Speed;                                                    // 0x01A8(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	float                                              Lacunarity;                                               // 0x01AC(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	float                                              Persistence;                                              // 0x01B0(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	float                                              DiscWidth;                                                // 0x01B4(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	float                                              DiscHeight;                                               // 0x01B8(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	struct FGbxAttributeFloat                          ZoomScale;                                                // 0x01BC(0x000C) (Edit, Net)
	struct FGbxAttributeFloat                          ZoomAccuracyScale;                                        // 0x01C8(0x000C) (Edit, Net)
	float                                              ZoomSmoothingSpeed;                                       // 0x01D4(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	float                                              ZoomSmoothInTime_OnZoom;                                  // 0x01D8(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	float                                              ZoomSmoothInTime_OnFire;                                  // 0x01DC(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	bool                                               bApplyAccuracyScaleOnlyWhenFiring;                        // 0x01E0(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	PAD(0x3);                                       // 0x01E1(0x0003) MISSED OFFSET
	float                                              MaxAccuracyPercent;                                       // 0x01E4(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	class FRotator                                    CurrentRotation;                                          // 0x01E8(0x000C) (Transient, IsPlainOldData)
	float                                              CurrentScale;                                             // 0x01F4(0x0004) (ZeroConstructor, Transient, IsPlainOldData)
	PAD(0x68);                                      // 0x01F8(0x0068) MISSED OFFSET
	class AWeapon* WeaponPrivate;                                            // 0x0260(0x0008) (ZeroConstructor, Transient, IsPlainOldData)

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class OakGame.WeaponSwayComponent");

		return ptr;
	}
};

// Class GbxWeapon.WeaponHeatComponent
// 0x00C0 (0x0238 - 0x0178)
class UWeaponHeatComponent : public UActorComponent
{
public:
	PAD(0x10);											// 0x0178(0x0010) MISSED OFFSET
	struct FGbxAttributeFloat HeatCoolDownRate;         // 0x0188(0x000C) (Edit, Net)
	struct FGbxAttributeFloat HeatCoolDownDelay;        // 0x0194(0x000C) (Edit, Net)
	struct FGbxAttributeFloat UseHeatImpulse;           // 0x01A0(0x000C) (Edit, Net)
	struct FGbxAttributeFloat ActiveHeatRate;           // 0x01AC(0x000C) (Edit, Net)
	PAD(0x4);											// 0x01B8(0x0001) (Edit, Net, ZeroConstructor, IsPlainOldData)
	struct FGbxAttributeFloat OverheatTime;				// 0x01BC(0x000C) (Edit, Net)
	struct FGbxAttributeFloat OverheatCoolDownDelay;	// 0x01C8(0x000C) (Edit, Net)
	PAD(0x4);											// 0x01D4(0x0004) MISSED OFFSET
	float Heat;											// 0x0218(0x0004) (BlueprintVisible, BlueprintReadOnly, ZeroConstructor, Transient, IsPlainOldData)
	PAD(0x1C);											// 0x021C(0x000C) MISSED OFFSET

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class GbxWeapon.WeaponHeatComponent");

		return ptr;
	}
};

// Class GbxWeapon.Projectile
// 0x0228 (0x0680 - 0x0458)
class AProjectile : public AActor
{
public:
	PAD(0x10);													// 0x0458(0x0010) MISSED OFFSET
	struct FDamageInfo DamageInfo;								// 0x0470(0x0058) (Edit, BlueprintVisible, Net)
	PAD(0x8);													// 0x04C8(0x0008) (Edit, ZeroConstructor, IsPlainOldData)
	float DefaultImpactEffectDelay;								// 0x04D0(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	PAD(0x14);													// 0x04D4(0x0001) (BlueprintVisible, BlueprintReadOnly, Net, ZeroConstructor, Transient, IsPlainOldData)
	float SpawnModifierValue;									// 0x04E8(0x0004) (BlueprintVisible, BlueprintReadOnly, Net, ZeroConstructor, Transient, IsPlainOldData)
	PAD(0x4);													// 0x04EC(0x0004) MISSED OFFSET
	class UGbxProjectileMovementComponent* MovementComponent;	// 0x04F0(0x0008) (ExportObject, ZeroConstructor, Transient, InstancedReference, IsPlainOldData)
	class UExplosionComponent* ExplosionComponent;				// 0x04F8(0x0008) (ExportObject, ZeroConstructor, InstancedReference, IsPlainOldData)
	PAD(0x128);													// 0x0500(0x0001) MISSED OFFSET
	float DamageAreaRadius;										// 0x0628(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	class FVector DamageAreaBoxExtent;							// 0x062C(0x000C) (Edit, BlueprintVisible, IsPlainOldData)
	PAD(0x38);													// 0x0638(0x0020) (Edit, BlueprintVisible)
	class AWeapon* WeaponFiredFrom;								// 0x0670(0x0008) (ZeroConstructor, Transient, IsPlainOldData)
	float SpawnSpeedScale;										// 0x0678(0x0004) (Net, ZeroConstructor, Transient, IsPlainOldData)
	PAD(0x4);													// 0x067C(0x0004) MISSED OFFSET

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class GbxWeapon.Projectile");

		return ptr;
	}
};

// Class OakGame.OakProjectile
// 0x0060 (0x06E0 - 0x0680)
class AOakProjectile : public AProjectile
{
public:
	PAD(0x10);											// 0x0680(0x0010) MISSED OFFSET
	bool bIgnoreCollisionWithInstigatorAndChildrens;	// 0x0690(0x0001) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, DisableEditOnInstance, IsPlainOldData)
	bool bIsGrenade;                                    // 0x0691(0x0001) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData)
	bool bSuppressSpawnModifierEffect;                  // 0x0692(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	PAD(0x36);											// 0x0693(0x0001) MISSED OFFSET
	bool bRegisterAsTargetable;                         // 0x06C9(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	bool bShouldDisplayNameplate;                       // 0x06CA(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	PAD(0x5);											// 0x06CB(0x0005) MISSED OFFSET
	class UDamageComponent* DamageComponent;			// 0x06D0(0x0008) (ExportObject, ZeroConstructor, InstancedReference, IsPlainOldData)
	PAD(0x8);											// 0x06D8(0x0008) (ExportObject, ZeroConstructor, InstancedReference, IsPlainOldData)

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class OakGame.OakProjectile");

		return ptr;
	}
};

// Class Engine.SphereComponent
// 0x0010 (0x0720 - 0x0710)
class USphereComponent
{
public:
	PAD(0x0710);
	float SphereRadius;	// 0x0710(0x0004) (Edit, BlueprintVisible, ExportObject, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	PAD(0xC);			// 0x0714(0x0001) (Edit, BlueprintVisible, ExportObject, ZeroConstructor, IsPlainOldData)
};

// BlueprintGeneratedClass Proj_Weapon_BaseProjectile.Proj_Weapon_BaseProjectile_C
// 0x00AA (0x078A - 0x06E0)
class AProj_Weapon_BaseProjectile_C : public AOakProjectile
{
public:
	PAD(0x28);									// 0x06E0(0x0008) (Transient, DuplicateTransient)
	class USphereComponent* SphereCollision;	// 0x06F0(0x0008) (BlueprintVisible, ZeroConstructor, InstancedReference, IsPlainOldData)
	PAD(0x8);									// 0x06F8(0x0008) (BlueprintVisible, ZeroConstructor, InstancedReference, IsPlainOldData)
	float FuseTime;                             // 0x0700(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float Damage;                               // 0x0704(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	struct FGbxAttributeFloat DamageRadius;		// 0x0708(0x000C) (Edit, BlueprintVisible, Net)
	PAD(0x76);									// 0x0714(0x0008) (Edit, BlueprintVisible, Net, ZeroConstructor, DisableEditOnInstance, IsPlainOldData)

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("BlueprintGeneratedClass Proj_Weapon_BaseProjectile.Proj_Weapon_BaseProjectile_C");

		return ptr;
	}
};

// Class GbxVehicle.Vehicle
// 0x00B8 (0x0578 - 0x04C0)
class AVehicle : public APawn
{
public:
	PAD(0x10);                                      // 0x04C0(0x0010) MISSED OFFSET
	class USkeletalMeshComponent* Mesh;				// 0x04D0(0x0008) (Edit, BlueprintVisible, ExportObject, BlueprintReadOnly, ZeroConstructor, DisableEditOnInstance, EditConst, InstancedReference, IsPlainOldData)
	PAD(0x18);                                      // 0x04D8(0x0018) MISSED OFFSET
	bool bUnpushable;								// 0x0500(0x0001) (BlueprintVisible, ZeroConstructor, IsPlainOldData)
	PAD(0x1F);										// 0x0501(0x0007) MISSED OFFSET
	class UDamageComponent* CachedDamageComponent;	// 0x0520(0x0008) (ExportObject, ZeroConstructor, Transient, InstancedReference, IsPlainOldData)
	PAD(0x28);										// 0x0528(0x0008) (ExportObject, ZeroConstructor, Transient, InstancedReference, IsPlainOldData)
	float CharacterCollidingResistance;				// 0x0550(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	PAD(0x24);                                      // 0x0554(0x0024) MISSED OFFSET

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class GbxVehicle.Vehicle");

		return ptr;
	}

	class UTeam* GetTeam()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function GbxVehicle.Vehicle.GetTeam");

		UTeam* ReturnValue;

		ProcessEvent(this, fn, &ReturnValue);

		return ReturnValue;
	}
};

// Class Engine.Character
// 0x03F0 (0x08B0 - 0x04C0)
class ACharacter : public APawn
{
public:
	class USkeletalMeshComponent* Mesh;	// 0x04C0(0x0008) (Edit, BlueprintVisible, ExportObject, BlueprintReadOnly, ZeroConstructor, EditConst, InstancedReference, IsPlainOldData)
	PAD(0x3E8);							// 0x04C8(0x03E8) (Edit, BlueprintVisible, ExportObject, BlueprintReadOnly, ZeroConstructor, EditConst, InstancedReference, IsPlainOldData)

	void ClientCheatFly()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.Character.ClientCheatFly");

		void* params;

		ProcessEvent(this, fn, &params);
	}
};

// Class GbxGameSystemCore.TeamComponent
// 0x0040 (0x01B8 - 0x0178)
class UTeamComponent : public UActorComponent
{
public:
	PAD(0x40);

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindObject<UClass>("Class GbxGameSystemCore.TeamComponent");
		return ptr;
	}

	bool IsHostile(class AActor* actor)
	{
		static auto fn = UObject::FindObject<UObject>("Function GbxGameSystemCore.TeamComponent.IsHostile");

		struct
		{
			AActor* Actor;
			bool ReturnValue;
		} params;

		params.Actor = actor;

		ProcessEvent(this, fn, &params);

		return params.ReturnValue;
	}
};

// Class GbxGameSystemCore.DamageComponent
// 0x06B0 (0x0878 - 0x01C8)
class UDamageComponent
{
public:
	PAD(0x5E8);													// 0x0000(0x05E8) MISSED OFFSET
	struct FGbxAttributeFloat SelfReflectionChance;             // 0x07B0(0x000C) (Edit)
	struct FGbxAttributeFloat SelfReflectionDamageTakenScale;   // 0x07BC(0x000C) (Edit)
	struct FGbxAttributeFloat SelfReflectionDamageScale;        // 0x07C8(0x000C) (Edit)
	struct FGbxAttributeInteger	SelfReflectionTowardsAttacker;	// 0x07D4(0x000C) (Edit)
	PAD(0x98);													// 0x07E0(0x0878) MISSED OFFSET

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class GbxGameSystemCore.DamageComponent");

		return ptr;
	}

	float GetTotalMaxHealth()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function GbxGameSystemCore.DamageComponent.GetTotalMaxHealth");

		float totalMaxHealth;
		ProcessEvent(this, fn, &totalMaxHealth);

		return totalMaxHealth;
	};

	float GetMaxShield()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function GbxGameSystemCore.DamageComponent.GetMaxShield");

		float maxShield;
		ProcessEvent(this, fn, &maxShield);

		return maxShield;
	};

	float GetMaxHealth()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function GbxGameSystemCore.DamageComponent.GetMaxHealth");

		float maxHealth;
		ProcessEvent(this, fn, &maxHealth);

		return maxHealth;
	};

	float GetCurrentShield()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function GbxGameSystemCore.DamageComponent.GetCurrentShield");

		float currentShield;
		ProcessEvent(this, fn, &currentShield);

		return currentShield;
	};

	float GetCurrentHealthPercent()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function GbxGameSystemCore.DamageComponent.GetCurrentHealthPercent");

		float currentHealthPercent;
		ProcessEvent(this, fn, &currentHealthPercent);

		return currentHealthPercent;
	};

	float GetCurrentHealth()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function GbxGameSystemCore.DamageComponent.GetCurrentHealth");

		float currentHealth;
		ProcessEvent(this, fn, &currentHealth);

		return currentHealth;
	};

	void SetGodMode(bool bInGodMode)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function GbxGameSystemCore.DamageComponent.SetGodMode");

		ProcessEvent(this, fn, &bInGodMode);
	}

	void SetDemiGodMode(bool bInDemiGodMode)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function GbxGameSystemCore.DamageComponent.SetDemiGodMode");

		ProcessEvent(this, fn, &bInDemiGodMode);
	}

	void SetCurrentShield(float NewCurrentShield)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function GbxGameSystemCore.DamageComponent.SetCurrentShield");

		ProcessEvent(this, fn, &NewCurrentShield);
	}

	void SetCurrentHealth(float NewCurrentHealth)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function GbxGameSystemCore.DamageComponent.SetCurrentHealth");

		ProcessEvent(this, fn, &NewCurrentHealth);
	}

	bool IsInGodMode()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function GbxGameSystemCore.DamageComponent.IsInGodMode");

		bool params;
		ProcessEvent(this, fn, &params);

		return params;
	};

	bool IsInDemiGodMode()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function GbxGameSystemCore.DamageComponent.IsInDemiGodMode");

		bool params;
		ProcessEvent(this, fn, &params);

		return params;
	};
};

// Class GbxGameSystemCore.Team
// 0x0080 (0x00B0 - 0x0030)
class UTeam 
{
public:
	PAD(0x30);
	class FString				DisplayName;					// 0x0030(0x0010) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor)
	TEnumAsByte<ETeamAttitude>	SelfAttitude;					// 0x0040(0x0001) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	TEnumAsByte<ETeamAttitude>	DefaultAttitude;				// 0x0041(0x0001) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	TEnumAsByte<ETeamAttitude>	DefaultAttitudeFromTeams;		// 0x0042(0x0001) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	bool						bUseDefaultAttitudeFromTeams;	// 0x0043(0x0001) (ZeroConstructor, IsPlainOldData)
	PAD(0x6C);													// 0x0044(0x0004) MISSED OFFSET

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindObject<UClass>("Class GbxGameSystemCore.Team");
		return ptr;
	}

	TEnumAsByte<ETeamAttitude> GetAttitudeToward(class UTeam* Team)
	{
		static UObject* fn = UObject::FindObject<UObject>("Function GbxGameSystemCore.Team.GetAttitudeToward");

		struct {
			UTeam* Team;
			TEnumAsByte<ETeamAttitude> ReturnValue;
		} params;

		params.Team = Team;

		ProcessEvent(this, fn, &params);

		return params.ReturnValue;
	}
};

// Class GbxGameSystemCore.GbxCharacter
// 0x04D0 (0x0D80 - 0x08B0)
class AGbxCharacter : public ACharacter
{
public:
	PAD(0x60);									// 0x08B0(0x0050) MISSED OFFSET
	class UDamageComponent* DamageComponent;	// 0x0910(0x0008) 
	PAD(0x468);									// 0x0918(0x0008) 

	class UTeam* GetTeam()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function GbxGameSystemCore.GbxCharacter.GetTeam");

		UTeam* team;
		ProcessEvent(this, fn, &team);

		return team;
	};

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class GbxGameSystemCore.GbxCharacter");

		return ptr;
	}
};

// Class GbxWeapon.WeaponUseComponent
// 0x00A0 (0x0218 - 0x0178)
class UWeaponUseComponent : public UActorComponent
{
public:
	PAD(0x20);											// 0x0178(0x0008) MISSED OFFSET
	class UTexture2D* Icon;								// 0x0198(0x0008) (Edit, ZeroConstructor, IsPlainOldData)
	class FString ModeName;								// 0x01A0(0x0010) (Edit, ZeroConstructor)
	PAD(0x68);											// 0x01B0(0x0010) (Edit, ZeroConstructor)
};

// Class GbxWeapon.WeaponChargeComponent
// 0x0168 (0x0300 - 0x0198)
class UWeaponChargeComponent
{
public:
	PAD(0x198);
	struct FGbxAttributeFloat ChargeTime;			// 0x0198(0x000C) (Edit, BlueprintVisible, Net)
	struct FGbxAttributeFloat DischargeTime;		// 0x01A4(0x000C) (Edit, BlueprintVisible, Net)
	struct FGbxAttributeFloat CancelChargeDelay;	// 0x01B0(0x000C) (Edit, BlueprintVisible, Net)
	float RequiredUseChargePct;						// 0x01BC(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float CommitUseChargePct;						// 0x01C0(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	struct FGbxAttributeFloat OverchargeHoldTime;	// 0x01C4(0x000C) (Edit, BlueprintVisible, Net)
	PAD(0x1);										// 0x01D0(0x0001) (Edit, BlueprintVisible)
	EWeaponChargeState ChargeState;					// 0x02B8(0x0001) (Net, ZeroConstructor, Transient, IsPlainOldData)
	PAD(0x47);                                      // 0x02B9(0x0047) MISSED OFFSET

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class GbxWeapon.WeaponChargeComponent");

		return ptr;
	}
};

// Class GbxWeapon.WeaponSingleFeedReloadComponent
// 0x0038 (0x0210 - 0x01D8)
class UWeaponSingleFeedReloadComponent : public UWeaponReloadComponent
{
public:
	bool bDisableInterruptedToUse;		// 0x01D8(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
	PAD(0x3);							// 0x01D9(0x0003) MISSED OFFSET
	int SingleFeedIncrement;			// 0x01DC(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, Net, ZeroConstructor, IsPlainOldData)
	float SingleFeedCompletePercent;	// 0x01E0(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	PAD(0x2C);							// 0x01E4(0x0025) MISSED OFFSET

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindObject<UClass>("Class GbxWeapon.WeaponSingleFeedReloadComponent");
		return ptr;
	}
};

// Class GbxWeapon.WeaponClipReloadComponent
// 0x0030 (0x0208 - 0x01D8)
class UWeaponClipReloadComponent : public UWeaponReloadComponent
{
public:
	float ReloadCompletePercent;                                    // 0x01D8(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	struct FGbxAttributeFloat TapedReloadTime;                                          // 0x01DC(0x000C) (Edit, BlueprintVisible, BlueprintReadOnly)
	float TapedReloadCompletePercent;                               // 0x01E8(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	int TapedClipCount;                                           // 0x01EC(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	PAD(0x18);                                      // 0x01F0(0x0011) MISSED OFFSET

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindObject<UClass>("Class GbxWeapon.WeaponClipReloadComponent");
		return ptr;
	}
};

// Class GbxWeapon.WeaponFireComponent
// 0x06E8 (0x0900 - 0x0218)
class UWeaponFireComponent : public UWeaponUseComponent
{
public:
	struct FGbxAttributeFloat FireRate;					// 0x0218(0x000C) (Edit, BlueprintVisible, Net)
	struct FGbxAttributeInteger AutomaticBurstCount;	// 0x0224(0x000C) (Edit, BlueprintVisible, Net)
	struct FGbxAttributeFloat BurstFireDelay;			// 0x0230(0x000C) (Edit, BlueprintVisible, Net)
	bool bAutoBurst;									// 0x023C(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	PAD(0x1C);					 						// 0x023D(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	TEnumAsByte<ECollisionChannel> TraceChannel;		// 0x0259(0x0001)
	PAD(0x86);					 						// 0x025A(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	struct FGbxAttributeInteger ShotAmmoCost;			// 0x02E0(0x000C) (Edit, BlueprintVisible, Net)
	PAD(0x4);											// 0x02EC(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	struct FGbxAttributeFloat Spread;					// 0x02F0(0x000C) (Edit, BlueprintVisible, Net)
	struct FGbxAttributeFloat AccuracyImpulse;			// 0x02FC(0x000C) (Edit, BlueprintVisible, Net)
	PAD(0x10);											// 0x0308(0x000C) (Edit, BlueprintVisible, Net)
	struct FGbxAttributeFloat Damage;					// 0x0318(0x000C) (Edit, BlueprintVisible, Net)
	PAD(0x44);											// 0x0324(0x000C) (Edit, BlueprintVisible, Net)
	struct FGbxAttributeFloat DamageRadius;				// 0x0368(0x000C)
	PAD(0x58C);											// 0x0374(0x000C) (Edit, BlueprintVisible, Net)
};

// Class GbxWeapon.WeaponFireProjectileComponent
// 0x0058 (0x0958 - 0x0900)
class UWeaponFireProjectileComponent : public UWeaponFireComponent
{
public:
	struct FGbxAttributeInteger ProjectilesPerShot;	// 0x0900(0x000C) (Edit, BlueprintVisible, Net)
	struct FGbxAttributeFloat ProjectileSpeedScale;	// 0x090C(0x000C) (Edit, BlueprintVisible, Net)
	PAD(0x30);										// 0x0918(0x0030) (Edit, ZeroConstructor, IsPlainOldData)
	float Range;									// 0x0948(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	PAD(0xC);										// 0x094C(0x000C) MISSED OFFSET

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class GbxWeapon.WeaponFireProjectileComponent");

		return ptr;
	}
};

// Class GbxWeapon.WeaponTypeAsset
// 0x0030 (0x0060 - 0x0030)
class UWeaponTypeAsset
{
public:
	PAD(0x30);
	struct FText DisplayName;				// 0x0030(0x0028) (Edit, DisableEditOnInstance)
	class FString ScaleformDisplayFrameID;	// 0x0048(0x0010) (Edit, ZeroConstructor, DisableEditOnInstance)
	PAD(0x8);
};

// Class GbxWeapon.Weapon
// 0x0480 (0x08D8 - 0x0458)
class AWeapon : public AActor
{
public:
	PAD(0x18);											// 0x0458(0x0018) MISSED OFFSET
	class UWeaponFireComponent* CurrentFireComponent;	// 0x0470(0x0008) (ExportObject, ZeroConstructor, Transient, InstancedReference, IsPlainOldData)
	class UWeaponTypeAsset* WeaponDisplayData;			// 0x0478(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	PAD(0x8);											// 0x0480(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	struct FGbxAttributeFloat SwitchModeTimeScale;		// 0x0488(0x000C) (Edit, BlueprintVisible, BlueprintReadOnly, Net)
	PAD(0x444);											// 0x0494(0x0008) (ExportObject, ZeroConstructor, Transient, InstancedReference, IsPlainOldData)
};

// Class OakGame.OakCharacter
// 0x1520 (0x22A0 - 0x0D80)
class AOakCharacter : public AGbxCharacter
{
public:
	PAD(0x140);                                     // 0x0D80(0x00D8) MISSED OFFSET
	EHealthState HealthState;						// 0x0EC0(0x0001) (Edit, BlueprintVisible, BlueprintReadOnly, Net, ZeroConstructor, DisableEditOnTemplate, Transient, EditConst, IsPlainOldData)
	EDeathType DeathType;							// 0x0EC1(0x0001) (Edit, BlueprintVisible, BlueprintReadOnly, Net, ZeroConstructor, DisableEditOnTemplate, Transient, EditConst, IsPlainOldData)
	PAD(0x24E);                                     // 0x0EC2(0x00D8) MISSED OFFSET
	class FName PlayerGivenNicknameKey;			// 0x1110(0x0008) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData)
	PAD(0x2D0);                                     // 0x1118(0x00D8) MISSED OFFSET
	TArray<void*> ActiveWeaponSlots;				// 0x13E8(0x0010) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor)
	PAD(0xEA8);                                     // 0x13F8(0x00D8) MISSED OFFSET

	// Function OakGame.OakCharacter.GetWeapon
	// (Final, Native, Public, BlueprintCallable, BlueprintPure, Const)
	// Parameters:
	// class UWeaponSlotData*         SlotToGet                      (Parm, ZeroConstructor, IsPlainOldData)
	// class AWeapon*                 ReturnValue                    (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData)

	class AWeapon* GetWeapon(void* SlotToGet)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function OakGame.OakCharacter.GetWeapon");

		struct {
			void* SlotToGet;
			AWeapon* ReturnValue;
		} params;

		params.SlotToGet = SlotToGet;

		ProcessEvent(this, fn, &params);

		return params.ReturnValue;
	}

	class AWeapon* GetActiveWeapon(unsigned char Slot)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function OakGame.OakCharacter.GetActiveWeapon");

		struct {
			unsigned char Slot;
			AWeapon* ReturnValue;
		} params;

		params.Slot = Slot;

		ProcessEvent(this, fn, &params);

		return params.ReturnValue;
	}

	void SetDead(EDeathType InDeathType)
	{
		static auto fn = UObject::FindObject<UObject>("Function OakGame.OakCharacter.SetDead");

		ProcessEvent(this, fn, &InDeathType);
	}
};

// Class OakGame.OakCharacter_Default
// 0x0080 (0x2320 - 0x22A0)
class AOakCharacter_Default : public AOakCharacter
{
public:
	class UStanceComponent* Stance;	// 0x22A0(0x0008) (BlueprintVisible, ExportObject, BlueprintReadOnly, ZeroConstructor, InstancedReference, IsPlainOldData)
	PAD(0x78);						// 0x22A8(0x0008) (BlueprintVisible, ExportObject, BlueprintReadOnly, ZeroConstructor, InstancedReference, IsPlainOldData)

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("Class OakGame.OakCharacter_Default");

		return ptr;
	}
};


// BlueprintGeneratedClass BPChar_AI.BPChar_AI_C
// 0x0028 (0x2348 - 0x2320)
class ABPChar_AI_C : public AOakCharacter_Default
{
public:
	PAD(0x0028);	// 0x2320(0x0008) (Transient, DuplicateTransient)

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("BlueprintGeneratedClass BPChar_AI.BPChar_AI_C");

		return ptr;
	}
};

// BlueprintGeneratedClass BPChar_Enemy.BPChar_Enemy_C
// 0x0039 (0x2381 - 0x2348)
class ABPChar_Enemy_C : public ABPChar_AI_C
{
public:
	PAD(0x039);                                           // 0x2348(0x0008) (Transient, DuplicateTransient)

	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;

		if (ptr == nullptr)
			ptr = UObject::FindObject<UClass>("BlueprintGeneratedClass BPChar_Enemy.BPChar_Enemy_C");

		return ptr;
	}
};

// Class Engine.Controller
// 0x00B0 (0x0508 - 0x0458)
class AController : public AActor
{
public:
	PAD(0x8);									// 0x0458(0x0008) MISSED OFFSET
	class APlayerState* PlayerState;			// 0x0460(0x0008) (BlueprintVisible, BlueprintReadOnly, Net, ZeroConstructor, IsPlainOldData)
	PAD(0x18);									// 0x0468(0x0008) MISSED OFFSET
	class FName StateName;						// 0x0480(0x0008) (ZeroConstructor, IsPlainOldData)
	class APawn* Pawn;							// 0x0488(0x0008) (Net, ZeroConstructor, IsPlainOldData)
	PAD(0x8);									// 0x0490(0x0008) MISSED OFFSET
	class ACharacter* Character;				// 0x0498(0x0008) (ZeroConstructor, IsPlainOldData)
	PAD(0x20);									// 0x04A8(0x0018) MISSED OFFSET
	class FRotator ControlRotation;			// 0x04C0(0x000C) (IsPlainOldData)
	unsigned char bAttachToPawn : 1;			// 0x04CC(0x0001) (Edit, DisableEditOnInstance)
	unsigned char bIsPlayerController : 1;		// 0x04CC(0x0001)
	PAD(0x3B);									// 0x04CD(0x003B) MISSED OFFSET

	void SetIgnoreLookInput(bool bNewLookInput)
	{
		static UObject* fn = UObject::FindObject<UObject>("Function Engine.Controller.SetIgnoreLookInput");

		ProcessEvent(this, fn, &bNewLookInput);
	}
};

// Class Engine.PlayerCameraManager
// Size: 0x2740 (Inherited: 0x220)
class APlayerCameraManager : public AActor
{
public:
	class FRotator GetCameraRotation()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.PlayerCameraManager.GetCameraRotation");

		FRotator CompToWorld;
		ProcessEvent(this, fn, &CompToWorld);
		return CompToWorld;
	};

	class FVector GetCameraLocation()
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.PlayerCameraManager.GetCameraLocation");

		FVector CompToWorld;
		ProcessEvent(this, fn, &CompToWorld);
		return CompToWorld;
	}
};

#ifdef _MSC_VER
#pragma pack(push, 0x8)
#endif

class FKey
{
public:
	FName KeyName;
	unsigned char UnknownData00[0x10];

	inline FKey() : KeyName(), UnknownData00{} {}

	inline FKey(const FName InName) : KeyName(InName), UnknownData00{} {}

	inline FKey(const char* InName) : KeyName(FName(InName)), UnknownData00{} {}
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif

// Class Engine.HUD
// 0x0120 (0x0578 - 0x0458)
class AHUD : public AActor
{
public:
	class APlayerController* PlayerOwner;                                              // 0x0458(0x0008) (BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	unsigned char                                      bLostFocusPaused : 1;                                     // 0x0460(0x0001) (BlueprintVisible, BlueprintReadOnly)
	unsigned char                                      bShowHUD : 1;                                             // 0x0460(0x0001) (Edit, BlueprintVisible)
	unsigned char                                      bShowDebugInfo : 1;                                       // 0x0460(0x0001) (BlueprintVisible)
	unsigned char                                      UnknownData00[0x3];                                       // 0x0461(0x0003) MISSED OFFSET
	int                                                CurrentTargetIndex;                                       // 0x0464(0x0004) (ZeroConstructor, Transient, IsPlainOldData)
	unsigned char                                      bShowHitBoxDebugInfo : 1;                                 // 0x0468(0x0001) (BlueprintVisible)
	unsigned char                                      bShowOverlays : 1;                                        // 0x0468(0x0001) (BlueprintVisible)
	unsigned char                                      bEnableDebugTextShadow : 1;                               // 0x0468(0x0001) (Edit, BlueprintVisible)
	unsigned char                                      UnknownData01[0x7];                                       // 0x0469(0x0007) MISSED OFFSET
	TArray<class AActor*>                              PostRenderedActors;                                       // 0x0470(0x0010) (ZeroConstructor)
	unsigned char                                      UnknownData02[0x8];                                       // 0x0480(0x0008) MISSED OFFSET
	TArray<class FName>                               DebugDisplay;                                             // 0x0488(0x0010) (ZeroConstructor, Config, GlobalConfig)
	TArray<class FName>                               ToggledDebugCategories;                                   // 0x0498(0x0010) (ZeroConstructor, Config, GlobalConfig)
	class UCanvas* Canvas;                                                   // 0x04A8(0x0008) (ZeroConstructor, IsPlainOldData)
	class UCanvas* DebugCanvas;                                              // 0x04B0(0x0008) (ZeroConstructor, IsPlainOldData)
	PAD(0x10);                                            // 0x04B8(0x0010) (ZeroConstructor)
	class UClass* ShowDebugTargetDesiredClass;                              // 0x04C8(0x0008) (ZeroConstructor, IsPlainOldData)
	class AActor* ShowDebugTargetActor;                                     // 0x04D0(0x0008) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData03[0x90];                                      // 0x04D8(0x0090) MISSED OFFSET
	class AActor* OverrideDebugTarget;                                      // 0x0568(0x0008) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData04[0x8];                                       // 0x0570(0x0008) MISSED OFFSET

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindObject<UClass>("Class Engine.HUD");
		return ptr;
	}
};

// Class Engine.PlayerController
// 0x0328 (0x0830 - 0x0508)
class APlayerController : public AController
{
public:
	PAD(0x18);											// 0x0508(0x0008) (ZeroConstructor, IsPlainOldData)
	AHUD* MyHUD;                                  // 0x0520(0x0008)
	class APlayerCameraManager* PlayerCameraManager;	// 0x0528(0x0008) (BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	PAD(0x16C);											// 0x0530(0x016C) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	float InputYawScale;								// 0x069C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, Config, IsPlainOldData)
	float InputPitchScale;								// 0x06A0(0x0004) (Edit, BlueprintVisible, ZeroConstructor, Config, IsPlainOldData)
	float InputRollScale;								// 0x06A4(0x0004) (Edit, BlueprintVisible, ZeroConstructor, Config, IsPlainOldData)
	unsigned char bShowMouseCursor : 1;                 // 0x06A8(0x0001) (Edit, BlueprintVisible)
	unsigned char bEnableClickEvents : 1;               // 0x06A8(0x0001) (Edit, BlueprintVisible)
	unsigned char bEnableTouchEvents : 1;               // 0x06A8(0x0001) (Edit, BlueprintVisible)
	unsigned char bEnableMouseOverEvents : 1;           // 0x06A8(0x0001) (Edit, BlueprintVisible)
	unsigned char bEnableTouchOverEvents : 1;           // 0x06A8(0x0001) (Edit, BlueprintVisible)
	unsigned char bForceFeedbackEnabled : 1;            // 0x06A8(0x0001) (Edit, BlueprintVisible)
	PAD(0x187);											// 0x06A9(0x0004) (Edit, BlueprintVisible, ZeroConstructor, Config, IsPlainOldData)

	bool ProjectWorldLocationToScreen(const class FVector WorldLocation, bool bPlayerViewportRelative, class FVector2D& ScreenLocation)
	{
		return ProjectWorldLocationToScreen(WorldLocation, bPlayerViewportRelative, false, ScreenLocation);
	}

	void ClientIgnoreLookInput(bool bIgnore)
	{
		static UObject* fn = UObject::FindObject<UObject>("Function Engine.PlayerController.ClientIgnoreLookInput");

		ProcessEvent(this, fn, &bIgnore);
	}

	bool ProjectWorldLocationToScreen(const class FVector WorldLocation, bool bPlayerViewportRelative, bool bUseForegroundProjection, class FVector2D& ScreenLocation)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.PlayerController.ProjectWorldLocationToScreen");

		struct {
			FVector WorldLocation;
			FVector2D ScreenLocation;
			bool bPlayerViewportRelative;
			bool bUseForegroundProjection;
			bool ReturnValue;
		} parms;

		parms = { WorldLocation, ScreenLocation, bPlayerViewportRelative, bUseForegroundProjection };
		ProcessEvent(this, fn, &parms);

		ScreenLocation = parms.ScreenLocation;
		return parms.ReturnValue;
	};

	void AddYawInput(float Val)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.PlayerController.AddYawInput");

		Val = Val / InputYawScale;

		ProcessEvent(this, fn, &Val);
	}

	void AddPitchInput(float Val)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.PlayerController.AddPitchInput");

		Val = Val / InputPitchScale;

		ProcessEvent(this, fn, &Val);
	}

	bool IsInputKeyDown(const FKey& Key)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.PlayerController.IsInputKeyDown");

		struct
		{
			FKey Key;
			bool ReturnValue;
		} params;

		params.Key = Key;
		params.ReturnValue = false;

		ProcessEvent(this, fn, &params);

		return params.ReturnValue;
	}

	// Function Engine.PlayerController.GetMousePosition
	// (Final, Native, Public, HasOutParms, BlueprintCallable, BlueprintPure, Const)
	// Parameters:
	// float                          LocationX                      (Parm, OutParm, ZeroConstructor, IsPlainOldData)
	// float                          LocationY                      (Parm, OutParm, ZeroConstructor, IsPlainOldData)
	// bool                           ReturnValue                    (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData)
	bool GetMousePosition(float* LocationX, float* LocationY)
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.PlayerController.GetMousePosition");

		struct {
			float LocationX;
			float LocationY;
			bool ReturnValue;
		} params;

		ProcessEvent(this, fn, &params);

		if (LocationX != nullptr)
			*LocationX = params.LocationX;
		if (LocationY != nullptr)
			*LocationY = params.LocationY;

		return params.ReturnValue;
	}

	void GetViewportSize(int* SizeX, int* SizeY)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.PlayerController.GetViewportSize");

		struct
		{
			int SizeX;
			int SizeY;
		} params;

		ProcessEvent(this, fn, &params);

		if (SizeX != nullptr)
			*SizeX = params.SizeX;
		if (SizeY != nullptr)
			*SizeY = params.SizeY;
	}
};

// Class Engine.DebugCameraController
// 0x0048 (0x0878 - 0x0830)
class ADebugCameraController : public APlayerController
{
public:
	PAD(0x48);

	void ToggleDisplay()
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.DebugCameraController.ToggleDisplay");

		struct {} params;

		ProcessEvent(this, fn, &params);
	}

	static UClass* StaticClass()
	{
		static UClass* ptr = UObject::FindObject<UClass>("Class Engine.DebugCameraController");

		return ptr;
	}
};

// Class Engine.Player
// 0x0020 (0x0048 - 0x0028)
class UPlayer : public UObject
{
public:
	PAD(0x8);									// 0x0028(0x0008) MISSED OFFSET
	class APlayerController* PlayerController;	// 0x0030(0x0008) (ZeroConstructor, Transient, IsPlainOldData)
	PAD(0x10);									// 0x0038(0x0004) (ZeroConstructor, IsPlainOldData)
};

// Class Engine.LocalPlayer
// 0x0180 (0x01C8 - 0x0048)
class ULocalPlayer : public UPlayer
{
public:
	PAD(0x180);	// 0x0048(0x0028) MISSED OFFSET
};


// Class Engine.GameInstance
// 0x00A0 (0x00C8 - 0x0028)
class UGameInstance : public UObject
{
public:
	PAD(0x10);									// 0x0028(0x0010) MISSED OFFSET
	TArray<class ULocalPlayer*> LocalPlayers;	// 0x0038(0x0010) (ZeroConstructor)
	PAD(0x80);									// 0x0048(0x0008) (ZeroConstructor, IsPlainOldData)
};

class UWorld : public UObject
{
public:
	PAD(0x8);									// 0x0028(0x0008)
	class ULevel* PersistentLevel;				// 0x0030(0x0008) (ZeroConstructor, Transient, IsPlainOldData)
	PAD(0x110);									// 0x0038(0x0008) (ZeroConstructor, Transient, IsPlainOldData)
	class AGameStateBase* GameState;			// 0x0148(0x0008) (ZeroConstructor, Transient, IsPlainOldData)
	PAD(0x10);									// 0x0150(0x0008) (ZeroConstructor, Transient, IsPlainOldData)
	TArray<class ULevel*> Levels;				// 0x0160(0x0010) (ZeroConstructor, Transient)
	PAD(0x18);									// 0x0170(0x0010)
	class ULevel* CurrentLevel;					// 0x0188(0x0008) (ZeroConstructor, Transient, IsPlainOldData)
	class UGameInstance* OwningGameInstance;	// 0x0190(0x0008)
	PAD(0x7F0);									// 0x0198(0x07F0) (ZeroConstructor, Transient, IsPlainOldData)
};

// Class Engine.ScriptViewportClient
// Size: 0x38 (Inherited: 0x28)
class UScriptViewportClient : public UObject
{
public:
	PAD(0x20); // 0x28(0x10)
};

class UGameViewportClient : public UScriptViewportClient
{
public:
	PAD(0x40);																// 0x0048(0x0040) MISSED OFFSET
	class UWorld* World;                                                    // 0x0088(0x0008) (ZeroConstructor, IsPlainOldData)
	class UGameInstance* GameInstance;                                      // 0x0090(0x0008) (ZeroConstructor, IsPlainOldData)
	PAD(0x280);																// 0x0098(0x0280) MISSED OFFSET
};

class UEngine : public UObject
{
public:
	PAD(0x8); // 0x28(0x8)
	class UFont* TinyFont; // 0x30(0x8)
	PAD(0x18); // 0x38(0x18)
	class UFont* SmallFont; // 0x50(0x8)
	PAD(0x18); // 0x58(0x18)
	class UFont* MediumFont; // 0x70(0x8)
	PAD(0x18); // 0x78(0x18)
	class UFont* LargeFont; // 0x90(0x8)
	PAD(0x18); // 0x98(0x18)
	class UFont* SubtitleFont; // 0xB0(0x8)
	PAD(0x6F8); // 0xE0(0x6F8)
	class UGameViewportClient* GameViewport; // 0x7B0(0x08)
};

// ScriptStruct CoreUObject.Color
// 0x0004
struct FColor
{
	unsigned char                                      B;                                                        // 0x0000(0x0001) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)
	unsigned char                                      G;                                                        // 0x0001(0x0001) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)
	unsigned char                                      R;                                                        // 0x0002(0x0001) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)
	unsigned char                                      A;                                                        // 0x0003(0x0001) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)

	FColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { B = b; G = g; R = r; A = a; };
	FColor() { B = 0; G = 0; R = 0; A = 0; };
};

// ScriptStruct CoreUObject.Plane
// 0x0004 (0x0010 - 0x000C)
struct alignas(16) FPlane : public FVector
{
	float                                              W;                                                        // 0x000C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)
};

// Class Engine.Canvas
// 0x0298 (0x02C0 - 0x0028)
class UCanvas : public UObject
{
public:
	static inline UFont* roboto = nullptr;				//Font Roboto.Roboto
	static inline UFont* tinyRoboto = nullptr;			//Font RobotoTiny.RobotoTiny
	static inline UFont* droidSansMono = nullptr;		//Font DroidSansMono.DroidSansMono
	static inline UFont* oakBody = nullptr;				//Font OAK_BODY.OAK_BODY
	static inline UFont* posteramaRegular = nullptr;	//Font PosteramaText-Regular_Font.PosteramaText-Regular_Font
	static inline UFont* robotoDistanceField = nullptr;	//Font RobotoDistanceField.RobotoDistanceField
	static inline UFont* defaultFont = nullptr;
	float                                              OrgX;                                                     // 0x0028(0x0004) (ZeroConstructor, IsPlainOldData)
	float                                              OrgY;                                                     // 0x002C(0x0004) (ZeroConstructor, IsPlainOldData)
	float                                              ClipX;                                                    // 0x0030(0x0004) (ZeroConstructor, IsPlainOldData)
	float                                              ClipY;                                                    // 0x0034(0x0004) (ZeroConstructor, IsPlainOldData)
	struct FColor                                      DrawColor;                                                // 0x0038(0x0004) (IsPlainOldData)
	unsigned char                                      bCenterX : 1;                                             // 0x003C(0x0001)
	unsigned char                                      bCenterY : 1;                                             // 0x003C(0x0001)
	unsigned char                                      bNoSmooth : 1;                                            // 0x003C(0x0001)
	unsigned char                                      UnknownData00[0x3];                                       // 0x003D(0x0003) MISSED OFFSET
	int                                                SizeX;                                                    // 0x0040(0x0004) (ZeroConstructor, IsPlainOldData)
	int                                                SizeY;                                                    // 0x0044(0x0004) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData01[0x8];                                       // 0x0048(0x0008) MISSED OFFSET
	struct FPlane                                      ColorModulate;                                            // 0x0050(0x0010) (IsPlainOldData)
	void* DefaultTexture;                                           // 0x0060(0x0008) (ZeroConstructor, IsPlainOldData)
	void* GradientTexture0;                                         // 0x0068(0x0008) (ZeroConstructor, IsPlainOldData)
	PAD(0x250);                                     // 0x0078(0x0248) MISSED OFFSET

	FVector2D UTextSizeS(std::string Text)
	{
		std::wstring wString = std::wstring(Text.begin(), Text.end());

		return K2_TextSize(defaultFont, FString(wString.c_str()), { 1.0f, 1.0f });
	}

	FVector2D K2_TextSize(class UFont* RenderFont, const class FString& RenderText, const class FVector2D& Scale)
	{
		static UObject* fn = UObject::FindObject<UObject>("Function Engine.Canvas.K2_TextSize");

		struct {
			UFont* RenderFont;
			FString RenderText;
			FVector2D Scale;
			FVector2D ReturnValue;
		} params;

		params = { RenderFont, RenderText, Scale };

		ProcessEvent(this, fn, &params);

		return params.ReturnValue;
	}

	/*void DrawCircle(FVector2D screenPosition, float radius, float pi, int numberOfSides, float thickness, const class FLinearColor& renderColor)
	{
		float angle = (float)((2.0f * pi) / numberOfSides);

		FVector2D points[numberOfSides + 1];
		memset(&points, 0, sizeof(points));

		for (int index = 0; index <= numberOfSides; index++)
		{
			float currentAngle = (float)(index * angle);

			points[index] = { (screenPosition.X + radius * cos(currentAngle)), (screenPosition.Y - radius * sin(currentAngle)) };

			if (index != 0)
			{
				this->DrawLine(points[index - 1], points[index], thickness, renderColor);
			}
		}
	}*/

	void UDrawTriangle(ColorVector2D pos1, ColorVector2D pos2, ColorVector2D pos3)
	{
		FCanvasUVTri data;
		memset(&data, 0, sizeof(FCanvasUVTri));
		data.V0_Color = pos1.Color;
		data.V0_Pos = pos1;
		data.V1_Color = pos2.Color;
		data.V1_Pos = pos2;
		data.V2_Color = pos3.Color;
		data.V2_Pos = pos3;

		TArray<FCanvasUVTri> triangles;
		triangles.Data = &data;
		triangles.Count = 1;
		triangles.Max = 1;

		K2_DrawTriangle(triangles);
	}

	void K2_DrawTriangle(TArray<class FCanvasUVTri> Triangles)
	{
		static UObject* fn = UObject::FindObject<UObject>("Function Engine.Canvas.K2_DrawTriangle");

		struct {
			void* RenderTexture;
			TArray<class FCanvasUVTri> Triangles;
		} parms;

		parms = { this->DefaultTexture, Triangles };

		ProcessEvent(this, fn, &parms);
	}

	void DrawPolygon(const class FVector2D& ScreenPosition, const class FVector2D& Radius, int numberOfSides, const class FLinearColor& RenderColor)
	{
		K2_DrawPolygon(this->DefaultTexture, ScreenPosition, Radius, numberOfSides, RenderColor);
	}

	void DrawPolygon(void* texture, const class FVector2D& ScreenPosition, const class FVector2D& Radius, int numberOfSides, const class FLinearColor& RenderColor)
	{
		K2_DrawPolygon(texture, ScreenPosition, Radius, numberOfSides, RenderColor);
	}

	void K2_DrawPolygon(void* texture, const class FVector2D& ScreenPosition, const class FVector2D& Radius, int NumberOfSides, const class FLinearColor& RenderColor)
	{
		static UObject* fn = UObject::FindObject<UObject>("Function Engine.Canvas.K2_DrawPolygon");

		struct {
			void* RenderTexture;
			FVector2D ScreenPosition;
			FVector2D Radius;
			int NumberOfSides;
			FLinearColor RenderColor;
		} parms;

		parms = { texture, ScreenPosition, Radius, NumberOfSides, RenderColor };

		ProcessEvent(this, fn, &parms);
	}

	void K2_DrawTexture(const class FVector2D& ScreenPosition, const class FVector2D& ScreenSize, const class FLinearColor& RenderColor)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.Canvas.K2_DrawTexture");

		struct {
			void* RenderTexture;
			FVector2D ScreenPosition;
			FVector2D ScreenSize;
			FVector2D CoordinatePosition;
			FVector2D CoordinateSize;
			FLinearColor RenderColor;
			EBlendMode BlendMode;
			float Rotation;
			FVector2D PivotPoint;
		} parms;

		parms = { this->DefaultTexture, ScreenPosition, ScreenSize, FVector2D::ZeroVector, FVector2D::OneVector, RenderColor, EBlendMode::BLEND_MAX, 0.0f, FVector2D::ZeroVector };

		ProcessEvent(this, fn, &parms);
	}

	void K2_DrawTexture(void* texture, const class FVector2D& ScreenPosition, const class FVector2D& ScreenSize, const class FLinearColor& RenderColor, uint8_t blendMode, float rotation)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.Canvas.K2_DrawTexture");

		struct {
			void* RenderTexture;
			FVector2D ScreenPosition;
			FVector2D ScreenSize;
			FVector2D CoordinatePosition;
			FVector2D CoordinateSize;
			FLinearColor RenderColor;
			EBlendMode BlendMode;
			float Rotation;
			FVector2D PivotPoint;
		} parms;

		parms = { texture, ScreenPosition, ScreenSize, FVector2D::ZeroVector, FVector2D::OneVector, RenderColor, (EBlendMode)blendMode, 0.0f, FVector2D::HalfVector };

		ProcessEvent(this, fn, &parms);
	}

	void DrawLine(FVector2D ScreenPositionA, FVector2D ScreenPositionB, float Thickness, const class FLinearColor& RenderColor)
	{
		K2_DrawLine(ScreenPositionA, ScreenPositionB, Thickness, RenderColor);
	}

	void DrawLine(FVector2D ScreenPositionA, FVector2D ScreenPositionB, float Thickness, const class FLinearColor& RenderColor, bool outline, bool verticalAlignment, const class FLinearColor& outlineColor)
	{
		K2_DrawLine(ScreenPositionA, ScreenPositionB, Thickness, RenderColor);

		if (outline)
		{
			if (verticalAlignment)
			{
				ScreenPositionA.X -= 1.0f;
				ScreenPositionB.X -= 1.0f;
				K2_DrawLine(ScreenPositionA, ScreenPositionB, Thickness, outlineColor);
				ScreenPositionA.X += 2.0f;
				ScreenPositionB.X += 2.0f;
				K2_DrawLine(ScreenPositionA, ScreenPositionB, Thickness, outlineColor);
			}
			else
			{
				ScreenPositionA.Y -= 1.0f;
				ScreenPositionB.Y -= 1.0f;
				K2_DrawLine(ScreenPositionA, ScreenPositionB, Thickness, outlineColor);
				ScreenPositionA.Y += 2.0f;
				ScreenPositionB.Y += 2.0f;
				K2_DrawLine(ScreenPositionA, ScreenPositionB, Thickness, outlineColor);
			}
		}
	}

	void K2_DrawLine(const class FVector2D& ScreenPositionA, const class FVector2D& ScreenPositionB, float Thickness, const class FLinearColor& RenderColor)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.Canvas.K2_DrawLine");

		struct {
			FVector2D ScreenPositionA;
			FVector2D ScreenPositionB;
			float Thickness;
			FLinearColor RenderColor;
		} parms;

		parms = { ScreenPositionA , ScreenPositionB, Thickness, RenderColor };

		ProcessEvent(this, fn, &parms);
	}

	void DrawBox(FVector2D ScreenPosition, FVector2D ScreenSize, float Thickness, const class FLinearColor& RenderColor, bool outline, const class FLinearColor& outlineColor)
	{
		K2_DrawBox(ScreenPosition, ScreenSize, Thickness, RenderColor);

		if (outline)
		{
			ScreenPosition = ScreenPosition - (1.0f * Thickness);
			ScreenSize = ScreenSize + (2.0f * Thickness);
			K2_DrawBox(ScreenPosition, ScreenSize, Thickness, outlineColor);
			ScreenPosition = ScreenPosition + (2.0f * Thickness);
			ScreenSize = ScreenSize - (4.0f * Thickness);
			K2_DrawBox(ScreenPosition, ScreenSize, Thickness, outlineColor);
		}
	}

	void K2_DrawBox(const class FVector2D& ScreenPosition, const class FVector2D& ScreenSize, float Thickness, const class FLinearColor& RenderColor)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.Canvas.K2_DrawBox");

		struct {
			FVector2D ScreenPosition;
			FVector2D ScreenSize;
			float Thickness;
			FLinearColor RenderColor;
		} parms;

		parms = { ScreenPosition , ScreenSize, Thickness, RenderColor };

		ProcessEvent(this, fn, &parms);
	}

	void UDrawTextCenteredF(FString RenderText, class FVector2D ScreenPosition, class FLinearColor RenderColor)
	{
		K2_DrawText(defaultFont, RenderText, ScreenPosition, RenderColor, false, FLinearColor::Zero, FVector2D::OneVector, true, true, true, FLinearColor::outline);
	}

	void UDrawTextF(FString RenderText, class FVector2D ScreenPosition, class FLinearColor RenderColor)
	{
		K2_DrawText(defaultFont, RenderText, ScreenPosition, RenderColor, false, FLinearColor::Zero, FVector2D::OneVector, false, false, true, FLinearColor::outline);
	}

	void UDrawTextCenteredS(std::string RenderText, class FVector2D ScreenPosition, class FLinearColor RenderColor)
	{
		std::wstring wString = std::wstring(RenderText.begin(), RenderText.end());

		K2_DrawText(defaultFont, FString(wString.c_str()), ScreenPosition, RenderColor, false, FLinearColor::Zero, FVector2D::OneVector, true, true, true, FLinearColor::outline);
	}

	void UDrawTextCenteredS(std::string RenderText, class FVector2D ScreenPosition, class FLinearColor RenderColor, bool outline)
	{
		std::wstring wString = std::wstring(RenderText.begin(), RenderText.end());

		K2_DrawText(defaultFont, FString(wString.c_str()), ScreenPosition, RenderColor, false, FLinearColor::Zero, FVector2D::OneVector, true, true, outline, FLinearColor::outline);
	}

	void UDrawTextCentered(std::wstring RenderText, class FVector2D ScreenPosition, class FLinearColor RenderColor)
	{
		K2_DrawText(defaultFont, FString(RenderText.c_str()), ScreenPosition, RenderColor, false, FLinearColor::Zero, FVector2D::OneVector, true, true, true, FLinearColor::outline);
	}

	void UDrawTextCenteredS(UFont* font, std::string RenderText, class FVector2D ScreenPosition, class FLinearColor RenderColor)
	{
		std::wstring wString = std::wstring(RenderText.begin(), RenderText.end());

		K2_DrawText(font, FString(wString.c_str()), ScreenPosition, RenderColor, false, FLinearColor::Zero, FVector2D::OneVector, true, true, true, FLinearColor::outline);
	}

	void UDrawTextS(UFont* font, std::string RenderText, class FVector2D ScreenPosition, class FLinearColor RenderColor)
	{
		std::wstring wString = std::wstring(RenderText.begin(), RenderText.end());

		K2_DrawText(font, FString(wString.c_str()), ScreenPosition, RenderColor, false, FLinearColor::Zero, FVector2D::OneVector, false, false, true, FLinearColor::outline);
	}

	void UDrawTextSCenterVertical(std::string RenderText, class FVector2D ScreenPosition, class FLinearColor RenderColor, bool outline)
	{
		std::wstring wString = std::wstring(RenderText.begin(), RenderText.end());

		K2_DrawText(defaultFont, FString(wString.c_str()), ScreenPosition, RenderColor, false, FLinearColor::Zero, FVector2D::OneVector, false, true, outline, FLinearColor::outline);
	}

	void UDrawTextS(std::string RenderText, class FVector2D ScreenPosition, class FLinearColor RenderColor, bool verticalCenter)
	{
		std::wstring wString = std::wstring(RenderText.begin(), RenderText.end());

		K2_DrawText(defaultFont, FString(wString.c_str()), ScreenPosition, RenderColor, false, FLinearColor::Zero, FVector2D::OneVector, false, verticalCenter, true, FLinearColor::outline);
	}

	void UDrawTextS(std::string RenderText, class FVector2D ScreenPosition, class FLinearColor RenderColor, bool verticalCenter, bool horizontalCenter)
	{
		std::wstring wString = std::wstring(RenderText.begin(), RenderText.end());

		K2_DrawText(defaultFont, FString(wString.c_str()), ScreenPosition, RenderColor, false, FLinearColor::Zero, FVector2D::OneVector, horizontalCenter, verticalCenter, true, FLinearColor::outline);
	}

	void UDrawTextS(std::string RenderText, class FVector2D ScreenPosition, class FLinearColor RenderColor)
	{
		std::wstring wString = std::wstring(RenderText.begin(), RenderText.end());

		K2_DrawText(defaultFont, FString(wString.c_str()), ScreenPosition, RenderColor, false, FLinearColor::Zero, FVector2D::OneVector, false, false, true, FLinearColor::outline);
	}

	void UDrawTextS(std::string RenderText, class FVector2D ScreenPosition, bool centered, class FLinearColor RenderColor, class FLinearColor OutlineColor)
	{
		std::wstring wString = std::wstring(RenderText.begin(), RenderText.end());

		K2_DrawText(defaultFont, FString(wString.c_str()), ScreenPosition, RenderColor, false, OutlineColor, FVector2D::OneVector, centered, true, true, FLinearColor::outline);
	}

	void UDrawText(std::wstring RenderText, class FVector2D ScreenPosition, class FLinearColor RenderColor)
	{
		K2_DrawText(defaultFont, FString(RenderText.c_str()), ScreenPosition, RenderColor, false, FLinearColor::Zero, FVector2D::OneVector, false, false, true, FLinearColor::outline);
	}

	void K2_DrawText(class UFont* RenderFont, const class FString& RenderText, const class FVector2D& ScreenPosition, const class FLinearColor& RenderColor, float Kerning, const class FLinearColor& ShadowColor, const class FVector2D& ShadowOffset, bool bCentreX, bool bCentreY, bool bOutlined, const class FLinearColor& OutlineColor)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.Canvas.K2_DrawText");

		struct {
			UFont* RenderFont;
			FString RenderText;
			FVector2D ScreenPosition;
			FLinearColor RenderColor;
			float Kerning;
			FLinearColor ShadowColor;
			FVector2D ShadowOffset;
			bool bCentreX;
			bool bCentreY;
			bool bOutlined;
			FLinearColor OutlineColor;
		} parms;

		parms = { RenderFont , RenderText, ScreenPosition, RenderColor, Kerning, ShadowColor, ShadowOffset, bCentreX, bCentreY, bOutlined, OutlineColor };

		ProcessEvent(this, fn, &parms);
	};
};

// Class GbxTest.TestLibrary
// 0x0000 (0x0028 - 0x0028)
class UTestLibrary
{
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class GbxTest.TestLibrary");
	}

	// Function GbxTest.TestLibrary.SimulateInputKey
// (Final, Native, Static, Public, HasOutParms, BlueprintCallable)
// Parameters:
// class APlayerController*       PlayerController               (ConstParm, Parm, OutParm, ZeroConstructor, ReferenceParm, IsPlainOldData)
// class FKey                    Key                            (Parm)
// TEnumAsByte<EInputEvent>       EVENTTYPE                      (Parm, ZeroConstructor, IsPlainOldData)
// float                          AmountDepressed                (Parm, ZeroConstructor, IsPlainOldData)

	void static SimulateInputKey(class APlayerController* PlayerController, const class FKey& Key, TEnumAsByte<EInputEvent> EventType, float AmountDepressed)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function GbxTest.TestLibrary.SimulateInputKey");

		struct {
			class APlayerController* PlayerController;
			class FKey Key;
			TEnumAsByte<EInputEvent> EventType;
			float AmountDepressed;
		} params;

		params = { PlayerController, Key, EventType, AmountDepressed };

		ProcessEvent(defaultObj, fn, &params);
	}
};

class UKismetSystemLibrary
{
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Engine.KismetSystemLibrary");
	}

	static class FString GetDisplayName(class UObject* Object)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.KismetSystemLibrary.GetDisplayName");

		struct {
			UObject* object;
			FString ReturnValue;
		} params;

		params = { Object };

		ProcessEvent(defaultObj, fn, &params);

		return params.ReturnValue;
	}

	static std::string GetDisplayNameS(class UObject* Object)
	{
		auto displayNameF = GetDisplayName(Object);

		return CleanName(displayNameF);
	}

	static std::string CleanName(FString name)
	{
		auto stringReturnValue = name.ToString();

		auto firstUnderscore = stringReturnValue.find('_');

		auto secondUnderscore = stringReturnValue.find('_', firstUnderscore + 1);

		if (firstUnderscore != std::string::npos && secondUnderscore != std::string::npos)
		{
			return stringReturnValue.substr(firstUnderscore + 1, secondUnderscore - firstUnderscore - 1);
		}

		return stringReturnValue;
	}

	static void GetComponentBounds(class USceneComponent* Component, class FVector* Origin, class FVector* BoxExtent, float* SphereRadius)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.KismetSystemLibrary.GetComponentBounds");

		struct {
			USceneComponent* Component;
			FVector Origin;
			FVector BoxExtent;
			float SphereRadius;
		} params;

		params.Component = Component;

		ProcessEvent(defaultObj, fn, &params);

		if (Origin != nullptr)
			*Origin = params.Origin;
		if (BoxExtent != nullptr)
			*BoxExtent = params.BoxExtent;
		if (SphereRadius != nullptr)
			*SphereRadius = params.SphereRadius;
	}

	// Function Engine.KismetSystemLibrary.GetObjectName
// (Final, Native, Static, Public, BlueprintCallable, BlueprintPure)
// Parameters:
// class UObject*                 Object                         (ConstParm, Parm, ZeroConstructor, IsPlainOldData)
// class FString                  ReturnValue                    (Parm, OutParm, ZeroConstructor, ReturnParm)

	static class FString GetObjectName(class UObject* Object)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.KismetSystemLibrary.GetObjectName");

		struct {
			UObject* object;
			FString ReturnValue;
		} params;

		params = { Object };

		ProcessEvent(defaultObj, fn, &params);
		return params.ReturnValue;
	}

	static bool LineTraceSingle(UObject* WorldContextObject, const class FVector& Start, const class FVector& End, struct FHitResult* OutHit)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.KismetSystemLibrary.LineTraceSingle");

		struct {
			UObject* WorldContextObject;
			FVector Start;
			FVector End;
			ETraceTypeQuery TraceChannel;
			bool bTraceComplex;
			class TArray<class AActor*> ActorsToIgnore;
			EDrawDebugTrace DrawDebugType;
			struct FHitResult OutHit;
			bool bIgnoreSelf;
			class FLinearColor TraceColor;
			class FLinearColor TraceHitColor;
			float DrawTime;
			bool ReturnValue;
		} params;

		auto color = FLinearColor();

		auto actorsToIgnore = TArray<class AActor*>();
		params.WorldContextObject = WorldContextObject;
		params.Start = Start;
		params.End = End;
		params.TraceChannel = ETraceTypeQuery::TraceTypeQuery1;
		params.bTraceComplex = true;
		params.ActorsToIgnore = actorsToIgnore;
		params.DrawDebugType = EDrawDebugTrace::None;
		params.bIgnoreSelf = true;
		params.TraceColor = color;
		params.TraceHitColor = color;
		params.DrawTime = 0.0f;
		params.ReturnValue = false;

		ProcessEvent(defaultObj, fn, &params);

		if (OutHit != nullptr)
			*OutHit = params.OutHit;

		return (bool)params.OutHit.bBlockingHit;
	}

	static void DrawDebugSphere(class UObject* WorldContextObject, const class FVector& Center, float Radius, int Segments, const class FLinearColor& LineColor, float Duration, float Thickness)
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.KismetSystemLibrary.DrawDebugSphere");

		struct {
			class UObject* WorldContextObject;
			class FVector Center;
			float Radius;
			int Segments;
			class FLinearColor LineColor;
			float Duration;
			float Thickness;
		} params;

		params.WorldContextObject = WorldContextObject;
		params.Center = Center;
		params.Radius = Radius;
		params.Segments = Segments;
		params.LineColor = LineColor;
		params.Duration = Duration;
		params.Thickness = Thickness;

		ProcessEvent(defaultObj, fn, &params);
	}

	// Function Engine.KismetMathLibrary.Conv_ColorToLinearColor
// 00007FF7A603AA20
// (Final, Native, Static, Public, HasDefaults, BlueprintCallable, BlueprintPure)
// Parameters:
// struct FColor                  InColor                        (Parm, IsPlainOldData)
// struct FLinearColor            ReturnValue                    (Parm, OutParm, ReturnParm, IsPlainOldData)

};

// Class Engine.KismetMathLibrary
// Size: 0x28 (Inherited: 0x28)
class UKismetMathLibrary : public UObject
{
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Engine.KismetMathLibrary");
	}

	static class FRotator NormalizedDeltaRotator(class FRotator A, class FRotator B)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.KismetMathLibrary.NormalizedDeltaRotator");

		struct
		{
			class FRotator A;
			class FRotator B;
			class FRotator ReturnValue;
		} params;

		params.A = A;
		params.B = B;

		ProcessEvent(defaultObj, fn, &params);

		return params.ReturnValue;
	}

	static FRotator FindLookAtRotation(const FVector& Start, const FVector& Target)
	{
		static UObject* fn = nullptr;

		if (fn == nullptr)
			fn = UObject::FindObject<UObject>("Function Engine.KismetMathLibrary.FindLookAtRotation");

		struct
		{
			FVector Start;
			FVector Target;
			FRotator ReturnValue;
		} params;

		params.Start = Start;
		params.Target = Target;
		ProcessEvent(defaultObj, fn, &params);
		return params.ReturnValue;
	}

	static FLinearColor Conv_ColorToLinearColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		return Conv_ColorToLinearColor(FColor(r, g, b, a));
	}

	static FLinearColor Conv_ColorToLinearColor(const struct FColor& InColor)
	{
		static UObject* fn = UObject::FindObject<UObject>("Function Engine.KismetMathLibrary.Conv_ColorToLinearColor");

		struct {
			FColor object;
			FLinearColor ReturnValue;
		} params;

		params = { InColor };

		ProcessEvent(defaultObj, fn, &params);

		return params.ReturnValue;
	}
};

// ScriptStruct SlateCore.InputEvent
// 0x0018
struct FInputEvent
{
	PAD(0x18);                                      // 0x0000(0x0018) MISSED OFFSET
};

// ScriptStruct SlateCore.PointerEvent
// 0x0058 (0x0070 - 0x0018)
struct FPointerEvent : public FInputEvent
{
	PAD(0x58);                                      // 0x0018(0x0058) MISSED OFFSET
};

// Class Engine.KismetInputLibrary
// 0x0000 (0x0028 - 0x0028)
class UKismetInputLibrary : public UObject
{
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Engine.KismetInputLibrary");
	}

	// Function Engine.KismetInputLibrary.PointerEvent_IsMouseButtonDown
	// (Final, Native, Static, Public, HasOutParms, BlueprintCallable, BlueprintPure)
	// Parameters:
	// struct FPointerEvent           Input                          (ConstParm, Parm, OutParm, ReferenceParm)
	// class FKey                    MouseButton                    (Parm)
	// bool                           ReturnValue                    (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData)
	static bool PointerEvent_IsMouseButtonDown(const struct FPointerEvent& Input, const class FKey& MouseButton)
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.KismetInputLibrary.PointerEvent_IsMouseButtonDown");

		struct {
			struct FPointerEvent Input;
			class FKey Key;
			bool ReturnValue;
		} params;

		params = { Input, MouseButton, false };

		ProcessEvent(defaultObj, fn, &params);

		return params.ReturnValue;
	}
};

class ValidTarget
{
public:
	USkeletalMeshComponent* Mesh;
	FVector Point;
};