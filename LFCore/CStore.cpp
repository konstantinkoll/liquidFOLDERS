
#include "stdafx.h"
#include "CStore.h"
#include "FileSystem.h"
#include "ShellProperties.h"
#include "Stores.h"
#include <assert.h>


extern CHAR KeyChars[38];


// CStore
//

#define ABORT(Result)       { if (pProgress) pProgress->ProgressState = LFProgressError; return Result; }
#define FASTEST_INDEX()     ((p_StoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexHybrid)

CStore::CStore(LFStoreDescriptor* pStoreDescriptor, HANDLE hMutexForStore, UINT StoreDataSize)
{
	assert(pStoreDescriptor);
	assert(hMutexForStore);

	p_StoreDescriptor = pStoreDescriptor;
	hMutex = hMutexForStore;
	m_AdditionalDataSize = StoreDataSize;
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

	// Write access is only possible when the store is mounted
	if (WriteAccess && !LFIsStoreMounted(p_StoreDescriptor))
		return LFStoreNotMounted;

	// Open index(es)
	if (WriteAccess)
	{
		// Main index
		m_pIndexMain = new CIndex(this, TRUE, m_AdditionalDataSize);

		// Aux index for hybrid indexing
		if ((p_StoreDescriptor->Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid)
			m_pIndexAux = new CIndex(this, FALSE, m_AdditionalDataSize);

		m_WriteAccess = TRUE;
	}
	else
	{
		// Select aux store for hybrid indexing:
		// - Faster access (resides on local harddrive)
		// - Available even in unmounted state
		m_pIndexMain = new CIndex(this, FASTEST_INDEX(), m_AdditionalDataSize);
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

	// Open store (also creates index)
	if ((Result=Open(TRUE))!=LFOk)
		return Result;

	// Synchronize
	if ((Result=Synchronize(pProgress))!=LFOk)
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

	// Open index and check
	BOOL Repaired = FALSE;

	CIndex* pIndex = new CIndex(this, FASTEST_INDEX(), m_AdditionalDataSize);
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
		if (!DirectoryWriteable(p_StoreDescriptor->IdxPathMain))
			ABORT(LFDriveWriteProtected);

		Result = CopyDirectory(p_StoreDescriptor->IdxPathAux, p_StoreDescriptor->IdxPathMain);
		if (Result!=LFOk)
			ABORT(Result);
	}

	// Timestamp maintenance
	if (Scheduled || Repaired)
	{
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
	
void CStore::ScheduledMaintenance(LFMaintenanceList* pMaintenanceList, LFProgress* pProgress)
{
	pMaintenanceList->AddItem(p_StoreDescriptor->StoreName, p_StoreDescriptor->Comments, p_StoreDescriptor->StoreID, MaintenanceAndStatistics(TRUE, pProgress), LFGetStoreIcon(p_StoreDescriptor));
}

UINT CStore::GetFileLocation(LFItemDescriptor* pItemDescriptor, WCHAR* pPath, SIZE_T cCount)
{
	assert(pItemDescriptor);
	assert(pPath);

	return GetFileLocation(&pItemDescriptor->CoreAttributes, &pItemDescriptor->StoreData, pPath, cCount);
}


// Index operations
//

UINT CStore::Synchronize(LFProgress* /*pProgress*/)
{
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
		CommitImport(pItemDescriptor, (Result=(Move ? MoveFile(pPath, Path) : CopyFile(pPath, Path, FALSE)) ? LFOk : LFCannotImportFile)==LFOk, Metadata ? pPath : NULL);

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

UINT CStore::PrepareImport(LFItemDescriptor* pItemDescriptor, WCHAR* pPath, SIZE_T cCount)
{
	assert(pItemDescriptor);
	assert(pPath);
	assert(cCount>=2*MAX_PATH);

	if (!LFIsStoreMounted(p_StoreDescriptor))
		return LFStoreNotMounted;

	if (!m_WriteAccess)
		return LFIndexAccessError;

	// Randomize
	SYSTEMTIME st;
	GetSystemTime(&st);
	srand(st.wMilliseconds*rand());

	// StoreID
	strcpy_s(pItemDescriptor->StoreID, LFKeySize, p_StoreDescriptor->StoreID);

	// FileID
	pItemDescriptor->CoreAttributes.FileID[0] = RAND_CHAR();
	ZeroMemory(&pItemDescriptor->CoreAttributes.FileID[1], LFKeySize-1);

	// 1st directory level
	GetInternalFilePath(&pItemDescriptor->CoreAttributes, pPath, cCount);

	DWORD Result = CreateDirectory(pPath);
	if ((Result!=ERROR_SUCCESS) && (Result!=ERROR_ALREADY_EXISTS))
		return LFIllegalPhysicalPath;

	// 2nd directory level
	do
	{
		for (UINT a=1; a<LFKeySize-1; a++)
			pItemDescriptor->CoreAttributes.FileID[a] = RAND_CHAR();

		GetInternalFilePath(&pItemDescriptor->CoreAttributes, pPath, cCount);
	}
	while (FileExists(pPath));

	Result = CreateDirectory(pPath);
	if ((Result!=ERROR_SUCCESS) && (Result!=ERROR_ALREADY_EXISTS))
		return LFIllegalPhysicalPath;

	return GetFileLocation(pItemDescriptor, pPath, cCount);
}

UINT CStore::CommitImport(LFItemDescriptor* pItemDescriptor, BOOL Commit, WCHAR* pPath)
{
	assert(pItemDescriptor);
	assert(m_pIndexMain);

	if (Commit)
	{
		pItemDescriptor->CoreAttributes.Flags |= LFFlagNew;

		// Time added
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);
		SetAttribute(pItemDescriptor, LFAttrAddTime, &ft);

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
		return DeleteFile(&pItemDescriptor->CoreAttributes, &pItemDescriptor->StoreData);
	}
}

void CStore::Query(LFFilter* pFilter, LFSearchResult* pSearchResult)
{
	assert(m_pIndexMain);
	assert(pFilter);
	assert(pSearchResult);

	// Read-only operation, just needs main index
	m_pIndexMain->Query(pFilter, pSearchResult);
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
	if (p_StoreDescriptor->DatPath[0]!=L'\0')
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

UINT CStore::GetFileLocation(LFCoreAttributes* pCoreAttributes, void* /*pStoreData*/, WCHAR* pPath, SIZE_T cCount)
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

UINT CStore::RenameFile(LFCoreAttributes* pCoreAttributes, void* pStoreData, WCHAR* pNewName)
{
	if (!LFIsStoreMounted(p_StoreDescriptor))
		return LFStoreNotMounted;

	if (!m_WriteAccess)
		return LFIndexAccessError;

	WCHAR Path1[2*MAX_PATH];
	GetFileLocation(pCoreAttributes, pStoreData, Path1, 2*MAX_PATH);

	WCHAR OldName[256];
	wcscpy_s(OldName, 256, pCoreAttributes->FileName);
	wcscpy_s(pCoreAttributes->FileName, 256, pNewName);

	WCHAR Path2[2*MAX_PATH];
	GetFileLocation(pCoreAttributes, pStoreData, Path2, 2*MAX_PATH);

	UINT Result = FileExists(Path1) ? MoveFile(Path1, Path2) ? LFOk : LFCannotRenameFile : LFNoFileBody;

	if (Result!=LFOk)
		wcscpy_s(pCoreAttributes->FileName, 256, OldName);

	return Result;
}

UINT CStore::DeleteFile(LFCoreAttributes* pCoreAttributes, void* pStoreData)
{
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

	DWORD Error = GetLastError();
	return (Error==ERROR_NO_MORE_FILES) || (Error==ERROR_FILE_NOT_FOUND) || (Error==ERROR_PATH_NOT_FOUND) ? LFOk : LFCannotDeleteFile;
}


// Aux functions
//

void CStore::GetInternalFilePath(LFCoreAttributes* pCoreAttributes, WCHAR* pPath, SIZE_T cCount)
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
