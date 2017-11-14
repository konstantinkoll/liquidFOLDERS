
#include "stdafx.h"
#include "Filters.h"
#include "LFCore.h"
#include "LFVariantData.h"
#include "TableAttributes.h"
#include "Stores.h"
#include <assert.h>


extern LFMessageIDs LFMessages;


LFCORE_API LFFilter* LFAllocFilter(const LFFilter* pFilter)
{
	LFFilter* pNewFilter = new LFFilter;

	if (pFilter)
	{
		*pNewFilter = *pFilter;
		pNewFilter->pConditionList = NULL;

		LFFilterCondition* pFilterCondition = pFilter->pConditionList;
		while (pFilterCondition)
		{
			pNewFilter->pConditionList = LFAllocFilterCondition(pFilterCondition->Compare, pFilterCondition->AttrData, pNewFilter->pConditionList);

			pFilterCondition = pFilterCondition->pNext;
		}
	}
	else
	{
		ZeroMemory(pNewFilter, sizeof(LFFilter));
	}

	return pNewFilter;
}

LFCORE_API void LFFreeFilter(LFFilter* pFilter)
{
	if (pFilter)
	{
		LFFilterCondition* pFilterCondition = pFilter->pConditionList;
		while (pFilterCondition)
		{
			LFFilterCondition* pVictim = pFilterCondition;
			pFilterCondition = pFilterCondition->pNext;

			LFFreeFilterCondition(pVictim);
		}

		delete pFilter;
	}
}

LFCORE_API LFFilter* LFLoadFilter(LFItemDescriptor* pItemDescriptor)
{
	WCHAR Path[MAX_PATH];
	if (LFGetFileLocation(pItemDescriptor, Path, MAX_PATH)!=LFOk)
		return NULL;

	LFFilter* pFilter = LoadFilter(Path, pItemDescriptor->StoreID);
	if (pFilter)
		wcscpy_s(pFilter->OriginalName, 256, pItemDescriptor->CoreAttributes.FileName);

	return pFilter;
}

LFCORE_API LFFilter* LFLoadFilterEx(LPCWSTR pPath)
{
	assert(pPath);

	if (!GetMutexForStores())
		return NULL;

	WCHAR Path[MAX_PATH];
	wcscpy_s(Path, MAX_PATH, pPath);

	WCHAR* pChar = wcsrchr(Path, L'\\');
	if (pChar)
	{
		*(++pChar) = L'\0';

		pChar = pChar-Path+(LPWSTR)pPath;
	}
	else
	{
		pChar = (LPWSTR)pPath;
	}

	LFStoreDescriptor* pStoreDescriptor = FindStore(Path);

	LFFilter* pFilter = LoadFilter(pPath, pStoreDescriptor ? pStoreDescriptor->StoreID : "");
	if (pFilter)
	{
		wcscpy_s(pFilter->OriginalName, 256, pChar);

		pChar = wcschr(pFilter->OriginalName, L'.');
		if (pChar)
			*pChar = L'\0';
	}

	ReleaseMutexForStores();

	return pFilter;
}

