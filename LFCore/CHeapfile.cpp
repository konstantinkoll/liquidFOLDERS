
#pragma once
#include "stdafx.h"
#include "CHeapfile.h"
#include <assert.h>
#include <io.h>
#include <malloc.h>


// TODO: Header, NeedsCompaction in den Header speichern
CHeapfile::CHeapfile(char* Path, char* Filename, unsigned int _ElementSize, unsigned int _KeyOffset)
{
	strcpy_s(IdxFilename, MAX_PATH, Path);
	strcat_s(IdxFilename, MAX_PATH, Filename);
	if (_KeyOffset==0)
	{
		_KeyOffset = _ElementSize;
		_ElementSize += LFKeySize;
	}
	ElementSize = _ElementSize;
	KeyOffset = _KeyOffset;
	FileCount = 0;
	hFile = INVALID_HANDLE_VALUE;

	WIN32_FILE_ATTRIBUTE_DATA fileInfo;
	if (GetFileAttributesExA(IdxFilename, GetFileExInfoStandard, &fileInfo))
	{
		__int64 size = fileInfo.nFileSizeLow;
		size <<= 32;
		size |= fileInfo.nFileSizeLow;
		FileCount = (unsigned int)(size/ElementSize);
	}

	BufferSize = MaxBufferSize/ElementSize;
	if (BufferSize<2)
		BufferSize = 2;
	Buffer = malloc(BufferSize*ElementSize);

	FirstInBuffer = LastInBuffer = -1;
	BufferNeedsWriteback = NeedsCompaction = false;
}

CHeapfile::~CHeapfile()
{
	CloseFile();
	if (Buffer)
		free(Buffer);
}

void CHeapfile::GetFromItemDescriptor(void* /*PtrDst*/, LFItemDescriptor* /*f*/)
{
	assert(false);
}

void CHeapfile::WriteToItemDescriptor(LFItemDescriptor* /*f*/, void* /*PtrSrc*/)
{
	assert(false);
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
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
}

bool CHeapfile::Writeback()
{
	if (!BufferNeedsWriteback)
		return true;

	if (!OpenFile())
		return false;

	if (SetFilePointer(hFile, FirstInBuffer*ElementSize, NULL, FILE_BEGIN)==INVALID_SET_FILE_POINTER)
		return false;

	DWORD Written;
	DWORD Size = (LastInBuffer-FirstInBuffer+1)*ElementSize;
	if (!WriteFile(hFile, Buffer, Size, &Written, NULL))
		return false;
	if (Size!=Written)
		return false;

	BufferNeedsWriteback = false;
	return true;
}

inline void CHeapfile::ElementToBuffer(int ID)
{
	if (((ID>=FirstInBuffer) && (ID<=LastInBuffer)) || (ID>=FileCount))
		return;

	Writeback();
	if (!OpenFile())
		return;

	if (SetFilePointer(hFile, ID*ElementSize, NULL, FILE_BEGIN)==INVALID_SET_FILE_POINTER)
		return;

	DWORD Read;
	if (!ReadFile(hFile, Buffer, BufferSize*ElementSize, &Read, NULL))
		return;

	FirstInBuffer = ID;
	LastInBuffer = ID+(Read/ElementSize)-1;
}

bool CHeapfile::FindNext(int& Next, void*& Ptr)
{
	char* P;

	do
	{
		if (Next>=FileCount)
			return false;

		ElementToBuffer(Next);
		P = (char*)Buffer+(Next-FirstInBuffer)*ElementSize+KeyOffset;
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
		if (Next>=FileCount)
			return false;

		ElementToBuffer(Next);
		P = (char*)Buffer+(Next-FirstInBuffer)*ElementSize+KeyOffset;
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
		FirstInBuffer = FileCount;
		LastInBuffer = FileCount;
	}
	else
		if ((LastInBuffer-FirstInBuffer+1<(int)BufferSize) && (LastInBuffer==FileCount-1))
		{
			// Noch Platz am Ende
			LastInBuffer = FileCount;
		}
		else
		{
			Writeback();
			FirstInBuffer = FileCount;
			LastInBuffer = FileCount;
		}

	// Im RAM hinzufügen
	char* P = (char*)Buffer+(FileCount-FirstInBuffer)*ElementSize;
	if (KeyOffset==ElementSize-LFKeySize)
	{
		GetFromItemDescriptor(P, i);
		P += ElementSize-LFKeySize;
		memcpy(P, i->CoreAttributes.FileID, LFKeySize);
	}
	else
	{
		GetFromItemDescriptor(P, i);
	}

	MakeDirty();
	FileCount++;
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
	assert(i);

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
	Writeback();
	if (!NeedsCompaction)
		return true;

	char BufFilename[MAX_PATH];
	strcpy_s(BufFilename, MAX_PATH, IdxFilename);
	strcat_s(BufFilename, MAX_PATH, ".part");

	HANDLE hOutput = CreateFileA(BufFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hOutput==INVALID_HANDLE_VALUE)
		return false;

	int Next = 0;
	void* Ptr;
	bool res = true;

	#define ABORT { CloseHandle(hOutput); DeleteFileA(BufFilename); return false; }

	while (res)
	{
		res = FindNext(Next, Ptr);

		if (res)
		{
			DWORD Written;
			if (!WriteFile(hOutput, Ptr, ElementSize, &Written, NULL))
				ABORT
			if (ElementSize!=Written)
				ABORT
		}
	};

	CloseHandle(hOutput);
	CloseFile();

	if (!DeleteFileA(IdxFilename))
		return false;
	if (!MoveFileA(BufFilename, IdxFilename))
		return false;

	FirstInBuffer = LastInBuffer = -1;
	NeedsCompaction = false;
	return true;
}

void CHeapfile::MakeDirty(bool _NeedsCompaction)
{
	BufferNeedsWriteback = true;
	NeedsCompaction = _NeedsCompaction;
}
