
#include "stdafx.h"
#include "CHeapfile.h"
#include "IdxTables.h"
#include <assert.h>
#include <io.h>
#include <malloc.h>
#include <stdio.h>
#include <winioctl.h>


void Compress(HANDLE hFile, WCHAR* IdxFilename)
{
	// NTFS compression
	BY_HANDLE_FILE_INFORMATION fi;
	if (GetFileInformationByHandle(hFile, &fi))
		if ((fi.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED)==0)
		{
			WCHAR Root[4];
			wcsncpy_s(Root, 4, IdxFilename, 3);

			WCHAR VolumeName[MAX_PATH];
			DWORD Flags;
			if (GetVolumeInformation(Root, VolumeName, MAX_PATH, NULL, NULL, &Flags, NULL, 0))
				if (Flags & FS_FILE_COMPRESSION)
				{
					unsigned short mode = COMPRESSION_FORMAT_LZNT1;
					DWORD returned = 0;
					DeviceIoControl(hFile, FSCTL_SET_COMPRESSION, &mode, sizeof(mode), NULL, 0, &returned, NULL);
				}
		}
}


// CHeapFile
//

CHeapfile::CHeapfile(WCHAR* Path, WCHAR* Filename, UINT _ElementSize, UINT _KeyOffset)
{
	assert(sizeof(HeapfileHeader)==512);
	assert(_ElementSize);
	ZeroMemory(&Hdr, sizeof(HeapfileHeader));
	Buffer = NULL;

	wcscpy_s(IdxFilename, MAX_PATH, Path);
	wcscat_s(IdxFilename, MAX_PATH, Filename);

	if (_KeyOffset==0)
	{
		_KeyOffset = _ElementSize;
		_ElementSize += LFKeySize;
	}
	KeyOffset = _KeyOffset;
	RequestedElementSize = _ElementSize;
	ItemCount = 0;

	hFile = CreateFile(IdxFilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile==INVALID_HANDLE_VALUE)
	{
		OpenStatus = HeapNoAccess;
	}
	else
	{
		LARGE_INTEGER size;
		size.QuadPart = 0;
		if (!GetFileSizeEx(hFile, &size))
		{
			OpenStatus = HeapError;
			goto Finish;
		}

		if (size.QuadPart<sizeof(HeapfileHeader))
		{
Create:
			strcpy_s(Hdr.ID, sizeof(Hdr.ID), HeapSignature);
			Hdr.ElementSize = _ElementSize;
			Hdr.Version = CurIdxVersion;
			HeaderNeedsWriteback = TRUE;

			if (!WriteHeader())
			{
				OpenStatus = HeapCannotCreate;
				goto Finish;
			}

			SetEndOfFile(hFile);
			OpenStatus = HeapCreated;
		}
		else
		{
			DWORD Read;
			if (!ReadFile(hFile, &Hdr, sizeof(HeapfileHeader), &Read, NULL))
			{
				OpenStatus = HeapError;
				goto Finish;
			}
			if (sizeof(HeapfileHeader)!=Read)
				goto Create;
			if (strcmp(Hdr.ID, HeapSignature)!=0)
				goto Create;

			ItemCount = (UINT)((size.QuadPart-sizeof(HeapfileHeader))/Hdr.ElementSize);
			OpenStatus = (Hdr.Version<CurIdxVersion) ? HeapMaintenanceRequired : HeapOk;

			// Anpassungen für andere Index-Versionen und Tupelgrößen
			if (Hdr.ElementSize>_ElementSize)
			{
				if (KeyOffset==_ElementSize-LFKeySize)
					KeyOffset = Hdr.ElementSize-LFKeySize;
			}
			else
				if (Hdr.ElementSize<_ElementSize)
				{
					if (KeyOffset==_ElementSize-LFKeySize)
						KeyOffset = Hdr.ElementSize-LFKeySize;

					OpenStatus = HeapMaintenanceRequired;
				}
		}

		Compress(hFile, IdxFilename);

		AllocBuffer();
	}

Finish:
	BufferNeedsWriteback = HeaderNeedsWriteback = FALSE;
}

CHeapfile::~CHeapfile()
{
	CloseFile();
	if (Buffer)
		free(Buffer);
}

void CHeapfile::GetAttribute(void* PtrDst, UINT offset, UINT Attr, LFItemDescriptor* i)
{
	assert(PtrDst);
	assert(Attr<LFAttributeCount);

	if (i->AttributeValues[Attr])
	{
		size_t sz = GetAttributeSize(Attr, i->AttributeValues[Attr]);

		UINT EndOfTuple = Hdr.ElementSize;
		if (KeyOffset==Hdr.ElementSize-LFKeySize)
			EndOfTuple -= LFKeySize;

		if (offset+sz<=EndOfTuple)
		{
			CHAR* P = (CHAR*)PtrDst+offset;
			memcpy(P, i->AttributeValues[Attr], sz);
		}
	}
}

