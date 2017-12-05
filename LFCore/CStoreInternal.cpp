
#include "stdafx.h"
#include "CStoreInternal.h"
#include "FileSystem.h"
#include "LFCore.h"
#include "LFVariantData.h"
#include "Stores.h"
#include <assert.h>


extern CHAR KeyChars[38];


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
			if (!VolumeWriteable((CHAR)p_StoreDescriptor->DatPath[0]))
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
	if ((p_StoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal)
		if (p_StoreDescriptor->DatPath[0]!=L'\0')
			Result = DeleteDirectory(p_StoreDescriptor->DatPath) ? LFOk : LFDriveNotReady;

	return Result;
}

UINT CStoreInternal::PrepareImport(LPCWSTR pFilename, LPCSTR pExtension, LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount)
{
	assert(pFilename);
	assert(pExtension);
	assert(pItemDescriptor);
	assert(pPath);
	assert(cCount>=2*MAX_PATH);

	UINT Result;

	if ((Result=CStore::PrepareImport(pFilename, pExtension, pItemDescriptor, pPath, cCount))!=LFOk)
		return Result;

	// FileID
	pItemDescriptor->CoreAttributes.FileID[0] = RAND_CHAR();
	ZeroMemory(&pItemDescriptor->CoreAttributes.FileID[1], LFKeySize-1);

	// 1st directory level
	GetInternalFilePath(pItemDescriptor->CoreAttributes, pPath, cCount);

	Result = CreateDirectory(pPath);
	if ((Result!=ERROR_SUCCESS) && (Result!=ERROR_ALREADY_EXISTS))
		return LFIllegalPhysicalPath;

	// 2nd directory level
	do
	{
		for (UINT a=1; a<LFKeySize-1; a++)
			pItemDescriptor->CoreAttributes.FileID[a] = RAND_CHAR();

		GetInternalFilePath(pItemDescriptor->CoreAttributes, pPath, cCount);
	}
	while (FileExists(pPath));

	Result = CreateDirectory(pPath);
	if ((Result!=ERROR_SUCCESS) && (Result!=ERROR_ALREADY_EXISTS))
		return LFIllegalPhysicalPath;

	return GetFileLocation(pItemDescriptor, pPath, cCount);
}
