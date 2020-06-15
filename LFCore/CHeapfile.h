
#pragma once
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "Mutex.h"


// CHeapfile
//

#define MaxBufferSize     524288
#define HeapSignature     "LFIDX"

// Magic values
#define HeapOk                      FileOk					// 0
#define HeapSharingViolation        FileSharingViolation	// 1, Fatal error condition
#define HeapNoAccess                FileNoAccess			// 2, Fatal error condition

// Additional error conditions
#define HeapCreated                 3
#define HeapMaintenanceRequired     4
#define HeapError                   5						// Fatal error condition
#define HeapCannotCreate            6						// Fatal error condition

struct HeapfileHeader
{
	CHAR ID[6];
	UINT ElementSize;		// Includes StoreDataSize
	UINT Version;
	BOOL NeedsCompaction;
	UINT StoreDataSize;		// Part of ElementSize
	BYTE Fill[488];			// Pad to 512 byte
};

class CHeapfile sealed
{
public:
	CHeapfile(LPCWSTR Path, UINT TableID, UINT StoreDataSize, BOOL Initialize);
	~CHeapfile();

	UINT GetVersion() const;
	UINT_PTR GetItemCount() const;
	UINT GetElementSize() const;
	UINT GetRequiredElementSize() const;
	UINT64 GetRequiredFileSize() const;
	LPVOID GetStoreData(LPCVOID pData) const;
	UINT GetError(BOOL SingleStore=FALSE);

	void MakeDirty(BOOL NeedsCompaction=FALSE);
	BOOL FindNext(INT_PTR& Next, LPVOID& Ptr);
	BOOL FindKey(const FILEID& FileID, INT_PTR& Next, LPVOID& Ptr);
	void Add(LFItemDescriptor* pItemDescriptor);
	void Update(LFItemDescriptor* pItemDescriptor, LPVOID Ptr);
	void Update(LFItemDescriptor* pItemDescriptor, INT_PTR& Next);
	void Update(LFItemDescriptor* pItemDescriptor);
	void Invalidate(LPVOID Ptr);
	void Invalidate(const FILEID& FileID, INT_PTR& Next);
	void Invalidate(LFItemDescriptor* pItemDescriptor);
	void GetFromItemDescriptor(LPVOID Ptr, LFItemDescriptor* pItemDescriptor);
	BOOL Compact();

	UINT m_OpenStatus;

protected:
	void AllocBuffer();
	void CloseFile();
	void ElementToBuffer(const INT_PTR ID);
	BOOL WriteHeader();
	void Flush();

	HeapfileHeader m_Header;
	UINT m_TableID;
	UINT m_RequiredElementSize;
	UINT m_StoreDataSize;
	UINT m_KeyOffset;

	LPBYTE m_pBuffer;
	UINT_PTR m_ItemCount;
	UINT m_BufferCount;
	INT_PTR m_FirstInBuffer;
	INT_PTR m_LastInBuffer;
	BOOL m_HeaderNeedsWriteback;
	BOOL m_BufferNeedsWriteback;

private:
	LPCFILEID GetFileID(INT_PTR ID) const;
	static void ZeroCopy(LPVOID pDst, const SIZE_T DstSize, LPCVOID pSrc, const SIZE_T SrcSize);
	void GetAttribute(LPVOID Ptr, SIZE_T Offset, ATTRIBUTE Attr, LFItemDescriptor* pItemDescriptor) const;

	WCHAR m_Path[MAX_PATH];
	HANDLE hFile;
};

inline UINT CHeapfile::GetVersion() const
{
	return m_Header.Version;
}

inline UINT_PTR CHeapfile::GetItemCount() const
{
	return m_ItemCount;
}

inline UINT CHeapfile::GetElementSize() const
{
	return m_Header.ElementSize;
}

inline UINT CHeapfile::GetRequiredElementSize() const
{
	return max(m_Header.ElementSize, m_RequiredElementSize);
}

inline UINT64 CHeapfile::GetRequiredFileSize() const
{
	return GetRequiredElementSize()*m_ItemCount+sizeof(HeapfileHeader);
}

inline LPCFILEID CHeapfile::GetFileID(INT_PTR ID) const
{
	return (LPCFILEID)(m_pBuffer+m_KeyOffset+(ID-m_FirstInBuffer)*m_Header.ElementSize);
}

inline void CHeapfile::MakeDirty(BOOL NeedsCompaction)
{
	m_BufferNeedsWriteback = TRUE;

	m_Header.NeedsCompaction |= NeedsCompaction;
	m_HeaderNeedsWriteback |= NeedsCompaction;
}
