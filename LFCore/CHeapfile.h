
#pragma once
#include "LFCore.h"
#include "LFItemDescriptor.h"


// CHeapfile
//

#define MaxBufferSize     262144
#define HeapSignature     "LFIDX"

#define HeapOk                      0
#define HeapCreated                 1
#define HeapMaintenanceRequired     2
#define HeapNoAccess                3
#define HeapError                   4
#define HeapCannotCreate            5

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

	UINT GetItemCount();
	UINT GetRequiredElementSize();
	UINT GetRequiredFileSize();

	void MakeDirty(BOOL NeedsCompaction=FALSE);
	BOOL FindNext(INT& Next, void*& Ptr);
	BOOL FindKey(CHAR* FileID, INT& Next, void*& Ptr);
	void Add(LFItemDescriptor* pItemDescriptor);
	void Update(LFItemDescriptor* pItemDescriptor, void* Ptr);
	void Update(LFItemDescriptor* pItemDescriptor, INT& Next);
	void Update(LFItemDescriptor* pItemDescriptor);
	void Invalidate(void* Ptr);
	void Invalidate(CHAR* FileID, INT& Next);
	void Invalidate(LFItemDescriptor* pItemDescriptor);
	void GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* pItemDescriptor);
	BOOL Compact();

	UINT m_OpenStatus;

protected:
	void AllocBuffer();
	void CloseFile();
	void ElementToBuffer(INT ID);
	BOOL WriteHeader();
	void Flush();

	HeapfileHeader m_Header;
	BYTE m_TableID;
	UINT m_RequiredElementSize;
	UINT m_KeyOffset;

	void* m_pBuffer;
	UINT m_ItemCount;
	UINT m_BufferCount;
	INT m_FirstInBuffer;
	INT m_LastInBuffer;
	BOOL m_BufferNeedsWriteback;
	BOOL m_HeaderNeedsWriteback;

private:
	void GetAttribute(void* PtrDst, UINT Offset, UINT Attr, LFItemDescriptor* pItemDescriptor);

	WCHAR m_Filename[MAX_PATH];
	HANDLE hFile;
};
