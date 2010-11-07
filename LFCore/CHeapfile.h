
#pragma once
#include "LFCore.h"
#include "LFItemDescriptor.h"


// CHeapfile
//

#define MaxBufferSize                  65536
#define HeapSignature                  "LFIDX"

#define HeapOk                         0
#define HeapNoAccess                   1
#define HeapError                      2
#define HeapCreated                    3
#define HeapCannotCreate               4
#define HeapMaintenanceRecommended     5
#define HeapMaintenanceRequired        6

struct HeapfileHeader
{
	char ID[6];
	unsigned int ElementSize;
	unsigned int Version;
	bool NeedsCompaction;
	unsigned char Fill[493];		// Auf 512 Byte
};

class LFCore_API CHeapfile
{
	friend class CIndex;

public:
	CHeapfile(wchar_t* Path, wchar_t* Filename, unsigned int _ElementSize, unsigned int _KeyOffset=0);
	virtual ~CHeapfile();

	virtual void GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i) = 0;
	virtual void WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc) = 0;

	void GetAttribute(void* PtrDst, unsigned int offset, unsigned int attr, LFItemDescriptor* i);
	void CloseFile();
	bool FindNext(int& Next, void*& Ptr);
	bool FindKey(char* Key, int& Next, void*& Ptr);
	void Add(LFItemDescriptor* i);
	void Update(LFItemDescriptor* i, void* Ptr);
	void Update(LFItemDescriptor* i, int& Next);
	void Update(LFItemDescriptor* i);
	void Invalidate(void* Ptr);
	void Invalidate(char* Key, int& Next);
	void Invalidate(LFItemDescriptor* i);
	unsigned int GetItemCount();
	unsigned int GetRequiredElementSize();
	unsigned int GetRequiredDiscSize();
	bool Compact();
	void MakeDirty(bool NeedsCompaction=false);

	unsigned int OpenStatus;

protected:
	void* Buffer;
	HeapfileHeader Hdr;
	unsigned int RequestedElementSize;
	unsigned int KeyOffset;
	unsigned int BufferSize;
	int ItemCount;
	int FirstInBuffer;
	int LastInBuffer;
	bool BufferNeedsWriteback;
	bool HeaderNeedsWriteback;

	void AllocBuffer();
	bool OpenFile();
	bool WriteHeader();
	bool Writeback();
	void ElementToBuffer(int ID);

private:
	wchar_t IdxFilename[MAX_PATH];
	HANDLE hFile;
};
