
#include "stdafx.h"
#include "LFCore.h"
#include "PersistentFilters.h"
#include "Mutex.h"
#include "ShellProperties.h"
#include "Stores.h"
#include "StoreCache.h"
#include <assert.h>


extern HANDLE Mutex_Stores;

bool StoreFilter(wchar_t* fn, LFFilter* filter)
{
	assert(fn);
	assert(filter);

	PersistentFilterHeader Header;
	ZeroMemory(&Header, sizeof(Header));
	strcpy_s(Header.ID, 9, "LFFilter");
	Header.Version = 1;
	Header.szHeader = sizeof(PersistentFilterHeader);
	Header.szBody = sizeof(PersistentFilterBody);
	Header.szCondition = sizeof(PersistentFilterCondition);

	PersistentFilterBody Body;
	ZeroMemory(&Body, sizeof(Body));
	strcpy_s(Body.StoreID, LFKeySize, filter->StoreID);
	wcscpy_s(Body.Searchterm, 256, filter->Searchterm);
	LFFilterCondition* Condition = filter->ConditionList;
	while (Condition)
	{
		Body.cConditions++;
		Condition = Condition->Next;
	}

	bool res = false;
	HANDLE hFile = CreateFile(fn, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		DWORD Written;
		if (WriteFile(hFile, &Header, sizeof(Header), &Written, NULL))
			if (WriteFile(hFile, &Body, sizeof(Body), &Written, NULL))
			{
				res = true;
				Condition = filter->ConditionList;
				while (Condition)
				{
					if (!WriteFile(hFile, Condition, sizeof(PersistentFilterCondition), &Written, NULL))
					{
						res = false;
						break;
					}
					Condition = Condition->Next;
				}
			}

		CloseHandle(hFile);
	}

	return res;
}

LFCore_API unsigned int LFCreateFilter(char* key, LFFilter* filter, wchar_t* name, wchar_t* comments, LFItemDescriptor** created)
{
	assert(filter);

	if (created)
		*created = NULL;

	// Store finden
	char store[LFKeySize] = "";
	if (key)
		strcpy_s(store, LFKeySize, key);

	if (store[0]=='\0')
		if (GetMutex(Mutex_Stores))
		{
			strcpy_s(store, LFKeySize, DefaultStore);
			ReleaseMutex(Mutex_Stores);
		}
		else
			return LFMutexError;

	if (store[0]=='\0')
		return LFNoDefaultStore;

	// Store öffnen
	CIndex* idx1;
	CIndex* idx2;
	LFStoreDescriptor* slot;
	HANDLE StoreLock = NULL;
	unsigned int res = OpenStore(&store[0], true, idx1, idx2, &slot, &StoreLock);
	if (res==LFOk)
	{
		LFItemDescriptor* i = LFAllocItemDescriptor();
		SetAttribute(i, LFAttrFileName, name ? name : filter->Name);
		SetAttribute(i, LFAttrFileFormat, "filter");
		if (comments)
			SetAttribute(i, LFAttrComments, comments);

		wchar_t Path[2*MAX_PATH];
		res = PrepareImport(slot, i, Path, 2*MAX_PATH);
		if (res==LFOk)
			if (StoreFilter(Path, filter))
			{
				SetAttributesFromFile(i, Path, false);

				if (idx1)
					idx1->AddItem(i);
				if (idx2)
					idx2->AddItem(i);

				if (created)
				{
					*created = i;
					i = NULL;
				}
			}
			else
			{
				wchar_t* LastBackslash = wcsrchr(Path, L'\\');
				if (LastBackslash)
					*(LastBackslash+1) = L'\0';

				RemoveDir(Path);

				LFFreeItemDescriptor(i);
				res = LFIllegalPhysicalPath;
			}

		LFFreeItemDescriptor(i);

		if (idx1)
			delete idx1;
		if (idx2)
			delete idx2;
		ReleaseMutexForStore(StoreLock);
	}

	return res;
}
