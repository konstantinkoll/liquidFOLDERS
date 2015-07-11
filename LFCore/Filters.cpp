
#include "stdafx.h"
#include "Filters.h"
#include "LFCore.h"
#include "LFVariantData.h"
#include "Mutex.h"
#include "ShellProperties.h"
#include "Stores.h"
#include "StoreCache.h"
#include <assert.h>


extern HANDLE Mutex_Stores;


LFCORE_API LFFilter* LFAllocFilter(LFFilter* pFilter)
{
	LFFilter* pNewFilter = new LFFilter;

	if (pFilter)
	{
		*pNewFilter = *pFilter;
		pNewFilter->ConditionList = NULL;

		LFFilterCondition* pFilterCondition = pFilter->ConditionList;
		while (pFilterCondition)
		{
			pNewFilter->ConditionList = LFAllocFilterCondition(pFilterCondition->Compare, pFilterCondition->AttrData, pNewFilter->ConditionList);

			pFilterCondition = pFilterCondition->Next;
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
		LFFilterCondition* pFilterCondition = pFilter->ConditionList;
		while (pFilterCondition)
		{
			LFFilterCondition* pVictim = pFilterCondition;
			pFilterCondition = pFilterCondition->Next;

			LFFreeFilterCondition(pVictim);
		}

		delete pFilter;
	}
}

LFCORE_API LFFilter* LFLoadFilter(LFItemDescriptor* pItemDescriptor)
{
	WCHAR Path[2*MAX_PATH];
	if (LFGetFileLocation(pItemDescriptor, Path, 2*MAX_PATH, TRUE, TRUE, TRUE)!=LFOk)
		return NULL;

	LFFilter* pFilter = LoadFilter(Path, pItemDescriptor->StoreID);
	if (pFilter)
		wcscpy_s(pFilter->OriginalName, 256, pItemDescriptor->CoreAttributes.FileName);

	return pFilter;
}

LFCORE_API LFFilter* LFLoadFilterEx(WCHAR* pFilename)
{
	assert(pFilename);

	if (!GetMutex(Mutex_Stores))
		return NULL;

	WCHAR Path[2*MAX_PATH];
	wcscpy_s(Path, 2*MAX_PATH, pFilename);

	WCHAR* Ptr = wcsrchr(Path, L'\\');
	if (Ptr)
	{
		*(++Ptr) = L'\0';

		Ptr = Ptr-Path+pFilename;
	}
	else
	{
		Ptr = pFilename;
	}

	MessageBox(NULL,Path,0,0);
	MessageBox(NULL,Ptr,0,0);
	LFStoreDescriptor* slot = FindStore(Path);

	LFFilter* pFilter = LoadFilter(pFilename, slot ? slot->StoreID : "");
	if (pFilter)
	{
		wcscpy_s(pFilter->OriginalName, 256, Ptr);

		Ptr = wcschr(pFilter->OriginalName, L'.');
		if (Ptr)
			*Ptr = L'\0';
	}

	ReleaseMutex(Mutex_Stores);

	return pFilter;
}

LFCORE_API UINT LFSaveFilter(CHAR* StoreID, LFFilter* pFilter, WCHAR* pName, WCHAR* pComments)
{
	assert(StoreID);
	assert(pFilter);
	assert(pName);

	// Store finden
	CHAR Store[LFKeySize];
	strcpy_s(Store, LFKeySize, StoreID);

	if (Store[0]=='\0')
		if (!LFGetDefaultStore(Store))
			return LFNoDefaultStore;

	// Store öffnen
	CIndex* idx1;
	CIndex* idx2;
	LFStoreDescriptor* slot;
	HANDLE StoreLock = NULL;
	UINT Result = OpenStore(Store, TRUE, idx1, idx2, &slot, &StoreLock);
	if (Result==LFOk)
	{
		LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptor();

		SetAttribute(pItemDescriptor, LFAttrFileName, pName);
		SetAttribute(pItemDescriptor, LFAttrFileFormat, "filter");
		if (pComments)
			SetAttribute(pItemDescriptor, LFAttrComments, pComments);

		WCHAR Filename[2*MAX_PATH];
		Result = PrepareImport(slot, pItemDescriptor, Filename, 2*MAX_PATH);
		if (Result==LFOk)
			if (StoreFilter(Filename, pFilter))
			{
				SetAttributesFromFile(pItemDescriptor, Filename, FALSE);

				if (idx1)
					idx1->AddItem(pItemDescriptor);
				if (idx2)
					idx2->AddItem(pItemDescriptor);
			}
			else
			{
				WCHAR* Ptr = wcsrchr(Filename, L'\\');
				if (Ptr)
					*(++Ptr) = L'\0';

				RemoveDir(Filename);
				Result = LFIllegalPhysicalPath;
			}

		LFFreeItemDescriptor(pItemDescriptor);

		if (idx1)
			delete idx1;
		if (idx2)
			delete idx2;
		ReleaseMutexForStore(StoreLock);
	}

	return Result;
}


LFCORE_API LFFilterCondition* LFAllocFilterCondition(BYTE Compare, LFVariantData& v, LFFilterCondition* pNext)
{
	LFFilterCondition* pFilterCondition = new LFFilterCondition();

	pFilterCondition->Next = pNext;
	pFilterCondition->AttrData = v;
	pFilterCondition->Compare = Compare;

	return pFilterCondition;
}

LFCORE_API LFFilterCondition* LFAllocFilterConditionEx(BYTE Compare, UINT Attr, LFFilterCondition* pNext)
{
	LFFilterCondition* pFilterCondition = new LFFilterCondition();

	pFilterCondition->Next = pNext;
	pFilterCondition->AttrData.Attr = Attr;
	pFilterCondition->AttrData.Type = (Attr<LFAttributeCount) ? AttrTypes[Attr] : LFTypeUnicodeString;
	pFilterCondition->AttrData.IsNull = FALSE;
	pFilterCondition->Compare = Compare;

	return pFilterCondition;
}


// Filters
//

LFFilter* LoadFilter(WCHAR* pFilename, CHAR* StoreID)
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

		LFFilterCondition* pFilterCondition = new LFFilterCondition();
		memcpy_s(pFilterCondition, sizeof(LFFilterCondition), &Condition, sizeof(Condition));
		pFilterCondition->Next = pFilter->ConditionList;

		pFilter->ConditionList = pFilterCondition;
	}

Leave:
	CloseHandle(hFile);

	return pFilter;
}

BOOL StoreFilter(WCHAR* pFilename, LFFilter* pFilter)
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

	LFFilterCondition* pFilterCondition = pFilter->ConditionList;
	while (pFilterCondition)
	{
		Header.cConditions++;

		pFilterCondition = pFilterCondition->Next;
	}

	BOOL Result = FALSE;

	HANDLE hFile = CreateFile(pFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		DWORD Written;
		if (WriteFile(hFile, &Header, sizeof(Header), &Written, NULL))
		{
			pFilterCondition = pFilter->ConditionList;
			while (pFilterCondition)
			{
				if (!WriteFile(hFile, pFilterCondition, sizeof(LFFilterCondition), &Written, NULL))
					break;

				pFilterCondition = pFilterCondition->Next;
			}

			Result = TRUE;
		}

		CloseHandle(hFile);
	}

	return Result;
}
