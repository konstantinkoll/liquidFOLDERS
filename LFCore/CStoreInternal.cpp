
#include "stdafx.h"
#include "CStoreInternal.h"
#include "FileSystem.h"
#include <assert.h>


// CStoreInternal
//

CStoreInternal::CStoreInternal(LFStoreDescriptor* pStoreDescriptor, HMUTEX hMutexForStore)
	: CStore(pStoreDescriptor, hMutexForStore)
{
}


// Non-Index operations
//

UINT CStoreInternal::PrepareDelete()
{
	// Data path writeable?
	if (p_StoreDescriptor->DatPath[0]!=L'\0')
	{
		WCHAR Path[MAX_PATH];
		wcscpy_s(Path, MAX_PATH, p_StoreDescriptor->DatPath);
		wcscat_s(Path, MAX_PATH, L"*");

		if (FileExists(Path))
			if (!DirectoryWriteable(p_StoreDescriptor->DatPath))
				return LFDriveWriteProtected;
	}

	return CStore::PrepareDelete();
}


// Callbacks
//

UINT CStoreInternal::CreateDirectories()
{
	UINT Result = CStore::CreateDirectories();

	// Hide data path
	if ((Result==LFOk) && (p_StoreDescriptor->DatPath[0]!=L'\0') && ((p_StoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal))
		SetFileAttributes(p_StoreDescriptor->DatPath, FILE_ATTRIBUTE_HIDDEN);

	return Result;
}

UINT CStoreInternal::DeleteDirectories()
{
	UINT Result;

	// Delete index directories
	if ((Result=CStore::DeleteDirectories())!=LFOk)
		return Result;

	// Delete data directory
	if (p_StoreDescriptor->DatPath[0]!=L'\0')
		Result = DeleteDirectory(p_StoreDescriptor->DatPath) ? LFOk : LFDriveNotReady;

	return Result;
}