LFCORE_API UINT LFSaveFilter(LPCSTR pStoreID, LFFilter* pFilter, LPCWSTR pName, LPCWSTR pComment)
{
	assert(pStoreID);
	assert(pFilter);
	assert(pName);

	UINT Result;

	// Find store
	CHAR Store[LFKeySize];
	strcpy_s(Store, LFKeySize, pStoreID);

	if (Store[0]=='\0')
		if ((Result=LFGetDefaultStore(Store))!=LFOk)
			return Result;

	CStore* pStore;
	if ((Result=OpenStore(Store, pStore))==LFOk)
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


LFCORE_API LFFilterCondition* LFAllocFilterCondition(BYTE Compare, LFVariantData& Value, LFFilterCondition* pNext)
{
	LFFilterCondition* pFilterCondition = new LFFilterCondition;

	pFilterCondition->pNext = pNext;
	pFilterCondition->AttrData = Value;
	pFilterCondition->Compare = Compare;

	return pFilterCondition;
}

LFCORE_API LFFilterCondition* LFAllocFilterConditionEx(BYTE Compare, UINT Attr, LFFilterCondition* pNext)
{
	LFFilterCondition* pFilterCondition = new LFFilterCondition;

	pFilterCondition->pNext = pNext;
	pFilterCondition->AttrData.Attr = Attr;
	pFilterCondition->AttrData.Type = (Attr<LFAttributeCount) ? AttrProperties[Attr].Type : LFTypeUnicodeString;
	pFilterCondition->AttrData.IsNull = FALSE;
	pFilterCondition->Compare = Compare;

	return pFilterCondition;
}


// Filters
//

LFFilter* LoadFilter(LPCWSTR pFilename, LPCSTR StoreID)
{
	assert(pFilename);
	assert(StoreID);

	HANDLE hFile = CreateFile(pFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return NULL;

	LFFilter* pFilter = NULL;

	LFPersistentFilterHeader Header;

	DWORD Read;
	if (!ReadFile(hFile, &Header, sizeof(Header), &Read, NULL))
		goto Leave;

	if (strcmp(Header.ID, "LFFilter")!=0)
		goto Leave;

	pFilter = LFAllocFilter();

	pFilter->Mode = LFFilterModeSearch;
	pFilter->Options.IsPersistent = TRUE;

	if (!Header.AllStores)
		strcpy_s(pFilter->StoreID, LFKeySize, StoreID);

	wcscpy_s(pFilter->Searchterm, 256, Header.Searchterm);

	for (UINT a=Header.cConditions; a>0; a--)
	{
		if (SetFilePointer(hFile, sizeof(Header)+(a-1)*Header.szCondition, NULL, FILE_BEGIN)==INVALID_SET_FILE_POINTER)
			goto Leave;

		LFPersistentFilterCondition Condition;
		ZeroMemory(&Condition, sizeof(Condition));

		if (!ReadFile(hFile, &Condition, min(Header.szCondition, sizeof(Condition)), &Read, NULL))
			goto Leave;

		for (UINT a=0; a<LFAttributeCount; a++)
			if (AttrProperties[a].PersistentID==Condition.AttrData.Attr)
			{
				// Replace persistent ID with actual sequential number
				Condition.AttrData.Attr = a;

				LFFilterCondition* pFilterCondition = new LFFilterCondition;
				*pFilterCondition = Condition;
				pFilterCondition->pNext = pFilter->pConditionList;

				pFilter->pConditionList = pFilterCondition;

				break;
			}
	}

Leave:
	CloseHandle(hFile);

	return pFilter;
}

BOOL StoreFilter(LPCWSTR pFilename, LFFilter* pFilter)
{
	assert(pFilename);
	assert(pFilter);

	if (pFilter->Mode!=LFFilterModeSearch)
		return FALSE;

	LFPersistentFilterHeader Header;
	ZeroMemory(&Header, sizeof(Header));

	strcpy_s(Header.ID, 9, "LFFilter");
	Header.Version = 1;
	Header.szCondition = sizeof(LFPersistentFilterCondition);
	Header.AllStores = (pFilter->StoreID[0]=='\0');
	wcscpy_s(Header.Searchterm, 256, pFilter->Searchterm);

	LFFilterCondition* pFilterCondition = pFilter->pConditionList;
	while (pFilterCondition)
	{
		Header.cConditions++;

		pFilterCondition = pFilterCondition->pNext;
	}

	BOOL Result = FALSE;

	HANDLE hFile = CreateFile(pFilename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		DWORD Written;
		if (WriteFile(hFile, &Header, sizeof(Header), &Written, NULL))
		{
			pFilterCondition = pFilter->pConditionList;
			while (pFilterCondition)
			{
				// Replace sequential number with persistent ID
				LFPersistentFilterCondition Condition;
				Condition = *pFilterCondition;
				Condition.AttrData.Attr = AttrProperties[Condition.AttrData.Attr].PersistentID;
				Condition.pNext = NULL;

				if (!WriteFile(hFile, &Condition, sizeof(LFFilterCondition), &Written, NULL))
					break;

				pFilterCondition = pFilterCondition->pNext;
			}

			Result = TRUE;
		}

		CloseHandle(hFile);
	}

	return Result;
}
