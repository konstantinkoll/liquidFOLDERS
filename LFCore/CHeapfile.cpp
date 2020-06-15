
#include "stdafx.h"
#include "CHeapfile.h"
#include "FileSystem.h"
#include "TableIndexes.h"
#include <io.h>
#include <malloc.h>
#include <stdio.h>


// CHeapFile
//

CHeapfile::CHeapfile(LPCWSTR Path, UINT TableID, UINT StoreDataSize, BOOL Initialize)
{
	assert(sizeof(HeapfileHeader)==512);

	m_pBuffer = NULL;
	m_ItemCount = 0;
	m_BufferNeedsWriteback = m_HeaderNeedsWriteback = FALSE;

	// Path
	wcscpy_s(m_Path, MAX_PATH, Path);
	wcscat_s(m_Path, MAX_PATH, IndexTables[TableID].FileName);

	// Table
	m_RequiredElementSize = IndexTables[TableID].Size+(m_StoreDataSize=(TableID==IDXTABLE_MASTER) ? StoreDataSize : 0);
	if ((m_TableID=TableID)==IDXTABLE_MASTER)
	{
		m_KeyOffset = offsetof(LFCoreAttributes, FileID);
	}
	else
	{
		m_KeyOffset = m_RequiredElementSize;
		m_RequiredElementSize += LFKeySize;
	}

	// Open file
	if ((m_OpenStatus=CreateFileConcurrent(m_Path, TRUE, Initialize ? OPEN_ALWAYS : OPEN_EXISTING, hFile))!=FileOk)
	{
		if (GetLastError()==ERROR_FILE_NOT_FOUND)
			m_OpenStatus = HeapCannotCreate;

		return;
	}

	LARGE_INTEGER Size;
	if (!GetFileSizeEx(hFile, &Size))
	{
		m_OpenStatus = HeapError;
		return;
	}

	if (Size.QuadPart>=sizeof(HeapfileHeader))
	{
		// Sufficient file size
		DWORD Read;
		if (!ReadFile(hFile, &m_Header, sizeof(HeapfileHeader), &Read, NULL))
		{
			m_OpenStatus = HeapError;
			return;
		}

		// Header invalid: (re)create file
		if ((Read!=sizeof(HeapfileHeader)) || (strcmp(m_Header.ID, HeapSignature)!=0))
			goto Create;

		// Process header
		m_ItemCount = (UINT_PTR)((Size.QuadPart-sizeof(HeapfileHeader))/m_Header.ElementSize);
		m_OpenStatus = (m_Header.Version<CURIDXVERSION) ? HeapMaintenanceRequired : HeapOk;

		// Adjustments for other tuple sizes
		if (m_Header.ElementSize!=m_RequiredElementSize)
		{
			if (m_KeyOffset==m_RequiredElementSize-LFKeySize)
				m_KeyOffset = m_Header.ElementSize-LFKeySize;

			if (m_Header.ElementSize<m_RequiredElementSize)
				m_OpenStatus = HeapMaintenanceRequired;
		}
	}
	else
	{
Create:
		// (Re)create file
		ZeroMemory(&m_Header, sizeof(HeapfileHeader));

		strcpy_s(m_Header.ID, sizeof(m_Header.ID), HeapSignature);
		m_Header.ElementSize = m_RequiredElementSize;
		m_Header.Version = CURIDXVERSION;
		m_Header.StoreDataSize = m_StoreDataSize;

		m_HeaderNeedsWriteback = TRUE;

		if (!WriteHeader())
		{
			m_OpenStatus = HeapCannotCreate;
			return;
		}

		SetEndOfFile(hFile);
		CompressFile(hFile, (CHAR)m_Path[0]);

		m_OpenStatus = HeapCreated;
	}

	AllocBuffer();
}

CHeapfile::~CHeapfile()
{
	CloseFile();

	free(m_pBuffer);
}

LPVOID CHeapfile::GetStoreData(LPCVOID pData) const
{
	return m_Header.StoreDataSize ? (LPBYTE)pData+m_Header.ElementSize-m_Header.StoreDataSize : NULL;
}

UINT CHeapfile::GetError(BOOL SingleStore)
{
	switch (m_OpenStatus)
	{
	case HeapOk:
	case HeapCreated:
	case HeapMaintenanceRequired:
		return LFOk;

	case HeapSharingViolation:
		return SingleStore ? LFSharingViolation1 : LFSharingViolation2;

	case HeapNoAccess:
		return LFIndexAccessError;

	default:
		return LFIndexTableLoadError;
	}
}


