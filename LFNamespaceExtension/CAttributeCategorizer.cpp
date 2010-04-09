
#include "stdafx.h"
#include "LFNamespaceExtension.h"
#include "CFolderItem.h"
#include "CAttributeCategorizer.h"


CAttributeCategorizer::CAttributeCategorizer(CNSEFolder* parent, CShellColumn column)
	: CCategorizer(parent, column)
{
}

DWORD CAttributeCategorizer::OnGetCategory(CNSEItem* child)
{
	if (child)
		if (IS(child, CFolderItem))
		{
			UINT ID = ((CFolderItem*)child)->data.CategoryID;
			if (ID<LFAttrCategoryCount)
				return ID;
		}

	return 0;
}

void CAttributeCategorizer::OnGetCategoryName(CString& categoryName, DWORD categoryID)
{
	if (categoryID>=LFAttrCategoryCount)
	{
		categoryName = "?";
	}
	else
	{
		categoryName = theApp.m_AttrCategoryNames[categoryID];
	}
}

int CAttributeCategorizer::OnCompareCategories(DWORD catID1, DWORD catID2)
{
	return wcscmp(theApp.m_AttrCategoryNames[catID1], theApp.m_AttrCategoryNames[catID2]);
}
