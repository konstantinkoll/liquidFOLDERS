
// CMigrationList.cpp: Implementierung der Klasse CMigrationList
//

#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "CMigrationList.h"


// CMigrationList
//

CMigrationList::CMigrationList()
	: DynArray()
{
}

CMigrationList::~CMigrationList()
{
	if (m_Items)
		for (unsigned int a=0; a<m_ItemCount; a++)
		{
			LFFreeFileImportList(m_Items[a].List);
			LFFreeItemDescriptor(m_Items[a].Template);
		}
}

bool CMigrationList::AddFolder(wchar_t* name, wchar_t* path, LFItemDescriptor* it, int Icon, BOOL Recursive)
{
	ASSERT(path);

	ML_Entry entry = { L"", L"", Icon, LFAllocFileImportList(), LFAllocItemDescriptor(it), Recursive };
	wcscpy_s(entry.Name, 256, name);
	wcscpy_s(entry.Path, MAX_PATH, path);
	LFAddImportPath(entry.List, path);

	if (!DynArray::AddItem(entry))
	{
		LFFreeFileImportList(entry.List);
		LFFreeItemDescriptor(it);
		return false;
	}

	return true;
}