// File IO

void CHeapfile::AllocBuffer()
{
	assert(m_Header.ElementSize);

	// Free old buffer
	free(m_pBuffer);

	// Calculate exact size of new buffer
	m_BufferCount = MaxBufferSize/m_Header.ElementSize;
	if (m_BufferCount<2)
		m_BufferCount = 2;

	// Allocate buffer
	m_pBuffer = (LPBYTE)malloc(m_BufferCount*m_Header.ElementSize);

	// Set invalid
	m_FirstInBuffer = m_LastInBuffer = -1;
}

void CHeapfile::CloseFile()
{
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		WriteHeader();
		Flush();
		CloseHandle(hFile);

		hFile = INVALID_HANDLE_VALUE;
	}
}

void CHeapfile::ElementToBuffer(INT_PTR ID)
{
	assert(m_OpenStatus<=HeapMaintenanceRequired);
	assert(ID<(INT_PTR)m_ItemCount);

	Flush();

	LARGE_INTEGER Pos;
	Pos.QuadPart = sizeof(HeapfileHeader)+ID*m_Header.ElementSize;

	if (SetFilePointerEx(hFile, Pos, NULL, FILE_BEGIN))
	{
		DWORD Read;
		if (ReadFile(hFile, m_pBuffer, m_BufferCount*m_Header.ElementSize, &Read, NULL))
		{
			m_FirstInBuffer = ID;
			m_LastInBuffer = ID+(Read/m_Header.ElementSize)-1;
		}
	}
}

BOOL CHeapfile::WriteHeader()
{
	assert(hFile!=INVALID_HANDLE_VALUE);

	if (!m_HeaderNeedsWriteback)
		return TRUE;

	LARGE_INTEGER Pos;
	Pos.QuadPart = 0;

	if (SetFilePointerEx(hFile, Pos, NULL, FILE_BEGIN))
	{
		DWORD Written;
		if (WriteFile(hFile, &m_Header, sizeof(HeapfileHeader), &Written, NULL))
			if (Written==sizeof(HeapfileHeader))
			{
				m_HeaderNeedsWriteback = FALSE;
				return TRUE;
			}
	}

	return FALSE;
}

void CHeapfile::Flush()
{
	assert(hFile!=INVALID_HANDLE_VALUE);

	if (m_BufferNeedsWriteback)
	{
		LARGE_INTEGER Pos;
		Pos.QuadPart = sizeof(HeapfileHeader)+m_FirstInBuffer*m_Header.ElementSize;

		if (SetFilePointerEx(hFile, Pos, NULL, FILE_BEGIN))
		{
			DWORD Written;
			WriteFile(hFile, m_pBuffer, (DWORD)((m_LastInBuffer-m_FirstInBuffer+1)*m_Header.ElementSize), &Written, NULL);

			m_BufferNeedsWriteback = FALSE;
		}
	}
}


// Search and modification

#define SCANHEAPFILE(Next, Ptr, Condition) \
	assert(m_OpenStatus<=HeapMaintenanceRequired); \
	LPCFILEID lpcFileID = Ptr ? (LPCFILEID)((LPBYTE)Ptr+m_KeyOffset) : GetFileID(Next-1); \
	do \
	{ \
		if (Next>=(INT_PTR)m_ItemCount) \
			return FALSE; \
		if ((Next<m_FirstInBuffer) || (Next>m_LastInBuffer)) \
		{ \
			ElementToBuffer(Next); \
			lpcFileID = GetFileID(Next); \
		} \
		else \
		{ \
			lpcFileID = (LPCFILEID)((LPBYTE)lpcFileID+m_Header.ElementSize); \
		} \
		Next++; \
	} \
	while (Condition); \
	Ptr = (LPBYTE)lpcFileID-m_KeyOffset; \
	return TRUE;

BOOL CHeapfile::FindNext(INT_PTR& Next, LPVOID& Ptr)
{
	SCANHEAPFILE(Next, Ptr, *lpcFileID[0]=='\0');
}

BOOL CHeapfile::FindKey(const FILEID& FileID, INT_PTR& Next, LPVOID& Ptr)
{
	SCANHEAPFILE(Next, Ptr, *lpcFileID!=FileID);
}

