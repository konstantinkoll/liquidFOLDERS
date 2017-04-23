
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

	// Sort for binary search
	m_pFileImportList->Sort();

	// Synchronize with index
	UINT Result;
	if ((Result=m_pIndexMain->Synchronize(pProgress))!=LFOk)
		goto Finish;

	if (m_pIndexAux)
		if ((Result=m_pIndexAux->Synchronize(pProgress))!=LFOk)
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

			wcscpy_s((LPWSTR)pItemDescriptor->StoreData, MAX_PATH, &(*m_pFileImportList)[a].Path[wcslen(p_StoreDescriptor->DatPath)]);

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

	// Update store data
	GetSystemTimeAsFileTime(&p_StoreDescriptor->SynchronizeTime);

	Result = SaveStoreSettings(p_StoreDescriptor);

Finish:
	LFFreeFileImportList(m_pFileImportList);
	m_pFileImportList = NULL;

	return Result;
}

UINT CStoreWindows::GetFileLocation(LFCoreAttributes* /*pCoreAttributes*/, LPCVOID pStoreData, LPWSTR pPath, SIZE_T cCount) const
{
	assert(pStoreData);
	assert(pPath);

	if (!LFIsStoreMounted(p_StoreDescriptor))
		return LFStoreNotMounted;

	wcscpy_s(pPath, cCount, L"\\\\?\\");
	wcscat_s(pPath, cCount, p_StoreDescriptor->DatPath);
	wcscat_s(pPath, cCount, (LPCWSTR)pStoreData);

	return LFOk;
}

UINT CStoreWindows::PrepareImport(LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount)
{
	assert(pItemDescriptor);
	assert(pPath);
	assert(cCount>=2*MAX_PATH);

	UINT Result;

	// StoreData
	WCHAR* pData = (LPWSTR)&pItemDescriptor->StoreData;
	if (*pData==L'\0')
	{
		WCHAR SanitizedFileName[MAX_PATH];
		SanitizeFileName(SanitizedFileName, MAX_PATH, pItemDescriptor->CoreAttributes.FileName);

		WCHAR Path[2*MAX_PATH];
		WCHAR NumberStr[16] = L"";
		UINT Number = 1;

		// Check if file exists; if yes append number
		do
		{
			wcscpy_s(pData, MAX_PATH, SanitizedFileName);
			wcscat_s(pData, MAX_PATH, NumberStr);

			if (pItemDescriptor->CoreAttributes.FileFormat[0])
			{
				WCHAR Buffer[LFExtSize];
				MultiByteToWideChar(CP_ACP, 0, pItemDescriptor->CoreAttributes.FileFormat, -1, Buffer, LFExtSize);

				wcscat_s(pData, MAX_PATH, L".");
				wcscat_s(pData, MAX_PATH, Buffer);
			}

			swprintf_s(NumberStr, 16, L" (%u)", ++Number);

			if ((Result=GetFileLocation(&pItemDescriptor->CoreAttributes, pData, Path, 2*MAX_PATH))!=LFOk)
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

		WCHAR* pChar = wcsrchr(Roll, L'\\');
		if (pChar)
		{
			*(pChar+1) = L'\0';

			if (wcscmp(Roll, p_StoreDescriptor->DatPath)!=0)
			{
				*pChar = L'\0';

				pChar = wcsrchr(Roll, L'\\');
				if (pChar)
					SetAttribute(pItemDescriptor, LFAttrRoll, pChar+1);
			}
		}
	}

	return LFOk;
}

UINT CStoreWindows::RenameFile(LFCoreAttributes* pCoreAttributes, LPVOID pStoreData, LFItemDescriptor* pItemDescriptor)
{
	assert(pCoreAttributes);
	assert(pStoreData);
	assert(pItemDescriptor);

	// Sanitize Name
	WCHAR tmpStr[256];
	wcscpy_s(tmpStr, 256, pItemDescriptor->CoreAttributes.FileName);
	SanitizeFileName(pItemDescriptor->CoreAttributes.FileName, 256, tmpStr);

	// Path
	LPWSTR pData = (LPWSTR)pItemDescriptor->StoreData;

	LPWSTR pStr = wcsrchr(pData, L'\\');
	if (!pStr)
		pStr = pData;

	LPCWSTR pExtension = wcsrchr(pStr, L'.');

	WCHAR Extension[MAX_PATH];
	wcscpy_s(Extension, MAX_PATH, pExtension ? pExtension : L"");

	SIZE_T cCount = MAX_PATH+pData-pStr-wcslen(Extension);
	wcscpy_s(pStr, cCount, pItemDescriptor->CoreAttributes.FileName);
	wcscat_s(pStr, cCount, Extension);

	// Commit
	return CStore::RenameFile(pCoreAttributes, pStoreData, pItemDescriptor);
}

UINT CStoreWindows::DeleteFile(LFCoreAttributes* pCoreAttributes, LPCVOID pStoreData)
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

BOOL CStoreWindows::SynchronizeFile(LFCoreAttributes* pCoreAttributes, LPCVOID pStoreData)
{
	assert(m_pFileImportList);

	WCHAR Path[2*MAX_PATH];
	GetFileLocation(pCoreAttributes, pStoreData, Path, 2*MAX_PATH);

	// Find in import list using binary search
	INT First = 0;
	INT Last = (INT)m_pFileImportList->m_ItemCount;

	while (First<=Last)
	{
		const INT Mid = (First+Last)/2;
		LFFileImportListItem* pItem = &(*m_pFileImportList)[Mid];

		const INT Result = _wcsicmp(&Path[4], pItem->Path);
		if (!Result)
		{
			if (!pItem->Processed)
			{
				// Update metadata
				assert(pItem->FindFileDataPresent);

				SetFromFindData(pCoreAttributes, &pItem->FindFileData);

				pItem->Processed = TRUE;
			}

			return TRUE;
		}

		if (Result<0)
		{
			Last = Mid-1;
		}
		else
		{
			First = Mid+1;
		}
	}

	return FALSE;
}
