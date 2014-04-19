
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
	WCHAR Name[256];
	WCHAR Path[MAX_PATH];
	INT Icon;
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

	bool AddFolder(WCHAR* name, WCHAR* path, LFItemDescriptor* it, INT Icon, BOOL Recursive);
};