__forceinline void CHeapfile::AllocBuffer()
{
	assert(Hdr.ElementSize);

	if (Buffer)
		free(Buffer);

	BufferSize = MaxBufferSize/Hdr.ElementSize;
	if (BufferSize<2)
		BufferSize = 2;
	Buffer = malloc(BufferSize*Hdr.ElementSize);

	FirstInBuffer = LastInBuffer = -1;
}

__forceinline BOOL CHeapfile::OpenFile()
{
	if (hFile==INVALID_HANDLE_VALUE)
		hFile = CreateFile(IdxFilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	return (hFile!=INVALID_HANDLE_VALUE);
}

void CHeapfile::CloseFile()
{
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		WriteHeader();
		Writeback();
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
}

BOOL CHeapfile::WriteHeader()
{
	if (!HeaderNeedsWriteback)
		return TRUE;

	if (!OpenFile())
		return FALSE;

	if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN)==INVALID_SET_FILE_POINTER)
		return FALSE;

	DWORD Written;
	if (!WriteFile(hFile, &Hdr, sizeof(HeapfileHeader), &Written, NULL))
		return FALSE;
	if (sizeof(HeapfileHeader)!=Written)
		return FALSE;

	HeaderNeedsWriteback = FALSE;
	return TRUE;
}

BOOL CHeapfile::Writeback()
{
	if (!BufferNeedsWriteback)
		return TRUE;

	if (!OpenFile())
		return FALSE;

	if (SetFilePointer(hFile, sizeof(HeapfileHeader)+FirstInBuffer*Hdr.ElementSize, NULL, FILE_BEGIN)==INVALID_SET_FILE_POINTER)
		return FALSE;

	DWORD Written;
	DWORD Size = (LastInBuffer-FirstInBuffer+1)*Hdr.ElementSize;
	if (!WriteFile(hFile, Buffer, Size, &Written, NULL))
		return FALSE;
	if (Size!=Written)
		return FALSE;

	BufferNeedsWriteback = FALSE;
	return TRUE;
}

__forceinline void CHeapfile::ElementToBuffer(INT ID)
{
	if (((ID>=FirstInBuffer) && (ID<=LastInBuffer)) || (ID>=ItemCount))
		return;

	Writeback();
	if (!OpenFile())
		return;

	if (SetFilePointer(hFile, sizeof(HeapfileHeader)+ID*Hdr.ElementSize, NULL, FILE_BEGIN)==INVALID_SET_FILE_POINTER)
		return;

	DWORD Read;
	if (!ReadFile(hFile, Buffer, BufferSize*Hdr.ElementSize, &Read, NULL))
		return;

	FirstInBuffer = ID;
	LastInBuffer = ID+(Read/Hdr.ElementSize)-1;
}

BOOL CHeapfile::FindNext(INT& Next, void*& Ptr)
{
	CHAR* P;

	do
	{
		if (Next>=ItemCount)
			return FALSE;

		ElementToBuffer(Next);
		P = (CHAR*)Buffer+(Next-FirstInBuffer)*Hdr.ElementSize+KeyOffset;
		Next++;
	}
	while (*P=='\0');

	Ptr = P-KeyOffset;
	return TRUE;
}

BOOL CHeapfile::FindKey(CHAR* Key, INT& Next, void*& Ptr)
{
	CHAR* P;

	do
	{
		if (Next>=ItemCount)
			return FALSE;

		ElementToBuffer(Next);
		P = (CHAR*)Buffer+(Next-FirstInBuffer)*Hdr.ElementSize+KeyOffset;
		Next++;
	}
	while (strcmp(P, Key)!=0);

	Ptr = P-KeyOffset;
	return TRUE;
}

void CHeapfile::Add(LFItemDescriptor* i)
{
	assert(i);

	if (FirstInBuffer==-1)
	{
		// Puffer unbenutzt
		FirstInBuffer = ItemCount;
		LastInBuffer = ItemCount;
	}
	else
		if ((LastInBuffer-FirstInBuffer+1<(INT)BufferSize) && (LastInBuffer==ItemCount-1))
		{
			// Noch Platz am Ende
			LastInBuffer = ItemCount;
		}
		else
		{
			Writeback();
			FirstInBuffer = ItemCount;
			LastInBuffer = ItemCount;
		}

	// Im RAM hinzufügen
	CHAR* P = (CHAR*)Buffer+(ItemCount-FirstInBuffer)*Hdr.ElementSize;
	if (KeyOffset==Hdr.ElementSize-LFKeySize)
	{
		GetFromItemDescriptor(P, i);
		P += Hdr.ElementSize-LFKeySize;
		memcpy(P, i->CoreAttributes.FileID, LFKeySize);
	}
	else
	{
		GetFromItemDescriptor(P, i);
	}

	MakeDirty();
	ItemCount++;
}

