#include "pch.h"

UEngine* GEngine = nullptr;

auto FWeakObjectPtr::IsValid() const noexcept -> bool
{
	if (ObjectSerialNumber == 0)
		return false;

	if (ObjectIndex < 0 || ObjectIndex >= UObject::GObjects->Num())
		return false;

	const auto& item = UObject::GetObjectByIndex<FUObjectItem>(ObjectIndex);
	if (item->SerialNumber != ObjectSerialNumber)
		return false;

	return !(item->IsUnreachable() || item->IsPendingKill());
}

auto FWeakObjectPtr::Get() const noexcept -> class UObject*
{
	if (!IsValid())
		return nullptr;

	auto item = UObject::GetObjectByIndex<FUObjectItem>(ObjectIndex);
	return item->Object;
}

std::string UObject::GetFullName() const
{
	std::string temp;

	for (auto outer = this->OuterPrivate; outer; outer = outer->OuterPrivate)
	{
		temp = outer->NamePrivate.GetString() + "." + temp;
	}

	auto objectClass = this->ClassPrivate;
	auto objectName = this->NamePrivate.GetString();
	std::string name = objectClass->NamePrivate.GetString() + " " + temp + objectName;
	return name;
}

UObject* FChunkedFixedUObjectArray::FindObject(const std::string& name) const
{
	for (uint32_t i = 0; i < NumElements; i++)
	{
		auto object = GetObjectPtr(i)->Object;

		if (object != nullptr && object->GetFullName() == name)
		{
			return object;
		}
	}
	return nullptr;
}

bool UObject::IsA(UClass* cmp) const
{
	for (auto super = ClassPrivate; super; super = static_cast<UClass*>(super->SuperStruct))
	{
		if (super == cmp)
		{
			return true;
		}
	}
	return false;
}
