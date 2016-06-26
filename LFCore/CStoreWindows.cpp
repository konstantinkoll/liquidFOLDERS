
#include "stdafx.h"
#include "CStoreWindows.h"
#include "FileSystem.h"
#include "ShellProperties.h"
#include "Stores.h"
#include <assert.h>


// CStoreWindows
//

CStoreWindows::CStoreWindows(LFStoreDescriptor* pStoreDescriptor, HMUTEX hMutexForStore)
	: CStore(pStoreDescriptor, hMutexForStore, sizeof(WCHAR)*MAX_PATH)
{
	m_pFileImportList = NULL;
}

UINT CStoreWindows::Synchronize(BOOL OnInitialize, LFProgress* pProgress)
{
	assert(m_pIndexMain);

	m_pFileImportList = LFAllocFileImportList();
	m_pFileImportList->AddPath(p_StoreDescriptor->DatPath);

	// Resolve
	m_pFileImportList->Resolve(TRUE, pProgress);

	// Synchronize with index
	UINT Result;
	if ((Result=m_pIndexMain->Synchronize(pProgress))!=LFOk)
		goto Finish;

	// Import new files
	Result = m_pFileImportList->m_LastError;

	for (UINT a=0; a<m_pFileImportList->m_ItemCount; a++)
	{
		if (!(*m_pFileImportList)[a].Processed)
		{
			// Progress
			if (pProgress)
			{
				wcscpy_s(pProgress->Object, 256, m_pFileImportList->GetFileName(a));

				if (UpdateProgress(pProgress))
				{
					Result = LFCancel;
					goto Finish;
				}
			}

			LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptor();
			SetNameExtFromFile(pItemDescriptor, (*m_pFileImportList)[a].Path);

			wcscpy_s((WCHAR*)pItemDescriptor->StoreData, MAX_PATH, &(*m_pFileImportList)[a].Path[wcslen(p_StoreDescriptor->DatPath)]);

			UINT Result;
			WCHAR Path[2*MAX_PATH];
			if ((Result=PrepareImport(pItemDescriptor, Path, 2*MAX_PATH))==LFOk)
				CommitImport(pItemDescriptor, TRUE, (*m_pFileImportList)[a].Path, OnInitialize);

			LFFreeItemDescriptor(pItemDescriptor);

			// Progress
			if (pProgress)
			{
				pProgress->MinorCurrent++;

				if (UpdateProgress(pProgress))
				{
					Result = LFCancel;
					goto Finish;
				}
			}
		}
	}

	GetSystemTimeAsFileTime(&p_StoreDescriptor->SynchronizeTime);

	Result = SaveStoreSettings(p_StoreDescriptor);

Finish:
	LFFreeFileImportList(m_pFileImportList);
	m_pFileImportList = NULL;

	return Result;
}

UINT CStoreWindows::GetFileLocation(LFCoreAttributes* /*pCoreAttributes*/, void* pStoreData, WCHAR* pPath, SIZE_T cCount) const
{
	assert(pStoreData);
	assert(pPath);

	if (!LFIsStoreMounted(p_StoreDescriptor))
		return LFStoreNotMounted;

	wcscpy_s(pPath, cCount, L"\\\\?\\");
	wcscat_s(pPath, cCount, p_StoreDescriptor->DatPath);
	wcscat_s(pPath, cCount, (WCHAR*)pStoreData);

	return LFOk;
}

