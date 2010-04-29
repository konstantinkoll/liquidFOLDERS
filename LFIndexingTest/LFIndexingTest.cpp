// LFIndexingTest.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "LFCore.h"
#include "..\\LFCore\\CHeapfile.h"
#include <iostream>

using namespace std;

struct IdxData
{
	UINT number1;
	UINT number2;
	char fill[8192];
};

void Test_CHeapfile()
{
	// Create empty file - in a live system, this is done by
	// the indexing class which employs a CHeapfile object.
	char Filename[MAX_PATH] = "J:\\TEST.IDX";
	HANDLE hFile = CreateFileA(Filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0,NULL);
	if (hFile==INVALID_HANDLE_VALUE)
	{
		cout << "Cannot create file";
		return;
	}
	CloseHandle(hFile);

	CHeapfile* f = new CHeapfile(Filename, "", sizeof(IdxData));

	char Key[LFKeySize] = { 0 };
	IdxData Buffer = { 0 };

	// Add 100000 entries
	cout << endl << "Create 100000 files...";
	DWORD start = GetTickCount();
	for (UINT a=0; a<100000; a++)
	{
		Buffer.number1 = a;
		Buffer.number2 = rand();
		sprintf_s(Key, LFKeySize, "%d", a);
		f->Add(Key, &Buffer);
	}
	cout << " " << GetTickCount()-start << " ms";

	// Scan all 100000 entries
	cout << endl << "Scan all 100000 files...";
	start = GetTickCount();
	int Next = 0;
	bool result;
	void* PtrToData;
	UINT x = 0;
	do
	{
		x++;
		result = f->FindNext(Next, PtrToData);
	}
	while (result);
	cout << " " << GetTickCount()-start << " ms";

	// Search and skip entries
	cout << endl << "Search and skip entries (press key to start)...";
	cin.get();
	start = GetTickCount();
	Next = 0;
	for (UINT a=0; a<100; a++)
	{
		UINT expect = Next+rand()%1000;
		sprintf_s(Key, LFKeySize, "%d", expect);
		result = f->FindKey(Key, Next, PtrToData);
		if (((IdxData*)PtrToData)->number1!=expect)
		{
			cout << "ERROR(" << expect << "-" << ((IdxData*)PtrToData)->number1 << ") ";
		}
		else
		{
			cout << "OK(" << expect << ") ";
		}
	}
	cout << endl << GetTickCount()-start << " ms";

	// Delete every 2nd entry
	cout << endl << "Delete every 2nd entry (press key to start)...";
	cin.get();
	start = GetTickCount();
	Next = 0;
	for (UINT a=0; a<100000; a++)
	{
		result = f->FindNext(Next, PtrToData);
		if (a%2)
			f->Invalidate(PtrToData);
	}
	cout << endl << GetTickCount()-start << " ms";

	Next = 0;
	x = 0;
	do
	{
		result = f->FindNext(Next, PtrToData);
		x++;
	}
	while (result);
	cout << endl << "Elements remaining: " << x-1;

	// Compact
	cout << endl << "Compaction (press key to start)...";
	cin.get();
	start = GetTickCount();
	f->Compact();
	cout << endl << GetTickCount()-start << " ms";

	// Verify compaction
	cout << endl << "Verify compaction (press key to start)...";
	cin.get();
	Next = 0;
	for (UINT a=0; a<50000; a++)
	{
		UINT expect = a*2;
		sprintf_s(Key, LFKeySize, "%d", expect);
		result = f->FindKey(Key, Next, PtrToData);
		if (((IdxData*)PtrToData)->number1!=expect)
		{
			cout << "ERROR(" << expect << "-" << ((IdxData*)PtrToData)->number1 << ") ";
		}
		else
		{
			cout << "OK(" << expect << ") ";
		}
	}

	delete f;
	cin.get();
}

void Test_BitArray()
{
	LFBitArray a(64000);

	cout << "Size:      " << a.m_nSize << endl;
	cout << "Allocated: " << a.m_nTAllocated << endl;
	cout << endl;

	cout << a.IsSet(0) << endl;
	a+=8;
	cout << a.IsSet(0) << endl;

	LFBitArray b(64000);
	LFBitArray c=b+0;

	cout << b.IsSet(0) << endl;
	cout << c.IsSet(0) << endl;

	cin.get();
}

int _tmain(int /*argc*/, _TCHAR* /*argv[]*/)
{
	//std::cout << "##### BitArray #####" << endl;
	//Test_BitArray();
	std::cout << endl << endl << "##### CHeapfile #####" << endl;
	Test_CHeapfile();
}
