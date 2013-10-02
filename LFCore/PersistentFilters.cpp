
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

	if (filter->Mode!=LFFilterModeSearch)
		return false;

	PersistentFilterHeader Header;
	ZeroMemory(&Header, sizeof(Header));
	strcpy_s(Header.ID, 9, "LFFilter");
	Header.Version = 1;
	Header.szHeader = sizeof(PersistentFilterHeader);
	Header.szBody = sizeof(PersistentFilterBody);
	Header.szCondition = sizeof(PersistentFilterCondition);

	PersistentFilterBody Body;
	ZeroMemory(&Body, sizeof(Body));
	Body.AllStores = (filter->StoreID[0]=='\0');
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
					if (!WriteFile(hFile, Condition, sizeof(LFFilterCondition), &Written, NULL))
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

LFFilter* LoadFilter(wchar_t* fn, char* key)
{
	assert(fn);

#define Abort1 { return NULL; }

	HANDLE hFile = CreateFile(fn, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		Abort1;

#define Abort2 { CloseHandle(hFile); Abort1; }

	PersistentFilterHeader Header;
	ZeroMemory(&Header, sizeof(Header));

	DWORD Read;
	if (!ReadFile(hFile, &Header, 14, &Read, NULL))
		Abort2;
	if (Header.ID[8]!='\0')
		Abort2;
	if (strcmp(Header.ID, "LFFilter")!=0)
		Abort2;
	if (!ReadFile(hFile, &Header.szBody, min(Header.szHeader, sizeof(Header))-14, &Read, NULL))
		Abort2;

	if (SetFilePointer(hFile, Header.szHeader, NULL, FILE_BEGIN)==INVALID_SET_FILE_POINTER)
		Abort2;

	PersistentFilterBody Body;
	ZeroMemory(&Body, sizeof(Body));

	if (!ReadFile(hFile, &Body, min(Header.szBody, sizeof(Body)), &Read, NULL))
		Abort2;

	LFFilter* f = LFAllocFilter();
	f->Mode = LFFilterModeSearch;
	f->Options.IsPersistent = true;
#define Abort3 { LFFreeFilter(f); Abort2; }

	strcpy_s(f->StoreID, LFKeySize, Body.AllStores ? "" : key);
	wcscpy_s(f->Searchterm, 256, Body.Searchterm);

	for (unsigned int a=Body.cConditions; a>0; a--)
	{
		if (SetFilePointer(hFile, Header.szHeader+Header.szBody+(a-1)*Header.szCondition, NULL, FILE_BEGIN)==INVALID_SET_FILE_POINTER)
			Abort3;

		PersistentFilterCondition Condition;
		ZeroMemory(&Condition, sizeof(Condition));

		if (!ReadFile(hFile, &Condition, min(Header.szCondition, sizeof(Condition)), &Read, NULL))
			Abort3;

		LFFilterCondition* c = new LFFilterCondition();
		memcpy_s(c, sizeof(LFFilterCondition), &Condition, sizeof(Condition));
		c->Next = f->ConditionList;
		f->ConditionList = c;
	}

	CloseHandle(hFile);
	return f;
}


LFCore_API unsigned int LFSaveFilter(char* key, LFFilter* filter, wchar_t* name, wchar_t* comments, LFItemDescriptor** created)
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
	unsigned int res = OpenStore(store, true, idx1, idx2, &slot, &StoreLock);
	if (res==LFOk)
	{
		LFItemDescriptor* i = LFAllocItemDescriptor();
		SetAttribute(i, LFAttrFileName, name ? name : filter->OriginalName);
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

LFCore_API LFFilter* LFLoadFilter(wchar_t* fn)
{
	if (!GetMutex(Mutex_Stores))
		return NULL;

	wchar_t Path[2*MAX_PATH];
	wcscpy_s(Path, 2*MAX_PATH, fn);
	wchar_t* LastBackslash = wcsrchr(Path, L'\\');
	if (LastBackslash)
		*(LastBackslash+1) = L'\0';

	LFStoreDescriptor* slot = FindStore(Path);

	LFFilter* f = LoadFilter(fn, slot ? slot->StoreID : "");
	if (f)
	{
		wchar_t Name[256];
		LastBackslash = wcsrchr(fn, L'\\');
		wcscpy_s(Name, 256, (!LastBackslash) ? fn : (*LastBackslash==L'\0') ? fn : LastBackslash+1);

		wchar_t* pExt = wcschr(Name, L'.');
		if (pExt)
			*pExt = L'\0';

		wcscpy_s(f->OriginalName, 256, Name);
	}

	ReleaseMutex(Mutex_Stores);

	return f;
}

LFCore_API LFFilter* LFLoadFilter(LFItemDescriptor* i)
{
	wchar_t Path[2*MAX_PATH];
	if (LFGetFileLocation(i, Path, 2*MAX_PATH, true, true, true)!=LFOk)
		return NULL;

	LFFilter* f = LoadFilter(Path, i->StoreID);
	if (f)
		wcscpy_s(f->OriginalName, 256, i->CoreAttributes.FileName);

	return f;
}
