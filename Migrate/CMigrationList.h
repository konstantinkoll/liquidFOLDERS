
// CMigrationList.h: Schnittstelle der Klasse CMigrationList
//

#pragma once
#include "LFCore.h"
#include "DynArray.h"


// CReportList
// CMigrationList
//

struct ML_Entry
{
	wchar_t Name[256];
	wchar_t Path[MAX_PATH];
	int Icon;
	LFFileImportList* List;
	LFItemDescriptor* Template;
	BOOL Recursive;
};

typedef DynArray<ML_Entry*> CReportList;

class CMigrationList : public DynArray<ML_Entry>
{
public:
	CMigrationList();
	~CMigrationList();

	bool AddFolder(wchar_t* name, wchar_t* path, LFItemDescriptor* it, int Icon, BOOL Recursive);
};
