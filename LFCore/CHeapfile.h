
#pragma once
#include "LFCore.h"
#include "LFItemDescriptor.h"


// CHeapfile
//

#define MaxBufferSize                  65536
#define HeapSignature                  "LFIDX"

#define HeapOk                         0
#define HeapError                      1
#define HeapCreated                    2
#define HeapCannotCreate               3
#define HeapMaintenanceRecommended     4
#define HeapMaintenanceRequired        5

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
public:
	CHeapfile(char* Path, char* Filename, unsigned int _ElementSize, unsigned int _KeyOffset=0);
	~CHeapfile();

	virtual void GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i);
	virtual void WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc);

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
	bool Compact();
	void MakeDirty(bool NeedsCompaction=false);

	unsigned int OpenStatus;
	unsigned int RequestedElementSize;

protected:
	void* Buffer;
	HeapfileHeader Hdr;
	int FirstInBuffer;
	int LastInBuffer;
	unsigned int KeyOffset;
	unsigned int BufferSize;
	int ItemCount;
	bool BufferNeedsWriteback;
	bool HeaderNeedsWriteback;

	void AllocBuffer();
	bool OpenFile();
	bool WriteHeader();
	bool Writeback();
	void ElementToBuffer(int ID);

private:
	char IdxFilename[MAX_PATH];
	HANDLE hFile;
};
