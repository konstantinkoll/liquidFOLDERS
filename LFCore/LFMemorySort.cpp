
#include "stdafx.h"
#include "LFCore.h"
#include "LFMemorySort.h"


void Swap4(LPBYTE pData1, LPBYTE pData2, SIZE_T /*szData*/)
{
	assert(pData1);
	assert(pData2);

	const UINT Temp = *((UINT*)pData1);
	*((UINT*)pData1) = *((UINT*)pData2);
	*((UINT*)pData2) = Temp;
}

void Swap8(LPBYTE pData1, LPBYTE pData2, SIZE_T /*szData*/)
{
	assert(pData1);
	assert(pData2);

	const UINT64 Temp = *((UINT64*)pData1);
	*((UINT64*)pData1) = *((UINT64*)pData2);
	*((UINT64*)pData2) = Temp;
}

void SwapGeneric(LPBYTE pData1, LPBYTE pData2, SIZE_T szData)
{
	assert(pData1);
	assert(pData2);

	// Swap QWORDs
	while (szData>=8)
	{
		const UINT64 Temp = *((UINT64*)pData1);
		*((UINT64*)pData1) = *((UINT64*)pData2);
		pData1 += 8;

		*((UINT64*)pData2) = Temp;
		pData2 += 8;

		szData -= 8;
	}

	// Swap remaining BYTEs
	while (szData>0)
	{
		const BYTE Temp = *pData1;
		*(pData1++) = *pData2;
		*(pData2++) = Temp;

		szData--;
	}
}

inline LPBYTE GetItemData(INT Index, const SortParameters& Parameters)
{
	assert(Index>=0);
	assert(Index<(INT)Parameters.ItemCount);

	return Parameters.pMemory+Index*Parameters.szData;
}

void Heap(INT Element, INT Count, const SortParameters& Parameters)
{
	while (Element<=Count/2-1)
	{
		INT Index = (Element+1)*2-1;
		if ((Index+1<Count) && (Parameters.zCompare(GetItemData(Index, Parameters), GetItemData(Index+1, Parameters), Parameters)<0))
			Index++;

		LPBYTE pData1;
		LPBYTE pData2;
		if (Parameters.zCompare(pData1=GetItemData(Element, Parameters), pData2=GetItemData(Index, Parameters), Parameters)<0)
		{
			Parameters.zSwap(pData1, pData2, Parameters.szData);
			Element = Index;
		}
		else
		{
			break;
		}
	}
}

void SortMemory(const SortParameters& Parameters)
{
	if (Parameters.ItemCount>1)
	{
		assert(Parameters.pMemory);

		for (INT a=Parameters.ItemCount/2-1; a>=0; a--)
			Heap(a, Parameters.ItemCount, Parameters);

		for (INT a=Parameters.ItemCount-1; a>0; a--)
		{
			Parameters.zSwap(GetItemData(0, Parameters), GetItemData(a, Parameters), Parameters.szData);
			Heap(0, a, Parameters);
		}
	}
}

LFCORE_API void LFSortMemory(LPVOID pMemory, UINT ItemCount, SIZE_T szData, PFNCOMPARE zCompare, ATTRIBUTE Attr, BOOL Descending, BOOL Parameter1, BOOL Parameter2)
{
	assert(szData>0);
	assert(zCompare);

	const SortParameters Parameters = { (LPBYTE)pMemory, ItemCount, szData, zCompare, Attr, Descending, Parameter1, Parameter2,
		szData==8 ? &Swap8 :
		szData==4 ? &Swap4 :
		&SwapGeneric };

	SortMemory(Parameters);
}