void CHeapfile::Update(LFItemDescriptor* i, void* Ptr)
{
	GetFromItemDescriptor(Ptr, i);
	MakeDirty();
}

void CHeapfile::Update(LFItemDescriptor* i, INT& Next)
{
	assert(i);

	void* Ptr;

	if (FindKey(i->CoreAttributes.FileID, Next, Ptr))
		Update(i, Ptr);
}

void CHeapfile::Update(LFItemDescriptor* i)
{
	assert(i);

	INT ID = 0;
	Update(i, ID);
}

void CHeapfile::Invalidate(void* Ptr)
{
	*((CHAR*)Ptr+KeyOffset) = 0;
	MakeDirty(TRUE);
}

void CHeapfile::Invalidate(CHAR* Key, INT& Next)
{
	assert(Key);

	void* Ptr;

	if (FindKey(Key, Next, Ptr))
		Invalidate(Ptr);
}

void CHeapfile::Invalidate(LFItemDescriptor* i)
{
	assert(i);

	INT ID = 0;
	Invalidate(i->CoreAttributes.FileID, ID);
}

UINT CHeapfile::GetItemCount()
{
	return ItemCount;
}

UINT CHeapfile::GetRequiredElementSize()
{
	return max(Hdr.ElementSize, RequestedElementSize);
}

UINT CHeapfile::GetRequiredDiscSize()
{
	return GetRequiredElementSize()*ItemCount+sizeof(HeapfileHeader);
}

BOOL CHeapfile::Compact()
{
	if ((!Hdr.NeedsCompaction) && (OpenStatus!=HeapMaintenanceRequired))
		return TRUE;

	WCHAR BufFilename[MAX_PATH];
	wcscpy_s(BufFilename, MAX_PATH, IdxFilename);
	wcscat_s(BufFilename, MAX_PATH, L".part");

	HANDLE hOutput = CreateFile(BufFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hOutput==INVALID_HANDLE_VALUE)
		return FALSE;

	Compress(hOutput, BufFilename);

	HeapfileHeader NewHdr = Hdr;
	NewHdr.ElementSize = max(Hdr.ElementSize, RequestedElementSize);
	NewHdr.NeedsCompaction = FALSE;
	NewHdr.Version = CurIdxVersion;

	#define ABORT { CloseHandle(hOutput); DeleteFile(BufFilename); return FALSE; }

	DWORD Written;
	if (!WriteFile(hOutput, &NewHdr, sizeof(HeapfileHeader), &Written, NULL))
		ABORT
	if (sizeof(HeapfileHeader)!=Written)
		ABORT

	INT Next = 0;
	INT Count = 0;
	void* Ptr;
	BOOL Result = TRUE;
	CHAR* tmpBuf = (CHAR*)malloc(NewHdr.ElementSize);
	ZeroMemory(tmpBuf, NewHdr.ElementSize);

	while (Result)
	{
		Result = FindNext(Next, Ptr);

		if (Result)
		{
			if ((NewHdr.ElementSize>Hdr.ElementSize) && (KeyOffset==Hdr.ElementSize-LFKeySize))
			{
				memcpy_s(tmpBuf, NewHdr.ElementSize, Ptr, Hdr.ElementSize-LFKeySize);

				CHAR* P = (CHAR*)Ptr+Hdr.ElementSize-LFKeySize;
				memcpy(tmpBuf+NewHdr.ElementSize-LFKeySize, P, LFKeySize);
			}
			else
			{
				memcpy_s(tmpBuf, NewHdr.ElementSize, Ptr, Hdr.ElementSize);
			}

			if (!WriteFile(hOutput, tmpBuf, NewHdr.ElementSize, &Written, NULL))
				ABORT
			if (NewHdr.ElementSize!=Written)
				ABORT

			Count++;
		}
	};

	free(tmpBuf);
	HeaderNeedsWriteback = FALSE;
	CloseHandle(hOutput);
	CloseFile();

	if (!DeleteFile(IdxFilename))
		return FALSE;
	if (!MoveFile(BufFilename, IdxFilename))
		return FALSE;

	if (KeyOffset==Hdr.ElementSize-LFKeySize)
		KeyOffset = NewHdr.ElementSize-LFKeySize;

	OpenStatus = HeapOk;
	Hdr = NewHdr;
	AllocBuffer();
	FirstInBuffer = LastInBuffer = -1;
	ItemCount = Count;
	return TRUE;
}

void CHeapfile::MakeDirty(BOOL NeedsCompaction)
{
	BufferNeedsWriteback = TRUE;
	Hdr.NeedsCompaction |= NeedsCompaction;
	HeaderNeedsWriteback |= NeedsCompaction;
}
