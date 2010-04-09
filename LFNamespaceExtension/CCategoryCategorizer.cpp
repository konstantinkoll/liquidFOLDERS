
#include "stdafx.h"
#include "LFNamespaceExtension.h"
#include "CFolderItem.h"
#include "CCategoryCategorizer.h"


CCategoryCategorizer::CCategoryCategorizer(CNSEFolder* parent, CShellColumn column)
	: CCategorizer(parent, column)
{
}

DWORD CCategoryCategorizer::OnGetCategory(CNSEItem* child)
{
	if (child)
		if (IS(child, CFolderItem))
		{
			UINT ID = ((CFolderItem*)child)->data.CategoryID;
			if (ID<LFItemCategoryCount)
				return ID;
		}

	return 0;
}

void CCategoryCategorizer::OnGetCategoryName(CString& categoryName, DWORD categoryID)
{
	if (categoryID>=LFItemCategoryCount)
	{
		categoryName = "?";
	}
	else
	{
		categoryName = theApp.m_ItemCategories[categoryID]->Name;
	}
}

int CCategoryCategorizer::OnCompareCategories(DWORD catID1, DWORD catID2)
{
	return (int)catID1-(int)catID2;
}
