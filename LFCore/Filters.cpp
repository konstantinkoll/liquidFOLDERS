
#include "stdafx.h"
#include "Filters.h"
#include "LFCore.h"
#include "LFVariantData.h"
#include "TableAttributes.h"
#include "Stores.h"


extern LFMessageIDs LFMessages;


LFCORE_API LFFilter* LFAllocFilter(BYTE Mode)
{
	LFFilter* pNewFilter = new LFFilter;
	ZeroMemory(pNewFilter, sizeof(LFFilter));

	pNewFilter->Query.Mode = Mode;
	pNewFilter->Query.Context = LFContextAuto;

	return pNewFilter;
}

LFCORE_API void LFFreeFilter(LFFilter* pFilter)
{
	if (pFilter)
	{
		while (pFilter->Query.pConditionList)
		{
			LFFilterCondition* pVictim = pFilter->Query.pConditionList;
			pFilter->Query.pConditionList = pFilter->Query.pConditionList->pNext;

			LFFreeFilterCondition(pVictim);
		}

		delete pFilter;
	}
}

LFCORE_API LFFilter* LFLoadFilter(LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	// Path
	WCHAR Path[MAX_PATH];
	if (LFGetFileLocation(pItemDescriptor, Path, MAX_PATH)!=LFOk)
		return NULL;

	// Open file
	HANDLE hFile = CreateFile(Path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return NULL;

	LFFilter* pFilter = NULL;

	// Read and verify header
	LFPersistentFilterHeader Header;
	DWORD Read;
	if (!ReadFile(hFile, &Header, sizeof(Header), &Read, NULL))
		goto Leave;

	if (strcmp(Header.ID, "LFFilter")!=0)
		goto Leave;

	// Allocate filter and set up data
	pFilter = LFAllocFilter();
	pFilter->IsPersistent = TRUE;
	wcscpy_s(pFilter->Name, 256, pItemDescriptor->CoreAttributes.FileName);

	if (!Header.AllStores)
		pFilter->Query.StoreID = pItemDescriptor->StoreID;

	wcscpy_s(pFilter->Query.SearchTerm, 256, Header.SearchTerm);

	// Filter conditions
	for (UINT a=Header.cConditions; a>0; a--)
	{
		if (SetFilePointer(hFile, sizeof(Header)+(a-1)*Header.szCondition, NULL, FILE_BEGIN)==INVALID_SET_FILE_POINTER)
			goto Leave;

		LFPersistentFilterCondition Condition;
		ZeroMemory(&Condition, sizeof(Condition));

		if (!ReadFile(hFile, &Condition, min(Header.szCondition, sizeof(Condition)), &Read, NULL))
			goto Leave;

		// Find actual attribute number from persistent ID
		for (UINT a=0; a<LFAttributeCount; a++)
			if (AttrProperties[a].PersistentID==Condition.VData.Attr)
			{
				// Replace persistent ID with actual sequential number
				Condition.VData.Attr = a;

				LFFilterCondition* pFilterCondition = new LFFilterCondition(Condition);
				pFilterCondition->pNext = pFilter->Query.pConditionList;

				pFilter->Query.pConditionList = pFilterCondition;

				break;
			}
	}

Leave:
	CloseHandle(hFile);

	return pFilter;
}

LFCORE_API UINT LFSaveFilter(const STOREID& StoreID, LFFilter* pFilter, LPCWSTR pName, LPCWSTR pComment)
{
	assert(pFilter);
	assert(pName);

	UINT Result;
	CStore* pStore;
	if ((Result=OpenStore(StoreID, pStore))==LFOk)
	{
		// Prepare item descriptor
		LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptor();

		if (pComment)
			SetAttribute(pItemDescriptor, LFAttrComments, pComment);

		// Import
		WCHAR Path[2*MAX_PATH];
		if ((Result=pStore->PrepareImport(pName, "filter", pItemDescriptor, Path, 2*MAX_PATH))==LFOk)
			Result = pStore->CommitImport(pItemDescriptor, StoreFilter(Path, pFilter), Path);

		delete pItemDescriptor;
		delete pStore;

		if (Result==LFOk)
			SendLFNotifyMessage(LFMessages.StatisticsChanged);
	}

	return Result;
}


LFCORE_API LFFilterCondition* LFAllocFilterCondition(BYTE Compare, const LFVariantData& VData, LFFilterCondition* pNext)
{
	LFFilterCondition* pFilterCondition = new LFFilterCondition;

	pFilterCondition->pNext = pNext;
	pFilterCondition->VData = VData;
	pFilterCondition->Compare = Compare;

	return pFilterCondition;
}


// Filters
//

LFFilter* CloneFilter(const LFFilter* pFilter)
{
	assert(pFilter);

	// Filter
	LFFilter* pNewFilter = new LFFilter(*pFilter);
	pNewFilter->Query.pConditionList = NULL;

	// Filter conditions
	LFFilterCondition* pFilterCondition = pFilter->Query.pConditionList;
	while (pFilterCondition)
	{
		pNewFilter->Query.pConditionList = LFAllocFilterCondition(pFilterCondition->Compare, pFilterCondition->VData, pNewFilter->Query.pConditionList);

		pFilterCondition = pFilterCondition->pNext;
	}

	return pNewFilter;
}

BOOL StoreFilter(LPCWSTR pPath, LFFilter* pFilter)
{
	assert(pPath);
	assert(pFilter);

	// Filter mode
	if (pFilter->Query.Mode!=LFFilterModeQuery)
		return FALSE;

	// Create file header
	LFPersistentFilterHeader Header;
	ZeroMemory(&Header, sizeof(Header));

	strcpy_s(Header.ID, 9, "LFFilter");
	Header.Version = 1;
	Header.szCondition = sizeof(LFPersistentFilterCondition);
	Header.AllStores = LFIsDefaultStoreID(pFilter->Query.StoreID);
	wcscpy_s(Header.SearchTerm, 256, pFilter->Query.SearchTerm);

	// Condition count
	LFFilterCondition* pFilterCondition = pFilter->Query.pConditionList;
	while (pFilterCondition)
	{
		Header.cConditions++;

		pFilterCondition = pFilterCondition->pNext;
	}

	// Write to file
	BOOL Result = FALSE;

	HANDLE hFile = CreateFile(pPath, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		DWORD Written;
		if (WriteFile(hFile, &Header, sizeof(Header), &Written, NULL))
		{
			pFilterCondition = pFilter->Query.pConditionList;
			while (pFilterCondition)
			{
				// Replace sequential number with persistent ID
				LFPersistentFilterCondition Condition(*pFilterCondition);
				Condition.VData.Attr = AttrProperties[Condition.VData.Attr].PersistentID;
				Condition.pNext = NULL;

				if (!WriteFile(hFile, &Condition, sizeof(LFPersistentFilterCondition), &Written, NULL))
					break;

				pFilterCondition = pFilterCondition->pNext;
			}

			Result = TRUE;
		}

		CloseHandle(hFile);
	}

	return Result;
}
