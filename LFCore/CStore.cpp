
#include "stdafx.h"
#include "CStore.h"
#include "FileProperties.h"
#include "FileSystem.h"
#include "Progress.h"
#include "Stores.h"


extern LFMessageIDs LFMessages;
extern CHAR KeyChars[38];


// CStore
//

#define ABORT(Result)       { if (pProgress) pProgress->ProgressState = (Result>LFCancel) ? LFProgressError : LFProgressCancelled; return Result; }

CStore::CStore(LFStoreDescriptor* pStoreDescriptor, HANDLE hMutexForStore, UINT AdditionalDataSize)
{
	assert(pStoreDescriptor);
	assert(hMutexForStore);
	assert(AdditionalDataSize<=LFMaxStoreDataSize);

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
		if ((p_StoreDescriptor->Flags & LFFlagsMounted)==0)
			return LFStoreNotMounted;

		if ((p_StoreDescriptor->Flags & LFFlagsWriteable)==0)
			return LFVolumeWriteProtected;
	}

	// Open index(es)
	if (WriteAccess)
	{
		// Main index
		m_pIndexMain = new CIndex(this, TRUE, TRUE, m_AdditionalDataSize);

		// Aux index for hybrid indexing
		if (p_StoreDescriptor->IndexMode==LFStoreIndexModeHybrid)
			m_pIndexAux = new CIndex(this, FALSE, TRUE, m_AdditionalDataSize);

		m_WriteAccess = TRUE;
	}
	else
	{
		// Select aux store for hybrid indexing:
		// - Faster access (resides on local harddrive)
		// - Available even in unmounted state
		m_pIndexMain = new CIndex(this, p_StoreDescriptor->IndexMode!=LFStoreIndexModeHybrid, FALSE, m_AdditionalDataSize);
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
	if (m_pIndexMain && !m_pIndexMain->Initialize())
		return LFIndexCreateError;

	if (m_pIndexAux && !m_pIndexAux->Initialize())
		return LFIndexCreateError;

	// Synchronize
	if ((Result=Synchronize(pProgress, TRUE))!=LFOk)
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

UINT CStore::MaintenanceAndStatistics(LFProgress* pProgress, BOOL Scheduled)
{
	Close();

	// Progress
	if (ProgressMinorStart(pProgress, IndexMaintenanceSteps+2, p_StoreDescriptor->StoreName, TRUE))
		return LFCancel;

	// Create store directories
	UINT Result = CreateDirectories();
	if (Result>LFVolumeWriteProtected)
	{
		// Set LFFlagsRenameDeleteAllowed to allow a broken store to be deleted
		p_StoreDescriptor->Flags |= LFFlagsRenameDeleteAllowed;

		ABORT(Result);
	}

	// Progress
	if (ProgressMinorNext(pProgress))
		return LFCancel;

	// Open index and check (always open main index: MountVolume relies on copying the main index for hybrid indexes)
	BOOL Repaired = FALSE;

	CIndex* pIndex = new CIndex(this, IsStoreMounted(p_StoreDescriptor), TRUE, m_AdditionalDataSize);
	Result = pIndex->MaintenanceAndStatistics(Scheduled, Repaired, pProgress);
	delete pIndex;

	if (Result!=LFOk)
	{
		if (Result==LFCancel)
			return LFCancel;

		ABORT(Result);
	}

	// Clone index for hybrid indexing
	if ((p_StoreDescriptor->IndexMode==LFStoreIndexModeHybrid) && IsStoreMounted(p_StoreDescriptor))
	{
		if (!(p_StoreDescriptor->Flags & LFFlagsWriteable))
			ABORT(LFVolumeWriteProtected);

		if ((Result=CopyDirectory(p_StoreDescriptor->IdxPathMain, p_StoreDescriptor->IdxPathAux))!=LFOk)
			ABORT(Result);
	}

	// Timestamp maintenance
	if (Scheduled || Repaired)
	{
		p_StoreDescriptor->State &= ~LFStoreStateError;

		if (Repaired)
			p_StoreDescriptor->IndexVersion = CURIDXVERSION;

		// Store aktualisieren
		Result = UpdateStoreInCache(p_StoreDescriptor, FALSE);
		if (Result!=LFOk)
			ABORT(Result);
	}

	// Progress
	return ProgressMinorNext(pProgress) ? LFCancel : LFOk;
}


// Index operations
//

UINT CStore::Synchronize(LFProgress* pProgress, BOOL /*OnInitialize*/)
{
	return ProgressMinorStart(pProgress, 1) ? LFCancel : LFOk;
}

UINT CStore::ImportFile(LPCWSTR pPath, LFItemDescriptor* pItemDescriptor, BOOL Move, BOOL RetrieveMetadata)
{
	assert(m_pIndexMain!=NULL);

	// IMPORTANT! Reset store data in item descriptor!
	// May be uninitialized or left from "Send to" operation!
	ResetStoreData(pItemDescriptor);

	// Import
	UINT Result;
	WCHAR Path[2*MAX_PATH];
	if ((Result=PrepareImport(pPath, pItemDescriptor, Path, 2*MAX_PATH))==LFOk)
		CommitImport(pItemDescriptor, (Result=(Move ? MoveFile(pPath, Path) : CopyFile(pPath, Path, FALSE)) ? LFOk : LFCannotImportFile)==LFOk, RetrieveMetadata ? Move ? Path : pPath : NULL);

	return Result;
}

UINT CStore::CommitImport(LFItemDescriptor* pItemDescriptor, BOOL Commit, LPCWSTR pPath, BOOL OnInitialize)
{
	assert(pItemDescriptor);
	assert(m_pIndexMain);

	UINT Result;

	if (Commit)
	{
		// Flags (must be first)
		if (!OnInitialize)
		{
			// Remove "Compressed" state
			pItemDescriptor->CoreAttributes.State &= ~LFItemStateCompressed;

			// Add "New" state
			pItemDescriptor->CoreAttributes.State |= LFItemStateNew;
		}

		// Metadata
		if (pPath)
		{
			SetFileContext(pItemDescriptor->CoreAttributes);
			SetAttributesFromFindData(pItemDescriptor->CoreAttributes, pPath);
			SetAttributesFromShell(pItemDescriptor, pPath);
			SetAttributesFromStore(pItemDescriptor);
		}

		// Time added
		FILETIME Time;
		GetSystemTimeAsFileTime(&Time);
		SetAttribute(pItemDescriptor, LFAttrAddTime, &Time);

		// Commit
		if (((Result=m_pIndexMain->Add(pItemDescriptor))==LFOk) && m_pIndexAux)
			Result = m_pIndexAux->Add(pItemDescriptor);
	}
	else
	{
		// Revert
		if ((Result=DeleteFile(pItemDescriptor))==LFCannotDeleteFile)
			Result = LFCannotImportFile;
	}

	return Result;
}

void CStore::Query(LFFilter* pFilter, LFSearchResult* pSearchResult)
{
	assert(m_pIndexMain);
	assert(pFilter);
	assert(pSearchResult);

	if (p_StoreDescriptor->Source==LFSourceNethood)
	{
		// Keep old copy of statistics
		LFStatistics Statistics = p_StoreDescriptor->Statistics;

		// Read-only operation, just needs main index
		m_pIndexMain->Query(pFilter, pSearchResult, TRUE);

		// Compare old and current statistics, send notify message if needed
		if (memcmp(&Statistics, &p_StoreDescriptor->Statistics, sizeof(LFStatistics)))
			SendLFNotifyMessage(LFMessages.StatisticsChanged);
	}
	else
	{
		// Read-only operation, just needs main index
		m_pIndexMain->Query(pFilter, pSearchResult);
	}
}

void CStore::DoTransaction(LFTransactionList* pTransactionList, UINT TransactionType, LFProgress* pProgress, UINT_PTR Parameter, const LFVariantData* pVariantData1, const LFVariantData* pVariantData2, const LFVariantData* pVariantData3)
{
	assert(m_pIndexMain);
	assert(pTransactionList);

	// Check if write access is allowed
	if (!m_WriteAccess && (TransactionType>LFTransactionLastReadonly))
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
	case LFTransactionAddToSearchResult:
		m_pIndexMain->AddToSearchResult(pTransactionList, (LFSearchResult*)Parameter);

		break;

	case LFTransactionResolveLocations:
		m_pIndexMain->ResolveLocations(pTransactionList);

		break;

	case LFTransactionSendTo:
		m_pIndexMain->SendTo(pTransactionList, *((LPCSTOREID)Parameter), pProgress);

		break;

	case LFTransactionArchive:
		m_pIndexMain->SetItemState(pTransactionList, TransactionTime, LFItemStateArchive);

		if (m_pIndexAux)
			m_pIndexAux->SetItemState(pTransactionList, TransactionTime, LFItemStateArchive);

		break;

	case LFTransactionPutInTrash:
		m_pIndexMain->SetItemState(pTransactionList, TransactionTime, LFItemStateTrash);

		if (m_pIndexAux)
			m_pIndexAux->SetItemState(pTransactionList, TransactionTime, LFItemStateTrash);

		break;

	case LFTransactionRecover:
		m_pIndexMain->SetItemState(pTransactionList, TransactionTime, 0);

		if (m_pIndexAux)
			m_pIndexAux->SetItemState(pTransactionList, TransactionTime, 0);

		break;

	case LFTransactionUpdate:
	case LFTransactionUpdateTask:
		m_pIndexMain->Update(pTransactionList, pVariantData1, pVariantData2, pVariantData3, TransactionType==LFTransactionUpdateTask);

		if (m_pIndexAux)
			m_pIndexAux->Update(pTransactionList, pVariantData1, pVariantData2, pVariantData3, TransactionType==LFTransactionUpdateTask);

		break;

	case LFTransactionSetUserContext:
		m_pIndexMain->SetUserContext(pTransactionList, (BYTE)Parameter);

		if (m_pIndexAux)
			m_pIndexAux->SetUserContext(pTransactionList, (BYTE)Parameter);

		break;

	case LFTransactionCompress:
		m_pIndexMain->Compress(pTransactionList, pProgress);

		if (m_pIndexAux)
			m_pIndexAux->Compress(pTransactionList, pProgress);

		break;

	case LFTransactionDelete:
		m_pIndexMain->Delete(pTransactionList, pProgress);

		if (m_pIndexAux)
			m_pIndexAux->Delete(pTransactionList, pProgress);

		break;

	default:
		pTransactionList->SetError(p_StoreDescriptor->StoreID, LFIllegalItemType, pProgress);
	}
}
	
