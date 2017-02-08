
#include "stdafx.h"
#include "CStore.h"
#include "FileSystem.h"
#include "ShellProperties.h"
#include "Stores.h"
#include <assert.h>


extern LFMessageIDs LFMessages;
extern CHAR KeyChars[38];


// CStore
//

#define ABORT(Result)       { if (pProgress) pProgress->ProgressState = (Result>LFCancel) ? LFProgressError : LFProgressCancelled; return Result; }
#define FASTEST_INDEX()     ((p_StoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexHybrid)

CStore::CStore(LFStoreDescriptor* pStoreDescriptor, HANDLE hMutexForStore, UINT AdditionalDataSize)
{
	assert(pStoreDescriptor);
	assert(hMutexForStore);

	p_StoreDescriptor = pStoreDescriptor;
	hMutex = hMutexForStore;
	m_AdditionalDataSize = AdditionalDataSize;
	m_pIndexMain = m_pIndexAux = NULL;
	m_WriteAccess = FALSE;
}

CStore::~CStore()
{
	Close();

	if (hMutex)
		ReleaseMutexForStore(hMutex);
}

UINT CStore::Open(BOOL WriteAccess)
{
	assert(m_pIndexMain==NULL);
	assert(m_pIndexAux==NULL);

	// Write access is only possible when the store is mounted and not write-protected
	if (WriteAccess)
	{
		if (!LFIsStoreMounted(p_StoreDescriptor))
			return LFStoreNotMounted;

		if ((p_StoreDescriptor->Flags & LFStoreFlagsWriteable)==0)
			return LFDriveWriteProtected;
	}

	// Open index(es)
	if (WriteAccess)
	{
		// Main index
		m_pIndexMain = new CIndex(this, TRUE, TRUE, m_AdditionalDataSize);

		// Aux index for hybrid indexing
		if ((p_StoreDescriptor->Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid)
			m_pIndexAux = new CIndex(this, FALSE, TRUE, m_AdditionalDataSize);

		m_WriteAccess = TRUE;
	}
	else
	{
		// Select aux store for hybrid indexing:
		// - Faster access (resides on local harddrive)
		// - Available even in unmounted state
		m_pIndexMain = new CIndex(this, FASTEST_INDEX(), FALSE, m_AdditionalDataSize);
	}

	return LFOk;
}

void CStore::Close()
{
	delete m_pIndexMain;
	delete m_pIndexAux;

	m_pIndexMain = m_pIndexAux = NULL;
}


// Non-Index operations
//

UINT CStore::Initialize(LFProgress* pProgress)
{
	assert(m_pIndexMain==NULL);
	assert(m_pIndexAux==NULL);

	UINT Result;

	// Create store directories
	if ((Result=CreateDirectories())!=LFOk)
		return Result;

	// Open store
	if ((Result=Open(TRUE))!=LFOk)
		return Result;

	// Initialize index
	if (m_pIndexMain)
		if (!m_pIndexMain->Initialize())
			return LFIndexCreateError;

	if (m_pIndexAux)
		if (!m_pIndexAux->Initialize())
			return LFIndexCreateError;

	// Synchronize
	if ((Result=Synchronize(TRUE, pProgress))!=LFOk)
		return Result;

	// Done
	return LFOk;
}

UINT CStore::PrepareDelete()
{
	Close();

	ReleaseMutexForStore(hMutex);
	hMutex = NULL;

	return LFOk;
}

UINT CStore::CommitDelete()
{
	return DeleteDirectories();
}

UINT CStore::MaintenanceAndStatistics(BOOL Scheduled, LFProgress* pProgress)
{
	Close();

	// Progress
	if (pProgress)
	{
		pProgress->MinorCount = IndexMaintenanceSteps+2;
		pProgress->MinorCurrent = 0;
		wcscpy_s(pProgress->Object, 256, p_StoreDescriptor->StoreName);

		if (UpdateProgress(pProgress))
			return LFCancel;
	}

	// Create store directories
	UINT Result = CreateDirectories();
	if (Result!=LFOk)
		ABORT(Result);

	// Progress
	if (pProgress)
	{
		pProgress->MinorCurrent++;
		pProgress->NoMinorCounter = TRUE;

		if (UpdateProgress(pProgress))
			return LFCancel;
	}

	// Open index and check (always open main index: MountVolume relies on copying the main index for hybrid indexes)
	BOOL Repaired = FALSE;

	CIndex* pIndex = new CIndex(this, LFIsStoreMounted(p_StoreDescriptor), m_AdditionalDataSize, TRUE);
	Result = pIndex->MaintenanceAndStatistics(Scheduled, &Repaired, pProgress);
	delete pIndex;

	if (Result!=LFOk)
	{
		if (Result==LFCancel)
			return LFCancel;

		ABORT(Result);
	}

	// Clone index for hybrid indexing
	if (((p_StoreDescriptor->Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid) && LFIsStoreMounted(p_StoreDescriptor))
	{
		if (!DirectoryWriteable(p_StoreDescriptor->IdxPathAux))
			ABORT(LFDriveWriteProtected);

		if ((Result=CopyDirectory(p_StoreDescriptor->IdxPathMain, p_StoreDescriptor->IdxPathAux))!=LFOk)
			ABORT(Result);
	}

	// Timestamp maintenance
	if (Scheduled || Repaired)
	{
		p_StoreDescriptor->Flags &= ~LFStoreFlagsError;

		if (Repaired)
			p_StoreDescriptor->IndexVersion = CURIDXVERSION;

		// Store aktualisieren
		Result = UpdateStoreInCache(p_StoreDescriptor, FALSE);
		if (Result!=LFOk)
			ABORT(Result);
	}

	// Progress
	if (pProgress)
	{
		pProgress->MinorCurrent++;

		if (UpdateProgress(pProgress))
			return LFCancel;
	}

	return LFOk;
}


// Index operations
//

UINT CStore::Synchronize(BOOL /*OnInitialize*/, LFProgress* pProgress)
{
	// Progress
	if (pProgress)
	{
		pProgress->Object[0] = L'\0';
		pProgress->MinorCount = pProgress->MinorCurrent = 1;

		if (UpdateProgress(pProgress))
			return LFCancel;
	}

	return LFOk;
}

UINT CStore::ImportFile(WCHAR* pPath, LFItemDescriptor* pItemDescriptor, BOOL Move, BOOL Metadata)
{
	assert(m_pIndexMain!=NULL);

	if (Metadata)
		SetNameExtFromFile(pItemDescriptor, pPath);

	UINT Result;
	WCHAR Path[2*MAX_PATH];
	if ((Result=PrepareImport(pItemDescriptor, Path, 2*MAX_PATH))==LFOk)
		CommitImport(pItemDescriptor, (Result=(Move ? MoveFile(pPath, Path) : CopyFile(pPath, Path, FALSE)) ? LFOk : LFCannotImportFile)==LFOk, Metadata ? Move ? Path : pPath : NULL);

	return Result;
}

BOOL CStore::UpdateMissingFlag(LFItemDescriptor* pItemDescriptor, BOOL Exists, BOOL RemoveNew)
{
	assert(m_pIndexMain);
	assert(pItemDescriptor);

	BOOL Result = m_pIndexMain->UpdateMissingFlag(pItemDescriptor, Exists, RemoveNew);

	if (m_pIndexAux)
		Result &= m_pIndexAux->UpdateMissingFlag(pItemDescriptor, Exists, RemoveNew);

	return Result;
}

UINT CStore::CommitImport(LFItemDescriptor* pItemDescriptor, BOOL Commit, WCHAR* pPath, BOOL OnInitialize)
{
	assert(pItemDescriptor);
	assert(m_pIndexMain);

	if (Commit)
	{
		if (!OnInitialize)
			pItemDescriptor->CoreAttributes.Flags |= LFFlagNew;

		// Time added
		FILETIME Time;
		GetSystemTimeAsFileTime(&Time);
		SetAttribute(pItemDescriptor, LFAttrAddTime, &Time);

		// Metadata
		if (pPath)
			SetAttributesFromFile(pItemDescriptor, pPath);

		// Commit
		UINT Result = m_pIndexMain->Add(pItemDescriptor);

		if ((Result==LFOk) && (m_pIndexAux))
			Result = m_pIndexAux->Add(pItemDescriptor);

		return Result;
	}
	else
	{
		// Revert
		UINT Result = DeleteFile(&pItemDescriptor->CoreAttributes, &pItemDescriptor->StoreData);
		if (Result==LFCannotDeleteFile)
			Result = LFCannotImportFile;

		return Result;
	}
}

void CStore::Query(LFFilter* pFilter, LFSearchResult* pSearchResult)
{
	assert(m_pIndexMain);
	assert(pFilter);
	assert(pSearchResult);

	if (p_StoreDescriptor->Source==LFTypeSourceNethood)
	{
		// Keep old copy of statistics
		UINT FileCount[LFLastQueryContext+1];
		memcpy_s(FileCount, sizeof(FileCount), p_StoreDescriptor->FileCount, sizeof(FileCount));

		INT64 FileSize[LFLastQueryContext+1];
		memcpy_s(FileSize, sizeof(FileSize), p_StoreDescriptor->FileSize, sizeof(FileSize));

		// Read-only operation, just needs main index
		m_pIndexMain->Query(pFilter, pSearchResult, TRUE);

		// Compare old and current statistics, send notify message if needed
		for (UINT a=0; a<=LFLastQueryContext; a++)
			if ((FileCount[a]!=p_StoreDescriptor->FileCount[a]) || (FileSize[a]!=p_StoreDescriptor->FileSize[a]))
			{
				SendLFNotifyMessage(LFMessages.StatisticsChanged);

				break;
			}
	}
	else
	{
		// Read-only operation, just needs main index
		m_pIndexMain->Query(pFilter, pSearchResult);
	}
}

void CStore::DoTransaction(LFTransactionList* pTransactionList, UINT TransactionType, LFProgress* pProgress, UINT_PTR Parameter, LFVariantData* pVariantData1, LFVariantData* pVariantData2, LFVariantData* pVariantData3)
{
	assert(m_pIndexMain);
	assert(pTransactionList);

	// Check if write access is allowed
	if (!m_WriteAccess && (TransactionType>LFTransactionTypeLastReadonly))
	{
		pTransactionList->SetError(p_StoreDescriptor->StoreID, LFIndexAccessError, pProgress);
		return;
	}

	// Transaction time
	FILETIME TransactionTime;
	GetSystemTimeAsFileTime(&TransactionTime);

	// Process
	switch (TransactionType)
	{
	case LFTransactionTypeAddToSearchResult:
		m_pIndexMain->AddToSearchResult(pTransactionList, (LFSearchResult*)Parameter);

		break;

	case LFTransactionTypeResolveLocations:
		m_pIndexMain->ResolveLocations(pTransactionList);

		break;

	case LFTransactionTypeSendTo:
		m_pIndexMain->SendTo(pTransactionList, (CHAR*)Parameter, pProgress);

		break;

	case LFTransactionTypeArchive:
		m_pIndexMain->UpdateItemState(pTransactionList, LFFlagArchive, &TransactionTime);

		if (m_pIndexAux)
			m_pIndexAux->UpdateItemState(pTransactionList, LFFlagArchive, &TransactionTime);

		break;

	case LFTransactionTypePutInTrash:
		m_pIndexMain->UpdateItemState(pTransactionList, LFFlagTrash, &TransactionTime);

		if (m_pIndexAux)
			m_pIndexAux->UpdateItemState(pTransactionList, LFFlagTrash, &TransactionTime);

		break;

	case LFTransactionTypeRestore:
		m_pIndexMain->UpdateItemState(pTransactionList, 0, &TransactionTime);

		if (m_pIndexAux)
			m_pIndexAux->UpdateItemState(pTransactionList, 0, &TransactionTime);

		break;

	case LFTransactionTypeUpdate:
		m_pIndexMain->Update(pTransactionList, pVariantData1, pVariantData2, pVariantData3);

		if (m_pIndexAux)
			m_pIndexAux->Update(pTransactionList, pVariantData1, pVariantData2, pVariantData3);

		break;

	case LFTransactionTypeDelete:
		m_pIndexMain->Delete(pTransactionList, pProgress);

		if (m_pIndexAux)
			m_pIndexAux->Delete(pTransactionList, pProgress);

		break;

	default:
		pTransactionList->SetError(p_StoreDescriptor->StoreID, LFIllegalItemType, pProgress);
	}
}


// Callbacks
//

UINT CStore::CreateDirectories()
{
	// Create data path
	if (wcslen(p_StoreDescriptor->DatPath)>3)
	{
		DWORD Result = CreateDirectory(p_StoreDescriptor->DatPath);

		if ((Result!=ERROR_SUCCESS) && (Result!=ERROR_ALREADY_EXISTS))
			return LFIllegalPhysicalPath;
	}

	// Create main index path
	if (p_StoreDescriptor->IdxPathMain[0]!=L'\0')
	{
		DWORD Result = CreateDirectory(p_StoreDescriptor->IdxPathMain);

		if ((Result!=ERROR_SUCCESS) && (Result!=ERROR_ALREADY_EXISTS))
			return LFIllegalPhysicalPath;

		// Hide directory
		if (LFGetSourceForVolume((CHAR)p_StoreDescriptor->IdxPathMain[0])!=LFTypeSourceInternal)
			HideFile(p_StoreDescriptor->IdxPathMain);
	}

	// Create aux index path
	if ((p_StoreDescriptor->Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid)
	{
		WCHAR tmpStr[MAX_PATH];
		GetAutoPath(p_StoreDescriptor, tmpStr);

		DWORD Result = CreateDirectory(tmpStr);
		if ((Result!=ERROR_SUCCESS) && (Result!=ERROR_ALREADY_EXISTS))
			return LFIllegalPhysicalPath;

		Result = CreateDirectory(p_StoreDescriptor->IdxPathAux);

		if ((Result!=ERROR_SUCCESS) && (Result!=ERROR_ALREADY_EXISTS))
			return LFIllegalPhysicalPath;

		// Hide directory
		if (LFGetSourceForVolume((CHAR)p_StoreDescriptor->IdxPathAux[0])!=LFTypeSourceInternal)
			HideFile(p_StoreDescriptor->IdxPathAux);
	}

	return LFOk;
}

UINT CStore::DeleteDirectories()
{
	// Delete index directories
	if (p_StoreDescriptor->IdxPathMain[0]!=L'\0')
		if (!DeleteDirectory(p_StoreDescriptor->IdxPathMain))
			return LFDriveNotReady;

	if (p_StoreDescriptor->IdxPathAux[0]!=L'\0')
		if (!DeleteDirectory(p_StoreDescriptor->IdxPathAux))
			return LFDriveNotReady;

	// Delete internal path, ignore error code
	WCHAR Path[MAX_PATH];
	GetAutoPath(p_StoreDescriptor, Path);
	DeleteDirectory(Path);

	return LFOk;
}

UINT CStore::GetFileLocation(LFCoreAttributes* pCoreAttributes, void* /*pStoreData*/, WCHAR* pPath, SIZE_T cCount) const
{
	assert(pCoreAttributes);
	assert(pPath);

	if (!LFIsStoreMounted(p_StoreDescriptor))
		return LFStoreNotMounted;

	WCHAR Buffer[MAX_PATH];
	SanitizeFileName(Buffer, MAX_PATH, pCoreAttributes->FileName);

	GetInternalFilePath(pCoreAttributes, pPath, cCount);
	wcscat_s(pPath, cCount, L"\\");
	wcsncat_s(pPath, cCount, Buffer, 127);

	if (pCoreAttributes->FileFormat[0])
	{
		WCHAR Buffer[LFExtSize];
		MultiByteToWideChar(CP_ACP, 0, pCoreAttributes->FileFormat, -1, Buffer, LFExtSize);

		wcscat_s(pPath, cCount, L".");
		wcscat_s(pPath, cCount, Buffer);
	}

	return LFOk;
}

UINT CStore::PrepareImport(LFItemDescriptor* pItemDescriptor, WCHAR* /*pPath*/, SIZE_T /*cCount*/)
{
	assert(pItemDescriptor);

	if (!LFIsStoreMounted(p_StoreDescriptor))
		return LFStoreNotMounted;

	if (!m_WriteAccess)
		return LFIndexAccessError;

	// StoreID
	strcpy_s(pItemDescriptor->StoreID, LFKeySize, p_StoreDescriptor->StoreID);

	// Randomize
	SYSTEMTIME st;
	GetSystemTime(&st);
	srand(st.wMilliseconds*rand());

	return LFOk;
}

UINT CStore::RenameFile(LFCoreAttributes* pCoreAttributes, void* pStoreData, LFItemDescriptor* pItemDescriptor)
{
	assert(pCoreAttributes);
	assert(pStoreData);
	assert(pItemDescriptor);

	if (!LFIsStoreMounted(p_StoreDescriptor))
		return LFStoreNotMounted;

	if (!m_WriteAccess)
		return LFIndexAccessError;

	WCHAR Path1[2*MAX_PATH];
	GetFileLocation(pCoreAttributes, pStoreData, Path1, 2*MAX_PATH);

	WCHAR Path2[2*MAX_PATH];
	GetFileLocation(pItemDescriptor, Path2, 2*MAX_PATH);

	return FileExists(Path1) ? MoveFile(Path1, Path2) ? LFOk : (GetLastError()==ERROR_WRITE_PROTECT) ? LFDriveWriteProtected : LFCannotRenameFile : LFNoFileBody;
}

UINT CStore::DeleteFile(LFCoreAttributes* pCoreAttributes, void* pStoreData)
{
	assert(pCoreAttributes);
	assert(pStoreData);

	if (!LFIsStoreMounted(p_StoreDescriptor))
		return LFStoreNotMounted;

	if (!m_WriteAccess)
		return LFIndexAccessError;

	WCHAR Path[2*MAX_PATH];
	GetFileLocation(pCoreAttributes, pStoreData, Path, 2*MAX_PATH);

	WCHAR* Ptr = wcsrchr(Path, L'\\');
	if (Ptr)
		*(Ptr+1) = L'\0';

	if (DeleteDirectory(Path))
		return LFOk;

	switch (GetLastError())
	{
	case ERROR_NO_MORE_FILES:
	case ERROR_FILE_NOT_FOUND:
	case ERROR_PATH_NOT_FOUND:
		return LFOk;

	case ERROR_WRITE_PROTECT:
		return LFDriveWriteProtected;

	default:
		return LFCannotDeleteFile;
	}
}

BOOL CStore::SynchronizeFile(LFCoreAttributes* /*pCoreAttributes*/, void* /*pStoreData*/)
{
	// Always keep file
	return TRUE;
}


// Aux functions
//

void CStore::GetInternalFilePath(LFCoreAttributes* pCoreAttributes, WCHAR* pPath, SIZE_T cCount) const
{
	assert(pCoreAttributes);
	assert(pPath);
	assert(p_StoreDescriptor->DatPath[0]!=L'\0');

	WCHAR Buffer1[3] = L" \\";
	Buffer1[0] = pCoreAttributes->FileID[0];

	WCHAR Buffer2[LFKeySize-1];
	MultiByteToWideChar(CP_ACP, 0, &pCoreAttributes->FileID[1], -1, Buffer2, LFKeySize-1);

	wcscpy_s(pPath, cCount, L"\\\\?\\");
	wcscat_s(pPath, cCount, p_StoreDescriptor->DatPath);
	wcscat_s(pPath, cCount, Buffer1);
	wcscat_s(pPath, cCount, Buffer2);
}

void CStore::CreateNewFileID(CHAR* pFileID) const
{
	assert(pFileID);
	assert(m_pIndexMain);

	pFileID[LFKeySize-1] = '\0';

	do
	{
		for (UINT a=0; a<LFKeySize-1; a++)
			pFileID[a] = RAND_CHAR();
	}
	while (m_pIndexMain->ExistingFileID(pFileID));
}
