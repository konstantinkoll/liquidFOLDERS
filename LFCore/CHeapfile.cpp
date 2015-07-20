
#include "stdafx.h"
#include "CHeapfile.h"
#include "IndexTables.h"
#include <assert.h>
#include <io.h>
#include <malloc.h>
#include <stdio.h>
#include <winioctl.h>


void Compress(HANDLE hFile, WCHAR cDrive)
{
	// NTFS compression
	BY_HANDLE_FILE_INFORMATION FileInformation;
	if (GetFileInformationByHandle(hFile, &FileInformation))
		if ((FileInformation.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED)==0)
		{
			WCHAR Root[4] = L" :\\";
			Root[0] = cDrive;

			DWORD Flags;
			if (GetVolumeInformation(Root, NULL, 0, NULL, NULL, &Flags, NULL, 0))
				if (Flags & FS_FILE_COMPRESSION)
				{
					USHORT Mode = COMPRESSION_FORMAT_LZNT1;
					DWORD Returned;

					DeviceIoControl(hFile, FSCTL_SET_COMPRESSION, &Mode, sizeof(Mode), NULL, 0, &Returned, NULL);
				}
		}
}

void ZeroCopy(void* pDst, const SIZE_T DstSize, void* pSrc, const SIZE_T SrcSize)
{
	memcpy(pDst, pSrc, min(DstSize, SrcSize));

	if (DstSize>SrcSize)
		ZeroMemory((BYTE*)pDst+SrcSize, DstSize-SrcSize);
}


// CHeapFile
//

#define OPENFILE(Disposition) CreateFile(m_Filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, Disposition, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

CHeapfile::CHeapfile(WCHAR* Path, BYTE TableID)
{
	assert(sizeof(HeapfileHeader)==512);

	m_pBuffer = NULL;
	m_ItemCount = 0;
	m_BufferNeedsWriteback = m_HeaderNeedsWriteback = FALSE;

	// Filename
	wcscpy_s(m_Filename, MAX_PATH, Path);
	wcscat_s(m_Filename, MAX_PATH, LFIndexTables[TableID].FileName);

	// Table
	m_TableID = TableID;
	m_RequiredElementSize = LFIndexTables[TableID].Size;

	m_KeyOffset = (TableID==IDXTABLE_MASTER) ? offsetof(LFCoreAttributes, FileID) : 0;
	if (m_KeyOffset==0)
	{
		m_KeyOffset = m_RequiredElementSize;
		m_RequiredElementSize += LFKeySize;
	}

	// Open file
	hFile = OPENFILE(OPEN_ALWAYS);
	if (hFile==INVALID_HANDLE_VALUE)
	{
		m_OpenStatus = HeapNoAccess;
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
		m_ItemCount = (UINT)((Size.QuadPart-sizeof(HeapfileHeader))/m_Header.ElementSize);
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

		m_HeaderNeedsWriteback = TRUE;

		if (!WriteHeader())
		{
			m_OpenStatus = HeapCannotCreate;
			return;
		}

		SetEndOfFile(hFile);
		Compress(hFile, m_Filename[0]);

		m_OpenStatus = HeapCreated;
	}

	AllocBuffer();
}

CHeapfile::~CHeapfile()
{
	CloseFile();

	free(m_pBuffer);
}

UINT CHeapfile::GetItemCount()
{
	return m_ItemCount;
}

UINT CHeapfile::GetRequiredElementSize()
{
	return max(m_Header.ElementSize, m_RequiredElementSize);
}

UINT CHeapfile::GetRequiredFileSize()
{
	return GetRequiredElementSize()*m_ItemCount+sizeof(HeapfileHeader);
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
	m_pBuffer = malloc(m_BufferCount*m_Header.ElementSize);

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

void CHeapfile::ElementToBuffer(INT ID)
{
	assert(m_OpenStatus<=HeapMaintenanceRequired);

	if (((ID>=m_FirstInBuffer) && (ID<=m_LastInBuffer)) || (ID>=(INT)m_ItemCount))
		return;

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
			WriteFile(hFile, m_pBuffer, (m_LastInBuffer-m_FirstInBuffer+1)*m_Header.ElementSize, &Written, NULL);

			m_BufferNeedsWriteback = FALSE;
		}
	}
}


// Search and modification

void CHeapfile::MakeDirty(BOOL NeedsCompaction)
{
	m_BufferNeedsWriteback = TRUE;

	m_Header.NeedsCompaction |= NeedsCompaction;
	m_HeaderNeedsWriteback |= NeedsCompaction;
}

