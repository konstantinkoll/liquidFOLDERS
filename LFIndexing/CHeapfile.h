
#pragma once


// CHeapfile
//

#define MaxBufferSize     262144

class LFIndexing_API CHeapfile
{
public:
	CHeapfile(char* _IdxFilename, unsigned int _ElementSize, unsigned int _KeyOffset=0);
	~CHeapfile();

	void CloseFile();
	bool FindNext(int& Next, void*& Ptr);
	bool FindKey(char* Key, int& Next, void*& Ptr);
	void Add(char* Key, void* Ptr);
	void Update(char* Key, void* Ptr);
	void Invalidate(void* Ptr);
	void InvalidateKey(char* Key, int& Next);
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