UINT CStore::UpdateItemState(LFItemDescriptor* pItemDescriptor, const WIN32_FIND_DATA& FindData, BOOL Exists, BOOL RemoveNew)
{
	assert(m_pIndexMain);
	assert(pItemDescriptor);

	UINT Result = m_pIndexMain->UpdateItemState(pItemDescriptor, FindData, Exists, RemoveNew);

	if (m_pIndexAux)
		m_pIndexAux->UpdateItemState(pItemDescriptor, FindData, Exists, RemoveNew);

	return Result;
}


// Callbacks
//

UINT CStore::CreateDirectories()
{
	// Volume write protected?
	if (!(p_StoreDescriptor->Flags & LFFlagsWriteable))
		return LFVolumeWriteProtected;

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
		if (p_StoreDescriptor->HideMainIndex)
			HideFile(p_StoreDescriptor->IdxPathMain);
	}

	// Create aux index path
	if (p_StoreDescriptor->IndexMode==LFStoreIndexModeHybrid)
	{
		WCHAR tmpStr[MAX_PATH];
		GetAutoPath(*p_StoreDescriptor, tmpStr);

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
			return LFVolumeNotReady;

	if (p_StoreDescriptor->IdxPathAux[0]!=L'\0')
		if (!DeleteDirectory(p_StoreDescriptor->IdxPathAux))
			return LFVolumeNotReady;

	// Delete internal directory
	WCHAR Path[MAX_PATH];
	GetAutoPath(*p_StoreDescriptor, Path);

	if (wcscmp(Path, p_StoreDescriptor->DatPath)!=0)
		// Ignore the error code because the directory might not exist!
		DeleteDirectory(Path);

	return LFOk;
}

