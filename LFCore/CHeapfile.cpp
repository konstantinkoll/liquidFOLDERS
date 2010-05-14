
#pragma once
#include "stdafx.h"
#include "CHeapfile.h"
#include "IdxTables.h"
#include <assert.h>
#include <io.h>
#include <malloc.h>


CHeapfile::CHeapfile(char* Path, char* Filename, unsigned int _ElementSize, unsigned int _KeyOffset)
{
	assert(sizeof(HeapfileHeader)==512);
	assert(_ElementSize);
	ZeroMemory(&Hdr, sizeof(HeapfileHeader));
	Buffer = NULL;

	strcpy_s(IdxFilename, MAX_PATH, Path);
	strcat_s(IdxFilename, MAX_PATH, Filename);

	if (_KeyOffset==0)
	{
		_KeyOffset = _ElementSize;
		_ElementSize += LFKeySize;
	}
	KeyOffset = _KeyOffset;
	RequestedElementSize = _ElementSize;
	ItemCount = 0;

	hFile = CreateFileA(IdxFilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile==INVALID_HANDLE_VALUE)
	{
		OpenStatus = HeapError;
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
			HeaderNeedsWriteback = true;

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

			ItemCount = (unsigned int)((size.QuadPart-sizeof(HeapfileHeader))/Hdr.ElementSize);
			OpenStatus = (Hdr.Version<CurIdxVersion) ? HeapMaintenanceRecommended : HeapOk;

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

		AllocBuffer();
	}

Finish:
	BufferNeedsWriteback = HeaderNeedsWriteback = false;
}

CHeapfile::~CHeapfile()
{
	CloseFile();
	if (Buffer)
		free(Buffer);
}

void CHeapfile::GetAttribute(void* PtrDst, unsigned int offset, unsigned int attr, LFItemDescriptor* i)
{
	assert(PtrDst);
	assert(attr<LFAttributeCount);

	size_t sz = GetAttributeSize(attr, i->AttributeValues[attr]);

	unsigned int EndOfTuple = Hdr.ElementSize;
	if (KeyOffset==Hdr.ElementSize-LFKeySize)
		EndOfTuple -= LFKeySize;

	if (offset+sz<=EndOfTuple)
	{
		char* P = (char*)PtrDst+offset;

		if (i->AttributeValues[attr])
		{
			memcpy_s(P, sz, i->AttributeValues[attr], sz);
		}
		else
		{
			ZeroMemory(P, sz);
		}
	}
}

inline void CHeapfile::AllocBuffer()
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

inline bool CHeapfile::OpenFile()
{
	if (hFile==INVALID_HANDLE_VALUE)
		hFile = CreateFileA(IdxFilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

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

bool CHeapfile::WriteHeader()
{
	if (!HeaderNeedsWriteback)
		return true;

	if (!OpenFile())
		return false;

	if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN)==INVALID_SET_FILE_POINTER)
		return false;

	DWORD Written;
	if (!WriteFile(hFile, &Hdr, sizeof(HeapfileHeader), &Written, NULL))
		return false;
	if (sizeof(HeapfileHeader)!=Written)
		return false;

	HeaderNeedsWriteback = false;
	return true;
}

bool CHeapfile::Writeback()
{
	if (!BufferNeedsWriteback)
		return true;

	if (!OpenFile())
		return false;

	if (SetFilePointer(hFile, sizeof(HeapfileHeader)+FirstInBuffer*Hdr.ElementSize, NULL, FILE_BEGIN)==INVALID_SET_FILE_POINTER)
		return false;

	DWORD Written;
	DWORD Size = (LastInBuffer-FirstInBuffer+1)*Hdr.ElementSize;
	if (!WriteFile(hFile, Buffer, Size, &Written, NULL))
		return false;
	if (Size!=Written)
		return false;

	BufferNeedsWriteback = false;
	return true;
}

inline void CHeapfile::ElementToBuffer(int ID)
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

bool CHeapfile::FindNext(int& Next, void*& Ptr)
{
	char* P;

	do
	{
		if (Next>=ItemCount)
			return false;

		ElementToBuffer(Next);
		P = (char*)Buffer+(Next-FirstInBuffer)*Hdr.ElementSize+KeyOffset;
		Next++;
	}
	while (*P==0);

	Ptr = P-KeyOffset;
	return true;
}