UINT CStoreWindows::PrepareImport(LFItemDescriptor* pItemDescriptor, WCHAR* pPath, SIZE_T cCount)
{
	assert(pItemDescriptor);
	assert(pPath);
	assert(cCount>=2*MAX_PATH);

	UINT Result;

	// StoreData
	WCHAR* Ptr = (WCHAR*)&pItemDescriptor->StoreData;
	if (*Ptr==L'\0')
	{
		WCHAR SanitizedFileName[MAX_PATH];
		SanitizeFileName(SanitizedFileName, MAX_PATH, pItemDescriptor->CoreAttributes.FileName);

		WCHAR Path[2*MAX_PATH];
		WCHAR NumberStr[16] = L"";
		UINT Number = 1;

		// Check if file exists; if yes append number
		do
		{
			wcscpy_s(Ptr, MAX_PATH, SanitizedFileName);
			wcscat_s(Ptr, 2*MAX_PATH, NumberStr);

			if (pItemDescriptor->CoreAttributes.FileFormat[0])
			{
				WCHAR Buffer[LFExtSize];
				MultiByteToWideChar(CP_ACP, 0, pItemDescriptor->CoreAttributes.FileFormat, -1, Buffer, LFExtSize);

				wcscat_s(Ptr, MAX_PATH, L".");
				wcscat_s(Ptr, MAX_PATH, Buffer);
			}

			swprintf_s(NumberStr, 16, L" (%u)", ++Number);

			if ((Result=GetFileLocation(&pItemDescriptor->CoreAttributes, Ptr, Path, 2*MAX_PATH))!=LFOk)
				return Result;
		}
		while (_waccess(Path, 0)==0);
	}

	if ((Result=CStore::PrepareImport(pItemDescriptor, pPath, cCount))!=LFOk)
		return Result;

	// File ID
	CreateNewFileID(pItemDescriptor->CoreAttributes.FileID);

	// Location
	if ((Result=CStore::GetFileLocation(pItemDescriptor, pPath, cCount))!=LFOk)
		return Result;

	// Roll
	LFVariantData Value;
	LFGetAttributeVariantDataEx(pItemDescriptor, LFAttrRoll, Value);

	if (LFIsNullVariantData(Value))
	{
		WCHAR Roll[2*MAX_PATH];
		wcscpy_s(Roll, 2*MAX_PATH, &pPath[4]);

		WCHAR* Ptr = wcsrchr(Roll, L'\\');
		if (Ptr)
		{
			*(Ptr+1) = L'\0';

			if (wcscmp(Roll, p_StoreDescriptor->DatPath)!=0)
			{
				*Ptr = L'\0';

				Ptr = wcsrchr(Roll, L'\\');
				if (Ptr)
					SetAttribute(pItemDescriptor, LFAttrRoll, Ptr+1);
			}
		}
	}

	return LFOk;
}

UINT CStoreWindows::RenameFile(LFCoreAttributes* pCoreAttributes, void* pStoreData, LFItemDescriptor* pItemDescriptor)
{
	assert(pCoreAttributes);
	assert(pStoreData);
	assert(pItemDescriptor);

	// Sanitize Name
	WCHAR tmpStr[256];
	wcscpy_s(tmpStr, 256, pItemDescriptor->CoreAttributes.FileName);
	SanitizeFileName(pItemDescriptor->CoreAttributes.FileName, 256, tmpStr);

	// Path
	WCHAR* pPath = (WCHAR*)pItemDescriptor->StoreData;

	WCHAR* Ptr = wcsrchr(pPath, L'\\');
	if (!Ptr)
		Ptr = pPath;

	WCHAR Extension[MAX_PATH];
	WCHAR* pExtension = wcsrchr(Ptr, L'.');
	wcscpy_s(Extension, MAX_PATH, pExtension ? pExtension : L"");

	SIZE_T cCount = MAX_PATH+pPath-Ptr-wcslen(Extension);
	wcscpy_s(Ptr, cCount, pItemDescriptor->CoreAttributes.FileName);
	wcscat_s(Ptr, cCount, Extension);

	// Commit
	return CStore::RenameFile(pCoreAttributes, pStoreData, pItemDescriptor);
}

UINT CStoreWindows::DeleteFile(LFCoreAttributes* pCoreAttributes, void* pStoreData)
{
	assert(pCoreAttributes);
	assert(pStoreData);

	if (!LFIsStoreMounted(p_StoreDescriptor))
		return LFStoreNotMounted;

	WCHAR Path[2*MAX_PATH];
	GetFileLocation(pCoreAttributes, pStoreData, Path, 2*MAX_PATH);

	if (::DeleteFile(Path))
		return LFOk;

	DWORD Error = GetLastError();
	return (Error==ERROR_NO_MORE_FILES) || (Error==ERROR_FILE_NOT_FOUND) || (Error==ERROR_PATH_NOT_FOUND) ? LFOk : LFCannotDeleteFile;
}

BOOL CStoreWindows::SynchronizeFile(LFCoreAttributes* pCoreAttributes, void* pStoreData, LFProgress* pProgress)
{
	WCHAR Path[2*MAX_PATH];
	GetFileLocation(pCoreAttributes, pStoreData, Path, 2*MAX_PATH);

	// Find in import list
	if (m_pFileImportList)
		for (UINT a=0; a<m_pFileImportList->m_ItemCount; a++)
			if (!(*m_pFileImportList)[a].Processed)
				if (wcscmp(&Path[4], (*m_pFileImportList)[a].Path)==0)
				{
					(*m_pFileImportList)[a].Processed = TRUE;

					// Progress
					if (pProgress)
					{
						pProgress->MinorCurrent++;

						UpdateProgress(pProgress);
					}

					break;
				}

	// Find file body, update metadata
	WIN32_FIND_DATA FindFileData;
	if (FileExists(Path, &FindFileData))
	{
		// Update metadata
		SetFromFindData(pCoreAttributes, &FindFileData);

		return TRUE;
	}

	return FALSE;
}