UINT CStore::PrepareImport(LPCWSTR pFilename, LPCSTR pExtension, LFItemDescriptor* pItemDescriptor, LPWSTR /*pPath*/, SIZE_T /*cCount*/)
{
	assert(pFilename);
	assert(pExtension);
	assert(pItemDescriptor);

	if (!IsStoreMounted(p_StoreDescriptor))
		return LFStoreNotMounted;

	if (!m_WriteAccess)
		return LFIndexAccessError;

	// Flags
	pItemDescriptor->Type = LFTypeFile;

	// Filename and extension
	SetAttribute(pItemDescriptor, LFAttrFileName, pFilename);
	SetAttribute(pItemDescriptor, LFAttrFileFormat, pExtension);

	// StoreID
	pItemDescriptor->StoreID = p_StoreDescriptor->StoreID;

	// Randomize
	SYSTEMTIME SystemTime;
	GetSystemTime(&SystemTime);
	srand(SystemTime.wMilliseconds*rand());

	return LFOk;
}

UINT CStore::RenameFile(const HORCRUXFILE& File, LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	if (!IsStoreMounted(p_StoreDescriptor))
		return LFStoreNotMounted;

	if (!m_WriteAccess)
		return LFIndexAccessError;

	UINT Result;
	WCHAR Path1[2*MAX_PATH];
	if ((Result=GetFilePath(File, Path1, 2*MAX_PATH))!=LFOk)
		return Result;

	WCHAR Path2[2*MAX_PATH];
	if ((Result=GetFilePath(pItemDescriptor, Path2, 2*MAX_PATH))!=LFOk)
		return Result;

	return FileExists(Path1) ? MoveFile(Path1, Path2) ? LFOk : (GetLastError()==ERROR_WRITE_PROTECT) ? LFVolumeWriteProtected : LFCannotRenameFile : LFNoFileBody;
}

