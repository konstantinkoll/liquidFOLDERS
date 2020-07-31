
#include "stdafx.h"
#include "CStoreInternal.h"
#include "FileSystem.h"
#include "LFCore.h"
#include "LFVariantData.h"
#include "Stores.h"


extern CHAR KeyChars[38];


// CStoreInternal
//

CStoreInternal::CStoreInternal(LFStoreDescriptor* pStoreDescriptor, HMUTEX hMutexForStore)
	: CStore(pStoreDescriptor, hMutexForStore)
{
}

void CStoreInternal::GetInternalFilePath(const LFCoreAttributes& CoreAttributes, LPWSTR pPath, SIZE_T cCount) const
{
	assert(pPath);
	assert(cCount>=2*MAX_PATH);

	WCHAR Buffer[LFKeySize+1];
	Buffer[0] = CoreAttributes.FileID[0];
	Buffer[1] = L'\\';
	MultiByteToWideChar(CP_ACP, 0, &CoreAttributes.FileID[1], -1, &Buffer[2], LFKeySize-1);

	wcscpy_s(pPath, cCount, L"\\\\?\\");
	wcscat_s(pPath, cCount, p_StoreDescriptor->DatPath);
	wcscat_s(pPath, cCount, Buffer);
}


// Non-Index operations
//

UINT CStoreInternal::GetFilePath(const HORCRUXFILE& File, LPWSTR pPath, SIZE_T cCount) const
{
	assert(pPath);
	assert(cCount>=2*MAX_PATH);

	if (!IsStoreMounted(p_StoreDescriptor))
		return LFStoreNotMounted;

	WCHAR Buffer[MAX_PATH];
	SanitizeFileName(Buffer, MAX_PATH, ((LPCCOREATTRIBUTES)File)->FileName);

	GetInternalFilePath(File, pPath, cCount);
	wcscat_s(pPath, cCount, L"\\");
	wcsncat_s(pPath, cCount, Buffer, 127);

	if (((LPCCOREATTRIBUTES)File)->FileFormat[0])
	{
		WCHAR Buffer[LFExtSize];
		MultiByteToWideChar(CP_ACP, 0, ((LPCCOREATTRIBUTES)File)->FileFormat, -1, Buffer, LFExtSize);

		wcscat_s(pPath, cCount, L".");
		wcscat_s(pPath, cCount, Buffer);
	}

	return LFOk;
}

UINT CStoreInternal::PrepareDelete()
{
	// Data path writeable?
	if (IsStoreMounted(p_StoreDescriptor))
	{
		WCHAR Path[MAX_PATH];
		wcscpy_s(Path, MAX_PATH, p_StoreDescriptor->DatPath);
		wcscat_s(Path, MAX_PATH, L"*");

		if (FileExists(Path) && !(p_StoreDescriptor->Flags & LFFlagsWriteable))
			return LFVolumeWriteProtected;
	}

	return CStore::PrepareDelete();
}


// Callbacks
//

UINT CStoreInternal::CreateDirectories()
{
	UINT Result = CStore::CreateDirectories();

	// Hide data directory when it exists, but not for stores with an internal index and auto-location enabled!
	if ((Result==LFOk) && IsStoreMounted(p_StoreDescriptor) && ((p_StoreDescriptor->IndexMode!=LFStoreIndexModeInternal) || !(p_StoreDescriptor->State & LFStoreStateAutoLocation)))
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
	if (IsStoreMounted(p_StoreDescriptor))
		Result = DeleteDirectory(p_StoreDescriptor->DatPath) ? LFOk : LFVolumeNotReady;

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

	return GetFilePath(pItemDescriptor, pPath, cCount);
}

UINT CStoreInternal::DeleteFile(const HORCRUXFILE& File)
{
	WCHAR Path[2*MAX_PATH];
	UINT Result;
	if ((Result=GetFilePath(File, Path, 2*MAX_PATH))!=LFOk)
		return Result;

	WCHAR* pChar = wcsrchr(Path, L'\\');
	if (pChar)
		*(pChar+1) = L'\0';

	return DeleteDirectory(Path) ? LFOk : CStore::DeleteFile();
}
