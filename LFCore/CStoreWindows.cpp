#include "stdafx.h"
#include "CStoreWindows.h"
#include "FileProperties.h"
#include "FileSystem.h"
#include "Progress.h"
#include "Stores.h"


// CStoreWindows
//

CStoreWindows::CStoreWindows(LFStoreDescriptor* pStoreDescriptor, HMUTEX hMutexForStore)
	: CStore(pStoreDescriptor, hMutexForStore, sizeof(WCHAR)*MAX_PATH)
{
	m_pFileImportList = NULL;
	m_szDatPath = wcslen(p_StoreDescriptor->DatPath);
}


// Non-index operations
//


UINT CStoreWindows::GetFilePath(const HORCRUXFILE& File, LPWSTR pPath, SIZE_T cCount) const
{
	assert((LPCVOID)File);
	assert(pPath);
	assert(cCount>=2*MAX_PATH);

	if (!IsStoreMounted(p_StoreDescriptor))
		return LFStoreNotMounted;

	wcscpy_s(pPath, cCount, L"\\\\?\\");
	wcscat_s(pPath, cCount, p_StoreDescriptor->DatPath);
	wcscat_s(pPath, cCount, (LPCWSTR)(LPCVOID)File);

	return LFOk;
}


// Index operations
//

UINT CStoreWindows::Synchronize(LFProgress* pProgress, BOOL OnInitialize)
{
	assert(m_pIndexMain);

	m_pFileImportList = LFAllocFileImportList();
	m_pFileImportList->AddPath(p_StoreDescriptor->DatPath);

	// Resolve
	m_pFileImportList->Resolve(TRUE, pProgress);

	// Synchronize with index
	UINT Result;
	if ((Result=m_pIndexMain->SynchronizeMatch())!=LFOk)
		goto Finish;

	if ((Result=m_pIndexMain->SynchronizeCommit(pProgress))!=LFOk)
		goto Finish;

	if (m_pIndexAux && ((Result=m_pIndexAux->SynchronizeCommit(pProgress))!=LFOk))
		goto Finish;

	// Import new files
	Result = m_pFileImportList->m_LastError;

	for (UINT a=0; a<m_pFileImportList->m_ItemCount; a++)
	{
		const LFFileImportItem* pItem = &(*m_pFileImportList)[a];

		if (!pItem->Processed)
		{
			// Progress
			if (SetProgressObject(pProgress, m_pFileImportList->GetFileName(a)))
			{
				Result = LFCancel;
				goto Finish;
			}

			// Set store data from path
			assert(m_szDatPath);
			LPCWSTR pStr = &pItem->Path[m_szDatPath];

			LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptor(NULL, pStr, (wcslen(pStr)+1)*sizeof(WCHAR));

			WCHAR Path[2*MAX_PATH];
			if ((Result=CStore::PrepareImport(pItem->Path, pItemDescriptor, Path, 2*MAX_PATH))==LFOk)
				CommitImport(pItemDescriptor, TRUE, pItem->Path, OnInitialize);

			LFFreeItemDescriptor(pItemDescriptor);

			// Progress
			if (ProgressMinorNext(pProgress))
			{
				Result = LFCancel;
				goto Finish;
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


// Callbacks
//

UINT CStoreWindows::PrepareImport(LPCWSTR pFilename, LPCSTR pExtension, LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount)
{
	assert(pFilename);
	assert(pExtension);
	assert(pItemDescriptor);
	assert(pPath);
	assert(cCount>=2*MAX_PATH);

	UINT Result;

	// Add store data (if not present from synchronization)
	LPWSTR pData = (LPWSTR)&pItemDescriptor->StoreData;
	if (*pData==L'\0')
	{
		WCHAR SanitizedFileName[MAX_PATH];
		SanitizeFileName(SanitizedFileName, MAX_PATH, pFilename);

		WCHAR Path[2*MAX_PATH];
		UINT Number = 1;

		WCHAR NumberStr[16];
		NumberStr[0] = L'\0';

		// Check if file exists; if yes append number
		do
		{
			wcscpy_s(pData, MAX_PATH, SanitizedFileName);
			wcscat_s(pData, MAX_PATH, NumberStr);

			if (*pExtension)
			{
				WCHAR Buffer[LFExtSize];
				MultiByteToWideChar(CP_ACP, 0, pExtension, -1, Buffer, LFExtSize);

				wcscat_s(pData, MAX_PATH, L".");
				wcscat_s(pData, MAX_PATH, Buffer);
			}

			swprintf_s(NumberStr, 16, L" (%u)", ++Number);

			if ((Result=GetFilePath(pItemDescriptor, Path, 2*MAX_PATH))!=LFOk)
				return Result;
		}
		while (_waccess(Path, 0)==0);
	}

	// Prepare import
	if ((Result=CStore::PrepareImport(pFilename, pExtension, pItemDescriptor, pPath, cCount))!=LFOk)
		return Result;

	// File ID
	CreateNewFileID(pItemDescriptor->CoreAttributes.FileID);

	// Location
	return GetFilePath(pItemDescriptor, pPath, cCount);
}

UINT CStoreWindows::RenameFile(const HORCRUXFILE& File, LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	// Sanitize name
	WCHAR tmpStr[256];
	wcscpy_s(tmpStr, 256, pItemDescriptor->CoreAttributes.FileName);
	SanitizeFileName(pItemDescriptor->CoreAttributes.FileName, 256, tmpStr);

	// Change path in store data of item descriptor
	LPCWSTR pData = (LPCWSTR)pItemDescriptor->StoreData;
	LPWSTR pName = (LPWSTR)ExtractFileName(pData);
	LPCWSTR pExtension = wcsrchr(pName, L'.');

	WCHAR Extension[MAX_PATH];
	wcscpy_s(Extension, MAX_PATH, pExtension ? pExtension : L"");

	const SIZE_T cCount = MAX_PATH+pData-pName-wcslen(Extension);
	wcscpy_s(pName, cCount, pItemDescriptor->CoreAttributes.FileName);
	wcscat_s(pName, cCount, Extension);

	// Commit
	return CStore::RenameFile(File, pItemDescriptor);
}

UINT CStoreWindows::DeleteFile(const HORCRUXFILE& File)
{
	UINT Result;
	WCHAR Path[2*MAX_PATH];
	if ((Result=GetFilePath(File, Path, 2*MAX_PATH))!=LFOk)
		return Result;

	return ::DeleteFile(Path) ? LFOk : CStore::DeleteFile();
}

void CStoreWindows::SetAttributesFromStore(LFItemDescriptor* pItemDescriptor)
{
	// Extract media collection from path
	if (LFIsNullAttribute(pItemDescriptor, LFAttrMediaCollection))
	{
		// Get file path
		WCHAR MediaCollection[MAX_PATH];
		wcscpy_s(MediaCollection, MAX_PATH, (LPCWSTR)pItemDescriptor->StoreData);

		// Isolate lowest subfolder
		WCHAR* pChar = wcsrchr(MediaCollection, L'\\');
		if (pChar)
		{
			*pChar = L'\0';

			// If there is another path component left, set it as media collection
			SetAttribute(pItemDescriptor, LFAttrMediaCollection, (pChar=wcsrchr(MediaCollection, L'\\'))!=NULL ? pChar+1 : MediaCollection);
		}
	}

	// Inherited roll, artist and title
	CStore::SetAttributesFromStore(pItemDescriptor);
}

LFFileImportItem* CStoreWindows::FindSimilarFile(const HORCRUXFILE& File)
{
	LPCCOREATTRIBUTES pCoreAttributes = File;
	LPCWSTR pName = ExtractFileName(File);

	for (UINT a=0; a<m_pFileImportList->m_ItemCount; a++)
	{
		LFFileImportItem* pItem = &(*m_pFileImportList)[a];

		if ((pItem->Processed!=PROCESSED_FINISHED) &&
			((pItem->Flags & (FII_MATCHED | FII_FINDDATAVALID))==FII_FINDDATAVALID) &&
			(pCoreAttributes->FileTime==pItem->FindData.ftLastWriteTime) &&
			(pCoreAttributes->CreationTime==pItem->FindData.ftCreationTime) &&
			(pCoreAttributes->FileSize==((((INT64)pItem->FindData.nFileSizeHigh) << 32) | pItem->FindData.nFileSizeLow)) &&
			(_wcsicmp(pName, ExtractFileName(pItem->Path))==0))
			return pItem;
	}

	return NULL;
}

void CStoreWindows::SynchronizeMatch(const HORCRUXFILE& File)
{
	assert(m_pFileImportList);

	WCHAR Path[2*MAX_PATH];
	if (GetFilePath(File, Path, 2*MAX_PATH)==LFOk)
	{
		LFFileImportItem* pItem = m_pFileImportList->FindPath(Path);

		// Set "matched" flag
		if (pItem && (pItem->Processed!=PROCESSED_FINISHED))
			pItem->Flags |= FII_MATCHED;
	}
}

BOOL CStoreWindows::SynchronizeCommit(const HORCRUXFILE& File)
{
	assert(m_pFileImportList);
	assert(m_szDatPath);

	WCHAR Path[2*MAX_PATH];
	if (GetFilePath(File, Path, 2*MAX_PATH)!=LFOk)
		return TRUE;

	// Find path
	LFFileImportItem* pItem = m_pFileImportList->FindPath(Path);

	// Find similar file
	if (!pItem && ((pItem=FindSimilarFile(File))!=NULL))
		wcscpy_s(File, MAX_PATH, &pItem->Path[m_szDatPath]);

	// Update metadata
	if (pItem && (pItem->Processed!=PROCESSED_FINISHED))
	{
		assert(pItem->Flags & FII_FINDDATAVALID);
		SetAttributesFromFindData(File, pItem->FindData, TRUE);

		assert(pItem->Processed+1<PROCESSED_FINISHED);
		pItem->Processed++;
	}

	return pItem!=NULL;
}
