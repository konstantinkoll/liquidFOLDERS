
#pragma once
#include "LFCore.h"


// CHeapfile
//

#define MaxBufferSize     262144

class LFCore_API CHeapfile
{
public:
	CHeapfile(char* Path, char* Filename, unsigned int _ElementSize, unsigned int _KeyOffset=0);
	~CHeapfile();

	virtual void GetFromItemDescriptor(void* PtrDst, LFItemDescriptor* i);
	virtual void WriteToItemDescriptor(LFItemDescriptor* i, void* PtrSrc);

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

protected:
	void* Buffer;
	int FileCount;
	int FirstInBuffer;
	int LastInBuffer;
	unsigned int KeyOffset;
	unsigned int ElementSize;
	unsigned int BufferSize;
	bool BufferNeedsWriteback;
	bool NeedsCompaction;

	bool Writeback();
	bool OpenFile();
	void ElementToBuffer(int ID);

private:
	char IdxFilename[MAX_PATH];
	HANDLE hFile;
};
