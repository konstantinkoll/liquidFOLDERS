
#pragma once
#include <assert.h>

struct SortParameters;

typedef INT(__stdcall* PFNCOMPARE)(LPCVOID pData1, LPCVOID pData2, const SortParameters& Parameters);
typedef void(__stdcall* PFNSWAP)(LPBYTE pData1, LPBYTE pData2, SIZE_T szData);

struct SortParameters
{
	LPBYTE pMemory;
	UINT ItemCount;
	SIZE_T szData;
	PFNCOMPARE zCompare;
	UINT Attr;
	BOOL Descending;
	BOOL Parameter1;
	BOOL Parameter2;
	PFNSWAP zSwap;
};