BOOL CHeapfile::FindNext(INT& Next, void*& Ptr)
{
	assert(m_OpenStatus<=HeapMaintenanceRequired);

	CHAR* pKey;

	do
	{
		if (Next>=(INT)m_ItemCount)
			return FALSE;

		ElementToBuffer(Next);

		pKey = (CHAR*)m_pBuffer+(Next-m_FirstInBuffer)*m_Header.ElementSize+m_KeyOffset;
		Next++;
	}
	while (*pKey==0);

	Ptr = pKey-m_KeyOffset;

	return TRUE;
}

BOOL CHeapfile::FindKey(CHAR* FileID, INT& Next, void*& Ptr)
{
	assert(m_OpenStatus<=HeapMaintenanceRequired);

	CHAR* pKey;

	do
	{
		if (Next>=(INT)m_ItemCount)
			return FALSE;

		ElementToBuffer(Next);

		pKey = (CHAR*)m_pBuffer+(Next-m_FirstInBuffer)*m_Header.ElementSize+m_KeyOffset;
		Next++;
	}
	while (strcmp(pKey, FileID)!=0);

	Ptr = pKey-m_KeyOffset;

	return TRUE;
}

void CHeapfile::Add(LFItemDescriptor* pItemDescriptor)
{
	assert(m_OpenStatus<=HeapMaintenanceRequired);
	assert(pItemDescriptor);

	if (m_FirstInBuffer==-1)
	{
		// Puffer unbenutzt
		m_FirstInBuffer = m_ItemCount;
	}
	else
		if ((m_LastInBuffer!=(INT)m_ItemCount-1) || (m_LastInBuffer-m_FirstInBuffer+1>=(INT)m_BufferCount))
		{
			// Falsche Elemente im Puffer, oder nicht mehr genug Platz
			Flush();
			m_FirstInBuffer = m_ItemCount;
		}

	m_LastInBuffer = m_ItemCount;

	// Im RAM hinzufügen
	BYTE* Ptr = (BYTE*)m_pBuffer+(m_ItemCount-m_FirstInBuffer)*m_Header.ElementSize;
	GetFromItemDescriptor(Ptr, pItemDescriptor);

	if (m_KeyOffset==m_Header.ElementSize-LFKeySize)
	{
		Ptr += m_Header.ElementSize-LFKeySize;
		memcpy(Ptr, pItemDescriptor->CoreAttributes.FileID, LFKeySize);
	}

	MakeDirty();

	m_ItemCount++;
}

void CHeapfile::Update(LFItemDescriptor* pItemDescriptor, void* Ptr)
{
	assert(m_OpenStatus<=HeapMaintenanceRequired);

	GetFromItemDescriptor(Ptr, pItemDescriptor);

	MakeDirty();
}

void CHeapfile::Update(LFItemDescriptor* pItemDescriptor, INT& Next)
{
	assert(m_OpenStatus<=HeapMaintenanceRequired);
	assert(pItemDescriptor);

	void* Ptr;

	if (FindKey(pItemDescriptor->CoreAttributes.FileID, Next, Ptr))
		Update(pItemDescriptor, Ptr);
}

void CHeapfile::Update(LFItemDescriptor* pItemDescriptor)
{
	assert(m_OpenStatus<=HeapMaintenanceRequired);
	assert(pItemDescriptor);

	INT ID = 0;

	Update(pItemDescriptor, ID);
}

void CHeapfile::Invalidate(void* Ptr)
{
	assert(m_OpenStatus<=HeapMaintenanceRequired);

	*((BYTE*)Ptr+m_KeyOffset) = 0;

	MakeDirty(TRUE);
}

void CHeapfile::Invalidate(CHAR* FileID, INT& Next)
{
	assert(m_OpenStatus<=HeapMaintenanceRequired);
	assert(FileID);

	void* Ptr;

	if (FindKey(FileID, Next, Ptr))
		Invalidate(Ptr);
}

void CHeapfile::Invalidate(LFItemDescriptor* pItemDescriptor)
{
	assert(m_OpenStatus<=HeapMaintenanceRequired);
	assert(pItemDescriptor);

	INT ID = 0;

	Invalidate(pItemDescriptor->CoreAttributes.FileID, ID);
}