void CStore::SetAttributesFromStore(LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	// Only for media files and color tables
	if (LFIsMediaFile(pItemDescriptor) || (LFGetSystemContextID(pItemDescriptor)==LFContextColorTables))
	{
		// Buffer
		WCHAR Name[256];
		wcscpy_s(Name, 256, pItemDescriptor->CoreAttributes.FileName);

		// Extract annotation in brackets
		WCHAR Annotation[256];
		Annotation[0] = L'\0';

		if (Name[0] && (Name[wcslen(Name)-1]==L')'))
		{
			LPWSTR pBracket = wcsrchr(Name, L'(');
			if (pBracket)
			{
				wcsncpy_s(Annotation, 256, pBracket+1, wcslen(pBracket)-2);

				if (SetAttributesFromAnnotation(pItemDescriptor, Annotation))
				{
					// Remove annotation and trim file name
					while ((pBracket>Name) && (*(pBracket-1)==L' '))
						*pBracket--;

					*pBracket = L'\0';
				}
			}
		}

		// Only for media files
		if (LFIsMediaFile(pItemDescriptor))
		{
			// Find separator char
			LPCWSTR pSeparator = wcsstr(Name, L" ? ");
			SIZE_T SeparatorLength = 3;
			BOOL SeparatorIsSpace = FALSE;

			if (!pSeparator)
			{
				pSeparator = wcschr(Name, L'?');
				SeparatorLength = 1;
			}

			// Find space character separating name and number
			if (!pSeparator)
			{
				if ((pSeparator=wcsrchr(Name, L' '))!=NULL)
				{
					// Only (and at least one) number chars following the right-most space?
					LPCWSTR pChar = pSeparator+1;

					do
					{
						if ((*pChar<L'0') || (*pChar>L'9'))
						{
							// No, so there is no separator!
							pSeparator = NULL;

							break;
						}

						// When the number gets larger than 4 digits, do not recognize
						// it as it is probably a year number in the file name. We do
						// not want that as media title!
						if (pChar-pSeparator>3)
						{
							pSeparator = NULL;

							break;
						}
					}
					while (*(++pChar));
				}

				SeparatorLength = 1;
				SeparatorIsSpace = TRUE;
			}

			if (pSeparator)
			{
				// Creator or media collection
				const ATTRIBUTE Attr = (pItemDescriptor->CoreAttributes.SystemContextID!=LFContextAudio) && LFIsNullAttribute(pItemDescriptor, LFAttrMediaCollection) ? LFAttrMediaCollection : LFAttrCreator;

				// If the separator is a space (and thus the remainder a number), do not use as LFAttrCreator
				if ((Attr==LFAttrCreator) && SeparatorIsSpace)
					// Use whole file name as title
					goto SetTitle;

				// Remove leading chars that are not letters
				WCHAR Value[256];
				wcsncpy_s(Value, 256, Name, pSeparator-Name);

				for (LPCWSTR pChar=Value; pChar; pChar++)
					if (*pChar>=L'A')
					{
						SetAttribute(pItemDescriptor, Attr, Value);
						break;
					}

				// Title
				if (!SeparatorIsSpace)
				{
					LPCWSTR pTitle = pSeparator+SeparatorLength;

					if (*pTitle)
						SetAttribute(pItemDescriptor, LFAttrTitle, pTitle);
				}
			}
		}
		else
		{
SetTitle:
			if (!LFIsPictureFile(pItemDescriptor) && LFIsNullAttribute(pItemDescriptor, LFAttrTitle))
				SetAttribute(pItemDescriptor, LFAttrTitle, Name);
		}
	}

	// Only for documents
	if (LFIsDocumentFileLoose(pItemDescriptor))
	{
		if (LFIsNullAttribute(pItemDescriptor, LFAttrTitle))
			SetAttribute(pItemDescriptor, LFAttrTitle, pItemDescriptor->CoreAttributes.FileName);
	}
}