bool CHeapfile::FindKey(char* Key, int& Next, void*& Ptr)
{
	char* P;

	do
	{
		if (Next>=ItemCount)
			return false;

		ElementToBuffer(Next);
		P = (char*)Buffer+(Next-FirstInBuffer)*Hdr.ElementSize+KeyOffset;
		Next++;
	}
	while (strcmp(P, Key)!=0);

	Ptr = P-KeyOffset;
	return true;
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
		if ((LastInBuffer-FirstInBuffer+1<(int)BufferSize) && (LastInBuffer==ItemCount-1))
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
	char* P = (char*)Buffer+(ItemCount-FirstInBuffer)*Hdr.ElementSize;
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

void CHeapfile::Update(LFItemDescriptor* i, int& Next)
{
	assert(i);

	void* Ptr;

	if (FindKey(i->CoreAttributes.FileID, Next, Ptr))
		Update(i, Ptr);
}


void CHeapfile::Update(LFItemDescriptor* i)
{
	assert(i);

	int ID = 0;
	Update(i, ID);
}

void CHeapfile::Invalidate(void* Ptr)
{
	*((char*)Ptr+KeyOffset) = 0;
	MakeDirty(true);
}

void CHeapfile::Invalidate(char* Key, int& Next)
{
	assert(Key);

	void* Ptr;

	if (FindKey(Key, Next, Ptr))
		Invalidate(Ptr);
}

void CHeapfile::Invalidate(LFItemDescriptor* i)
{
	assert(i);

	int ID = 0;
	Invalidate(i->CoreAttributes.FileID, ID);
}

bool CHeapfile::Compact()
{
	if ((!Hdr.NeedsCompaction) && (OpenStatus!=HeapMaintenanceRequired) && (OpenStatus!=HeapMaintenanceRecommended))
		return true;

	char BufFilename[MAX_PATH];
	strcpy_s(BufFilename, MAX_PATH, IdxFilename);
	strcat_s(BufFilename, MAX_PATH, ".part");

	HANDLE hOutput = CreateFileA(BufFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hOutput==INVALID_HANDLE_VALUE)
		return false;

	HeapfileHeader NewHdr = Hdr;
	NewHdr.ElementSize = min(Hdr.ElementSize, RequestedElementSize);
	NewHdr.NeedsCompaction = false;
	NewHdr.Version = CurIdxVersion;

	#define ABORT { CloseHandle(hOutput); DeleteFileA(BufFilename); return false; }

	DWORD Written;
	if (!WriteFile(hOutput, &NewHdr, sizeof(HeapfileHeader), &Written, NULL))
		ABORT
	if (sizeof(HeapfileHeader)!=Written)
		ABORT

	int Next = 0;
	void* Ptr;
	bool res = true;
	char* tmpBuf = (char*)malloc(NewHdr.ElementSize);
	ZeroMemory(tmpBuf, NewHdr.ElementSize);

	while (res)
	{
		res = FindNext(Next, Ptr);

		if (res)
		{
			if ((NewHdr.ElementSize>Hdr.ElementSize) && (KeyOffset==Hdr.ElementSize-LFKeySize))
			{
				memcpy_s(tmpBuf, NewHdr.ElementSize, Ptr, Hdr.ElementSize-LFKeySize);

				char* P = (char*)Ptr+Hdr.ElementSize-LFKeySize;
				memcpy_s(tmpBuf+NewHdr.ElementSize-LFKeySize, LFKeySize, P, LFKeySize);
			}
			else
			{
				memcpy_s(tmpBuf, NewHdr.ElementSize, Ptr, Hdr.ElementSize);
			}

			if (!WriteFile(hOutput, tmpBuf, NewHdr.ElementSize, &Written, NULL))
				ABORT
			if (NewHdr.ElementSize!=Written)
				ABORT
		}
	};

	free(tmpBuf);
	HeaderNeedsWriteback = false;
	CloseHandle(hOutput);
	CloseFile();

	if (!DeleteFileA(IdxFilename))
		return false;
	if (!MoveFileA(BufFilename, IdxFilename))
		return false;

	if (KeyOffset==Hdr.ElementSize-LFKeySize)
		KeyOffset = NewHdr.ElementSize-LFKeySize;

	OpenStatus = HeapOk;
	Hdr = NewHdr;
	AllocBuffer();
	FirstInBuffer = LastInBuffer = -1;
	return true;
}

void CHeapfile::MakeDirty(bool _NeedsCompaction)
{
	BufferNeedsWriteback = true;
	Hdr.NeedsCompaction = _NeedsCompaction;
	HeaderNeedsWriteback |= _NeedsCompaction;
}