__forceinline void CHeapfile::GetAttribute(void* PtrDst, INT_PTR Offset, UINT Attr, LFItemDescriptor* pItemDescriptor)
{
	assert(PtrDst);
	assert(Attr<LFAttributeCount);

	if (pItemDescriptor->AttributeValues[Attr])
	{
		SIZE_T Size = GetAttributeSize(Attr, pItemDescriptor->AttributeValues[Attr]);

		UINT EndOfTuple = m_Header.ElementSize;
		if (m_KeyOffset==m_Header.ElementSize-LFKeySize)
			EndOfTuple -= LFKeySize;

		if (Offset+Size<=EndOfTuple)
			memcpy((BYTE*)PtrDst+Offset, pItemDescriptor->AttributeValues[Attr], Size);
	}
}

void CHeapfile::GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* pItemDescriptor)
{
	assert(PtrDst);
	assert(pItemDescriptor);

	if (m_TableID==IDXTABLE_MASTER)
	{
		ZeroCopy(PtrDst, m_Header.ElementSize, &pItemDescriptor->CoreAttributes, sizeof(LFCoreAttributes));
	}
	else
	{
		assert(m_KeyOffset==m_Header.ElementSize-LFKeySize);

		ZeroMemory(PtrDst, m_Header.ElementSize-LFKeySize);

		for (UINT a=0; a<LFIndexTables[m_TableID].cTableEntries; a++)
			GetAttribute(PtrDst, LFIndexTables[m_TableID].pTableEntries[a].Offset, LFIndexTables[m_TableID].pTableEntries[a].Attr, pItemDescriptor);
	}
}


// Compaction

BOOL CHeapfile::Compact()
{
	if ((!m_Header.NeedsCompaction) && (m_OpenStatus!=HeapMaintenanceRequired))
		return TRUE;

	// Temporary filename
	WCHAR TempFilename[MAX_PATH];
	wcscpy_s(TempFilename, MAX_PATH, m_Filename);
	wcscat_s(TempFilename, MAX_PATH, L".part");

	// Open temporary file
	HANDLE hTempFile = OPENFILE(CREATE_ALWAYS);
	if (hTempFile==INVALID_HANDLE_VALUE)
		return FALSE;

	Compress(hTempFile, TempFilename[0]);

	// Temporary header
	HeapfileHeader TempHeader = m_Header;
	TempHeader.ElementSize = max(m_Header.ElementSize, m_RequiredElementSize);
	TempHeader.NeedsCompaction = FALSE;

	if (CURIDXVERSION>TempHeader.Version)
		TempHeader.Version = CURIDXVERSION;

	#define ABORT { CloseHandle(hTempFile); DeleteFile(TempFilename); return FALSE; }

	DWORD Written;
	if (!WriteFile(hTempFile, &TempHeader, sizeof(HeapfileHeader), &Written, NULL))
		ABORT
	if (Written!=sizeof(HeapfileHeader))
		ABORT

	// Copy (and enlarge) tuples
	BYTE* pTempBuffer = (BYTE*)malloc(TempHeader.ElementSize);
	ZeroMemory(pTempBuffer, TempHeader.ElementSize);

	INT TempCount = 0;

	INT Next = 0;
	void* Ptr;

	while (FindNext(Next, Ptr))
	{
		if (m_KeyOffset==m_Header.ElementSize-LFKeySize)
		{
			memcpy(pTempBuffer, Ptr, m_Header.ElementSize-LFKeySize);

			// Move keys to end of tuple
			memcpy(pTempBuffer+TempHeader.ElementSize-LFKeySize, (BYTE*)Ptr+m_Header.ElementSize-LFKeySize, LFKeySize);
		}
		else
		{
			memcpy(pTempBuffer, Ptr, m_Header.ElementSize);
		}

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
	if (!DeleteFile(m_Filename))
		return FALSE;

	if (!MoveFile(TempFilename, m_Filename))
		return FALSE;

	// Process new header
	if (m_KeyOffset==m_Header.ElementSize-LFKeySize)
		m_KeyOffset = TempHeader.ElementSize-LFKeySize;

	m_Header = TempHeader;
	m_ItemCount = TempCount;

	AllocBuffer();

	// Open file
	hFile = OPENFILE(OPEN_EXISTING);
	if (hFile==INVALID_HANDLE_VALUE)
	{
		m_OpenStatus = HeapNoAccess;

		return FALSE;
	}

	m_OpenStatus = HeapOk;

	return TRUE;
}