void CHeapfile::Add(LFItemDescriptor* pItemDescriptor)
{
	assert(m_OpenStatus<=HeapMaintenanceRequired);
	assert(pItemDescriptor);

	if (m_FirstInBuffer==-1)
	{
		// Buffer unused
		m_FirstInBuffer = m_ItemCount;
	}
	else
		if ((m_LastInBuffer!=(INT_PTR)m_ItemCount-1) || (m_LastInBuffer-m_FirstInBuffer+1>=(INT_PTR)m_BufferCount))
		{
			// Wrong elements in buffer, or not enough space
			Flush();
			m_FirstInBuffer = m_ItemCount;
		}

	m_LastInBuffer = m_ItemCount;

	// Add in RAM
	LPBYTE Ptr = m_pBuffer+(m_ItemCount-m_FirstInBuffer)*m_Header.ElementSize;
	GetFromItemDescriptor(Ptr, pItemDescriptor);

	if (m_KeyOffset==m_Header.ElementSize-LFKeySize)
		*((LPFILEID)(Ptr+m_Header.ElementSize-LFKeySize)) = pItemDescriptor->CoreAttributes.FileID;

	MakeDirty();

	m_ItemCount++;
}

void CHeapfile::Update(LFItemDescriptor* pItemDescriptor, LPVOID Ptr)
{
	assert(m_OpenStatus<=HeapMaintenanceRequired);
	assert(pItemDescriptor);

	GetFromItemDescriptor(Ptr, pItemDescriptor);

	MakeDirty();
}

void CHeapfile::Update(LFItemDescriptor* pItemDescriptor, INT_PTR& Next)
{
	assert(m_OpenStatus<=HeapMaintenanceRequired);
	assert(pItemDescriptor);

	LPVOID Ptr = NULL;
	if (FindKey(pItemDescriptor->CoreAttributes.FileID, Next, Ptr))
		Update(pItemDescriptor, Ptr);
}

void CHeapfile::Update(LFItemDescriptor* pItemDescriptor)
{
	assert(m_OpenStatus<=HeapMaintenanceRequired);
	assert(pItemDescriptor);

	INT_PTR ID = 0;
	Update(pItemDescriptor, ID);
}

void CHeapfile::Invalidate(LPVOID Ptr)
{
	assert(m_OpenStatus<=HeapMaintenanceRequired);

	*((LPBYTE)Ptr+m_KeyOffset) = 0;

	MakeDirty(TRUE);
}

void CHeapfile::Invalidate(const FILEID& FileID, INT_PTR& Next)
{
	assert(m_OpenStatus<=HeapMaintenanceRequired);

	LPVOID Ptr = NULL;
	if (FindKey(FileID, Next, Ptr))
		Invalidate(Ptr);
}

void CHeapfile::Invalidate(LFItemDescriptor* pItemDescriptor)
{
	assert(m_OpenStatus<=HeapMaintenanceRequired);
	assert(pItemDescriptor);

	INT_PTR ID = 0;
	Invalidate(pItemDescriptor->CoreAttributes.FileID, ID);
}

void CHeapfile::ZeroCopy(LPVOID pDst, const SIZE_T DstSize, LPCVOID pSrc, const SIZE_T SrcSize)
{
	assert(pDst);
	assert(pSrc);
	assert(SrcSize);
	assert(DstSize);

	memcpy(pDst, pSrc, min(DstSize, SrcSize));

	if (DstSize>SrcSize)
		ZeroMemory((LPBYTE)pDst+SrcSize, DstSize-SrcSize);
}

void CHeapfile::GetAttribute(LPVOID Ptr, SIZE_T Offset, ATTRIBUTE Attr, LFItemDescriptor* pItemDescriptor) const
{
	assert(Ptr);
	assert(Attr<LFAttributeCount);

	if (pItemDescriptor->AttributeValues[Attr])
	{
		const SIZE_T Size = GetAttributeSize(Attr, pItemDescriptor->AttributeValues[Attr]);

		UINT EndOfTuple = m_Header.ElementSize;
		if (m_KeyOffset==m_Header.ElementSize-LFKeySize)
			EndOfTuple -= LFKeySize;

		if (Offset+Size<=EndOfTuple)
			memcpy((LPBYTE)Ptr+Offset, pItemDescriptor->AttributeValues[Attr], Size);
	}
}

