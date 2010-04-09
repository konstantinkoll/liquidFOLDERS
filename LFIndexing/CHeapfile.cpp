
#pragma once
#include "stdafx.h"
#include "LFIndexing.h"
#include "liquidFOLDERS.h"
#include <io.h>
#include <malloc.h>


CHeapfile::CHeapfile(char* _IdxFilename, unsigned int _ElementSize, unsigned int _KeyOffset)
{
	strcpy_s(IdxFilename, MAX_PATH, _IdxFilename);
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
	if (GetFileAttributesExA(_IdxFilename, GetFileExInfoStandard, &fileInfo))
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
	Compact();
	CloseFile();
	if (Buffer)
		free(Buffer);
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

void CHeapfile::Add(char* Key, void* Ptr)
{
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
		memcpy(P, Ptr, ElementSize-LFKeySize);
		P += ElementSize-LFKeySize;
		memcpy(P, Key, LFKeySize);
	}
	else
	{
		memcpy(P, Ptr, ElementSize);
	}

	MakeDirty();
	FileCount++;
}

void CHeapfile::Update(char* Key, void* Ptr)
{
	int ID = 0;
	void* P;

	if (FindKey(Key, ID, P))
	{
		memcpy(P, Ptr, KeyOffset==ElementSize-LFKeySize ? ElementSize-LFKeySize : ElementSize);
		MakeDirty();
	}
}

void CHeapfile::Invalidate(void* Ptr)
{
	*((char*)Ptr+KeyOffset) = 0;
	MakeDirty(true);
}

void CHeapfile::InvalidateKey(char* Key, int& Next)
{
	char* P;

	do
	{
		if (Next>=FileCount)
			return;

		ElementToBuffer(Next);
		P = (char*)Buffer+(Next-FirstInBuffer)*ElementSize+KeyOffset;
		Next++;
	}
	while (strcmp(P, Key)!=0);

	*P = 0;
	MakeDirty(true);
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
	void* P;
	bool res = true;

	#define ABORT { CloseHandle(hOutput); DeleteFileA(BufFilename); return false; }

	while (res)
	{
		res = FindNext(Next, P);

		if (res)
		{
			DWORD Written;
			if (!WriteFile(hOutput, P, ElementSize, &Written, NULL))
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
