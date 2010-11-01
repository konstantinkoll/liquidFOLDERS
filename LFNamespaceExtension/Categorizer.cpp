
// Commands.cpp: Implementierung der Categorizer-Klassen
//

#include "stdafx.h"
#include "LFNamespaceExtension.h"
#include "CFileItem.h"
#include "CFolderItem.h"
#include "Categorizer.h"


// CAttributeCategorizer
//

CAttributeCategorizer::CAttributeCategorizer(CNSEFolder* parent, CShellColumn column)
	: CCategorizer(parent, column)
{
}

DWORD CAttributeCategorizer::OnGetCategory(CNSEItem* child)
{
	if (child)
		if (IS(child, CFolderItem))
		{
			UINT ID = ((CFolderItem*)child)->Attrs.CategoryID;
			if (ID<LFAttrCategoryCount)
				return ID;
		}

	return 0;
}

void CAttributeCategorizer::OnGetCategoryName(CString& categoryName, DWORD categoryID)
{
	if (categoryID>=LFAttrCategoryCount)
	{
		categoryName = _T("?");
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


// CCategoryCategorizer
//

CCategoryCategorizer::CCategoryCategorizer(CNSEFolder* parent, CShellColumn column)
	: CCategorizer(parent, column)
{
}

DWORD CCategoryCategorizer::OnGetCategory(CNSEItem* child)
{
	if (child)
		if (IS(child, CFolderItem))
		{
			UINT ID = ((CFolderItem*)child)->Attrs.CategoryID;
			if (ID<LFItemCategoryCount)
				return ID;
		}

	return 0;
}

void CCategoryCategorizer::OnGetCategoryName(CString& categoryName, DWORD categoryID)
{
	if (categoryID>=LFItemCategoryCount)
	{
		categoryName = _T("?");
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


// CFolderCategorizer
//

CFolderCategorizer::CFolderCategorizer(CNSEFolder* parent, CShellColumn column)
	: CCategorizer(parent, column)
{
}

DWORD CFolderCategorizer::OnGetCategory(CNSEItem* child)
{
	if (child)
		return IS(child, CFolderItem) ? 0 : 1;

	return 0;
}

void CFolderCategorizer::OnGetCategoryName(CString& categoryName, DWORD categoryID)
{
	switch (categoryID)
	{
	case 0:
		ENSURE(categoryName.LoadString(IDS_Folders));
		break;
	case 1:
		ENSURE(categoryName.LoadString(IDS_Files));
		break;
	default:
		categoryName = _T("?");
	}
}

int CFolderCategorizer::OnCompareCategories(DWORD catID1, DWORD catID2)
{
	return wcscmp(theApp.m_AttrCategoryNames[catID1], theApp.m_AttrCategoryNames[catID2]);
}


// CRatingCategorizer
//

CRatingCategorizer::CRatingCategorizer(CNSEFolder* parent, CShellColumn column)
	: CCategorizer(parent, column)
{
}

DWORD CRatingCategorizer::OnGetCategory(CNSEItem* child)
{
	if (child)
		if (IS(child, CFileItem))
			switch (this->column.index)
			{
			case LFAttrRating:
				return AS(child, CFileItem)->Attrs.Rating/2;
			case LFAttrPriority:
				return AS(child, CFileItem)->Attrs.Priority/2;
			}

	return 0;
}

void CRatingCategorizer::OnGetCategoryName(CString& categoryName, DWORD categoryID)
{
	if (categoryID<=5)
		switch (this->column.index)
		{
		case LFAttrRating:
			categoryName = theApp.m_Categories[0][categoryID];
			return;
		case LFAttrPriority:
			categoryName = theApp.m_Categories[1][categoryID];
			return;
		}

	categoryName = _T("?");
}

int CRatingCategorizer::OnCompareCategories(DWORD catID1, DWORD catID2)
{
	return (int)catID2-(int)catID1;
}


// CSizeCategorizer
//

CSizeCategorizer::CSizeCategorizer(CNSEFolder* parent, CShellColumn column)
	: CCategorizer(parent, column)
{
}

DWORD CSizeCategorizer::OnGetCategory(CNSEItem* child)
{
	if (child)
	{
		__int64 sz = 0;

		if (IS(child, CFileItem))
			sz = AS(child, CFileItem)->Attrs.FileSize;

		if (IS(child, CFolderItem))
			sz = AS(child, CFolderItem)->Attrs.Size;

		if (sz<32*1024)
			return 0;
		if (sz<128*1024)
			return 1;
		if (sz<1024*1024)
			return 2;
		if (sz<16384*1024)
			return 3;
		if (sz<131072*1024)
			return 4;

		return 5;
	}

	return 0;
}

void CSizeCategorizer::OnGetCategoryName(CString& categoryName, DWORD categoryID)
{
	if (categoryID>5)
	{
		categoryName = _T("?");
	}
	else
	{
		categoryName = theApp.m_Categories[2][categoryID];
	}
}

int CSizeCategorizer::OnCompareCategories(DWORD catID1, DWORD catID2)
{
	return (int)catID1-(int)catID2;
}
