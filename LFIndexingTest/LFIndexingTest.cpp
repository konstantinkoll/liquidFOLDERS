// LFIndexingTest.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "LFCore.h"
#include "..\\LFCore\\CIndex.h"
#include <iostream>

using namespace std;

struct IdxData
{
	UINT number1;
	UINT number2;
	char fill[8192];
};

void LFCore_API SetAttribute(LFItemDescriptor* i, unsigned int attr, const void* v, bool ToString=true, wchar_t* ustr=NULL);

void Test_CIndex()
{
	CIndex* idx;
	const UINT cnt = 10000;
	//char Path[MAX_PATH] = "C:\\users\\root\\";
	char Path[MAX_PATH] = "J:\\";


	// Add files
	cout << endl << "Add " << cnt << " files...";
	DWORD start = GetTickCount();

	idx = new CIndex(Path, "TEST1234");
	for (UINT a=0; a<cnt; a++)
	{
		LFItemDescriptor* i = LFAllocItemDescriptor();
		SetAttribute(i, LFAttrFileName, L"Test file");
		SetAttribute(i, LFAttrStoreID, L"TEST1234");
		char Key[LFKeySize];
		sprintf_s(Key, LFKeySize, "KEY%d", a);
		SetAttribute(i, LFAttrFileID, Key);
		__int64 sz = a;
		SetAttribute(i, LFAttrFileSize, &sz);

		i->CoreAttributes.SlaveID = a%6;

		idx->AddItem(i);
		delete i;
	}
	delete idx;

	cout << " " << GetTickCount()-start << " ms for " << cnt << " files";
	cin.get();


	// Retrieve files
	cout << endl << "Retrieve and verify " << cnt << " files...";
	start = GetTickCount();

	LFFilter* f = LFAllocFilter();
	f->Mode = LFFilterModeSearchInStore;
	strcpy_s(f->StoreID, LFKeySize, "TEST1234");

	LFSearchResult* res = LFAllocSearchResult(LFContextDefault);

	idx = new CIndex(Path, "TEST1234");
	idx->Retrieve(NULL, res);
	delete idx;

	for (UINT a=0; a<res->m_Count; a++)
	{
		char Key[LFKeySize];
		sprintf_s(Key, LFKeySize, "KEY%d", a);
		if (strcmp(Key, res->m_Files[a]->CoreAttributes.FileID)!=0)
			cout << a << " ";
	}

	cout << " " << GetTickCount()-start << " ms for " << res->m_Count << " files";
	cin.get();
}

/*void Test_BitArray()
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
}*/

int _tmain(int /*argc*/, _TCHAR* /*argv[]*/)
{
	//std::cout << "##### BitArray #####" << endl;
	//Test_BitArray();
	std::cout << endl << endl << "##### CIndex #####" << endl;
	Test_CIndex();
}