void CStore::SynchronizeMatch(const HORCRUXFILE& /*File*/)
{
}

BOOL CStore::SynchronizeCommit(const HORCRUXFILE& /*File*/)
{
	// Always keep file
	return TRUE;
}


// Aux functions
//

void CStore::CreateNewFileID(FILEID& FileID) const
{
	assert(m_pIndexMain);

	FileID[LFKeySize-1] = '\0';

	do
	{
		for (UINT a=0; a<LFKeySize-1; a++)
			FileID[a] = RAND_CHAR();
	}
	while (m_pIndexMain->ExistingFileID(FileID));
}

UINT CStore::PrepareImport(LPCWSTR pSourcePath, LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount)
{
	assert(pSourcePath);
	assert(pItemDescriptor);

	// Name
	WCHAR Filename[256];
	LPCWSTR pChar = wcsrchr(pSourcePath, L'\\');
	wcscpy_s(Filename, 256, pChar ? pChar+1 : pSourcePath);

	// Extension
	CHAR Extension[LFExtSize] = { 0 };
	WCHAR* pLastExt = wcsrchr(Filename, L'.');
	if (pLastExt)
	{
		pChar = pLastExt+1;
		SIZE_T cCount = 0;

		while (*pChar && (cCount<LFExtSize-1))
		{
			Extension[cCount++] = (*pChar<=0xFF) ? tolower(*pChar) & 0xFF : L'_';
			pChar++;
		}

		*pLastExt = L'\0';
	}

	// Callback
	return PrepareImport(Filename, Extension, pItemDescriptor, pPath, cCount);
}

UINT CStore::DeleteFile()
{
	switch (GetLastError())
	{
	case ERROR_NO_MORE_FILES:
	case ERROR_FILE_NOT_FOUND:
	case ERROR_PATH_NOT_FOUND:
		return LFOk;

	case ERROR_WRITE_PROTECT:
		return LFVolumeWriteProtected;

	default:
		return LFCannotDeleteFile;
	}
}


// Statistics

#define COUNT_FILE(ContextOps, TaskOps) \
	if (CoreAttributes.State & LFItemStateTrash) \
	{ \
		ContextOps(LFContextTrash); \
	} \
	else \
		if (CoreAttributes.State & LFItemStateArchive) \
		{ \
			ContextOps(LFContextArchive); \
		} \
		else \
		{ \
			if (!LFIsFilterFile(CoreAttributes)) \
				ContextOps(LFContextAllFiles); \
			const BYTE UserContext = LFGetUserContextID(CoreAttributes); \
			if (UserContext) \
				ContextOps(UserContext); \
			if (CoreAttributes.Rating) \
				ContextOps(LFContextFavorites); \
			if (CoreAttributes.State & LFItemStateTask) \
			{ \
				ContextOps(LFContextTasks); \
				TaskOps(CoreAttributes.Priority); \
			} \
			if (CoreAttributes.State & LFItemStateNew) \
				ContextOps(LFContextNew); \
		}

void CStore::AddFileToStatistics(LFStatistics& Statistics, const LFCoreAttributes& CoreAttributes) const
{
	#define ADDFILE_CONTEXT(Context) { Statistics.FileCount[Context]++; Statistics.FileSize[Context] += CoreAttributes.FileSize; }
	#define ADDFILE_TASK(Priority) { Statistics.TaskCount[Priority]++; }
	COUNT_FILE(ADDFILE_CONTEXT, ADDFILE_TASK);
}

void CStore::RemoveFileFromStatistics(LFStatistics& Statistics, const LFCoreAttributes& CoreAttributes) const
{
	#define SUBFILE_CONTEXT(Context) { Statistics.FileCount[Context]--; Statistics.FileSize[Context] -= CoreAttributes.FileSize; }
	#define SUBFILE_TASK(Priority) { Statistics.TaskCount[Priority]--; }
	COUNT_FILE(SUBFILE_CONTEXT, SUBFILE_TASK);
}
