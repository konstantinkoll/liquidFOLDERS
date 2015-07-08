
#pragma once
#include "LFCore.h"
#include "LFItemDescriptor.h"


// CHeapfile
//

#define MaxBufferSize          262144
#define HeapSignature         "LFIDX"

#define HeapOk                      0
#define HeapNoAccess                1
#define HeapError                   2
#define HeapCreated                 3
#define HeapCannotCreate            4
#define HeapMaintenanceRequired     5

struct HeapfileHeader
{
	CHAR ID[6];
	UINT ElementSize;
	UINT Version;
	BOOL NeedsCompaction;
	BYTE Fill[492];			// Auf 512 Byte
};

class CHeapfile
{
public:
	CHeapfile(WCHAR* Path, BYTE TableID);
	~CHeapfile();

	void GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i);

	void CloseFile();
	BOOL FindNext(INT& Next, void*& Ptr);
	BOOL FindKey(CHAR* Key, INT& Next, void*& Ptr);
	void Add(LFItemDescriptor* i);
	void Update(LFItemDescriptor* i, void* Ptr);
	void Update(LFItemDescriptor* i, INT& Next);
	void Update(LFItemDescriptor* i);
	void Invalidate(void* Ptr);
	void Invalidate(CHAR* Key, INT& Next);
	void Invalidate(LFItemDescriptor* i);
	UINT GetItemCount();
	UINT GetElementSize();
	UINT GetRequiredElementSize();
	UINT GetRequiredDiscSize();
	BOOL Compact();
	void MakeDirty(BOOL NeedsCompaction=FALSE);

	UINT OpenStatus;

protected:
	BYTE m_TableID;
	void* Buffer;
	HeapfileHeader Hdr;
	UINT RequestedElementSize;
	UINT KeyOffset;
	UINT BufferSize;
	INT ItemCount;
	INT FirstInBuffer;
	INT LastInBuffer;
	BOOL BufferNeedsWriteback;
	BOOL HeaderNeedsWriteback;

	void AllocBuffer();
	BOOL OpenFile();
	BOOL WriteHeader();
	BOOL Writeback();
	void ElementToBuffer(INT ID);

private:
	void GetAttribute(void* PtrDst, UINT offset, UINT Attr, LFItemDescriptor* i);

	WCHAR IdxFilename[MAX_PATH];
	HANDLE hFile;
};
