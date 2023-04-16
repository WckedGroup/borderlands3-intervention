#pragma once
#include "pch.h"

#define CONCAT_IMPL(x, y) x##y
#define MACRO_CONCAT(x, y) CONCAT_IMPL(x, y)
#define PAD(SIZE) BYTE MACRO_CONCAT(_pad, __COUNTER__)[SIZE];

bool DataCompare(PBYTE pData, PBYTE bSig, const char* szMask);

DWORD_PTR FindPattern(DWORD_PTR dwAddress, DWORD dwSize, const char* pbSig, const char* szMask, long offset);

DWORD_PTR FindPointerPattern(DWORD_PTR dwAddress, DWORD dwSize, const char* pbSig, const char* szMask, long offset);

bool PatchMem(void* address, void* bytes, uint64_t size);