void CHeapfile::GetFromItemDescriptor(LPVOID Ptr, LFItemDescriptor* pItemDescriptor)
{
	assert(Ptr);
	assert(pItemDescriptor);

	if (m_TableID==IDXTABLE_MASTER)
	{
		const SIZE_T DataSize = m_Header.ElementSize-m_Header.StoreDataSize;
		ZeroCopy(Ptr, DataSize, &pItemDescriptor->CoreAttributes, sizeof(LFCoreAttributes));

		if (m_Header.StoreDataSize)
			ZeroCopy((LPBYTE)Ptr+DataSize, m_Header.StoreDataSize, &pItemDescriptor->StoreData, LFMaxStoreDataSize);
	}
	else
	{
		assert(m_KeyOffset==m_Header.ElementSize-LFKeySize);

		ZeroMemory(Ptr, m_Header.ElementSize-LFKeySize);

		for (UINT a=0; a<IndexTables[m_TableID].cTableEntries; a++)
			GetAttribute(Ptr, IndexTables[m_TableID].pTableEntries[a].Offset, IndexTables[m_TableID].pTableEntries[a].Attr, pItemDescriptor);
	}
}


// Compaction

BOOL CHeapfile::Compact()
{
	if ((!m_Header.NeedsCompaction) && (m_OpenStatus!=HeapMaintenanceRequired))
		return TRUE;

	// Temporary filename
	WCHAR TempFilename[MAX_PATH];
	wcscpy_s(TempFilename, MAX_PATH, m_Path);
	wcscat_s(TempFilename, MAX_PATH, L".part");

	// Open temporary file
	HANDLE hTempFile;
	if (CreateFileConcurrent(TempFilename, TRUE, OPEN_ALWAYS, hTempFile)!=FileOk)
		return FALSE;

	CompressFile(hTempFile, (CHAR)TempFilename[0]);

	// Temporary header
	HeapfileHeader TempHeader = m_Header;
	TempHeader.ElementSize = max(m_Header.ElementSize, m_RequiredElementSize);
	TempHeader.StoreDataSize = m_StoreDataSize;
	TempHeader.NeedsCompaction = FALSE;
	TempHeader.Version = CURIDXVERSION;

	#define ABORT { CloseHandle(hTempFile); DeleteFile(TempFilename); return FALSE; }

	DWORD Written;
	if (!WriteFile(hTempFile, &TempHeader, sizeof(HeapfileHeader), &Written, NULL))
		ABORT
	if (Written!=sizeof(HeapfileHeader))
		ABORT

	// Copy (and enlarge) tuples
	SIZE_T PaddedDataSize = m_Header.StoreDataSize;
	if (m_KeyOffset==m_Header.ElementSize-LFKeySize)
		PaddedDataSize += LFKeySize;

	LPBYTE pTempBuffer = (LPBYTE)malloc(TempHeader.ElementSize);
	ZeroMemory(pTempBuffer, TempHeader.ElementSize);

	INT TempCount = 0;

	INT_PTR ID = 0;
	LPBYTE Ptr = NULL;
	while (FindNext(ID, (LPVOID&)Ptr))
	{
		memcpy(pTempBuffer, Ptr, m_Header.ElementSize-PaddedDataSize);

		if (PaddedDataSize)
			memcpy(pTempBuffer+TempHeader.ElementSize-PaddedDataSize, Ptr+m_Header.ElementSize-PaddedDataSize, PaddedDataSize);

		if (!WriteFile(hTempFile, pTempBuffer, TempHeader.ElementSize, &Written, NULL))
			ABORT
		if (Written!=TempHeader.ElementSize)
			ABORT

		TempCount++;
	};

	m_HeaderNeedsWriteback = FALSE;
	CloseFile();

	free(pTempBuffer);

	CloseHandle(hTempFile);

	// Temporary file becomes heapfile
	if (!DeleteFile(m_Path))
		return FALSE;

	if (!MoveFile(TempFilename, m_Path))
		return FALSE;

	// Process new header
	if (m_KeyOffset==m_Header.ElementSize-LFKeySize)
		m_KeyOffset = TempHeader.ElementSize-LFKeySize;

	m_Header = TempHeader;
	m_ItemCount = TempCount;

	AllocBuffer();

	// Reopen file
	return ((m_OpenStatus=CreateFileConcurrent(m_Path, TRUE, OPEN_EXISTING, hFile))==FileOk);
}
