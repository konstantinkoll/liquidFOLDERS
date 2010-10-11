
// CMigrationList.h: Schnittstelle der Klasse CMigrationList
//

#pragma once
#include "LFCore.h"
#include "DynArray.h"


// CMigrationList
//

struct ML_Entry
{
	LFFileImportList* List;
	LFItemDescriptor* Template;
	BOOL Resolve;
};

class CMigrationList : public DynArray<ML_Entry>
{
public:
	CMigrationList();
	~CMigrationList();

	bool AddFolder(wchar_t* path, LFItemDescriptor* it, BOOL Resolve);
};
