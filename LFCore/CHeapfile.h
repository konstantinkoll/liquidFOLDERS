
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
#define HeapNoAccess                3	// Fatal error condition
#define HeapError                   4	// Fatal error condition
#define HeapCannotCreate            5	// Fatal error condition

struct HeapfileHeader
{
	CHAR ID[6];
	UINT ElementSize;					// Includes StoreDataSize
	UINT Version;
	BOOL NeedsCompaction;
	UINT StoreDataSize;					// Part of ElementSize
	BYTE Fill[488];						// Pad to 512 byte
};

class CHeapfile
{
public:
	CHeapfile(WCHAR* Path, UINT TableID, UINT StoreDataSize=0);
	~CHeapfile();

	UINT GetItemCount() const;
	UINT GetRequiredElementSize() const;
	UINT64 GetRequiredFileSize() const;
	void* GetStoreData(void* Ptr) const;

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
	UINT m_TableID;
	UINT m_RequiredElementSize;
	UINT m_StoreDataSize;
	UINT m_KeyOffset;

	void* m_pBuffer;
	UINT m_ItemCount;
	UINT m_BufferCount;
	INT m_FirstInBuffer;
	INT m_LastInBuffer;
	BOOL m_HeaderNeedsWriteback;
	BOOL m_BufferNeedsWriteback;

private:
	static void ZeroCopy(void* pDst, const SIZE_T DstSize, void* pSrc, const SIZE_T SrcSize);
	void GetAttribute(void* PtrDst, INT_PTR Offset, UINT Attr, LFItemDescriptor* pItemDescriptor) const;

	WCHAR m_Filename[MAX_PATH];
	HANDLE hFile;
};

inline UINT CHeapfile::GetItemCount() const
{
	return m_ItemCount;
}
