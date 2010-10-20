
// CMigrationList.h: Schnittstelle der Klasse CMigrationList
//

#pragma once
#include "LFCore.h"
#include "DynArray.h"


// CReportList
// CMigrationList
//

struct ML_Item
{
	wchar_t Name[256];
	wchar_t Path[MAX_PATH];
	int Icon;
	LFFileImportList* List;
	LFItemDescriptor* Template;
	BOOL Recursive;
};

typedef DynArray<ML_Item*> CReportList;

class CMigrationList : public DynArray<ML_Item>
{
public:
	CMigrationList();
	~CMigrationList();

	bool AddFolder(wchar_t* name, wchar_t* path, LFItemDescriptor* it, int Icon, BOOL Recursive);
};
