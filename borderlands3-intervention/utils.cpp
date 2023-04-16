#include "pch.h"

bool DataCompare(PBYTE pData, PBYTE bSig, const char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bSig)
	{
		if (*szMask == 'x' && *pData != *bSig)
			return false;
	}
	return (*szMask) == 0;
}

DWORD_PTR FindPattern(DWORD_PTR dwAddress, DWORD dwSize, const char* pbSig, const char* szMask, long offset)
{
	size_t length = strlen(szMask);
	for (size_t i = NULL; i < dwSize - length; i++)
	{
		if (DataCompare((PBYTE)dwAddress + i, (PBYTE)pbSig, szMask))
			return dwAddress + i + offset;
	}
	return 0;
}

DWORD_PTR FindPointerPattern(DWORD_PTR dwAddress, DWORD dwSize, const char* pbSig, const char* szMask, long offset)
{
	auto address = FindPattern(dwAddress, dwSize, pbSig, szMask, 0);

	if (address == 0)
		return 0;

	auto ptr = *(DWORD32*)(address + offset);

	return ptr + address + offset + 4;
}

bool PatchMem(void* address, void* bytes, uint64_t size)
{
	DWORD oldProtection;
	if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtection)) { return false; }
	memcpy(address, bytes, size);
	return VirtualProtect(address, size, oldProtection, &oldProtection);
}