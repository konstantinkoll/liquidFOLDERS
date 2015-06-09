
// Categorizer.h: Schnittstelle der Categorizer-Klassen
//

#pragma once
#include "LF.h"
#include "LFCore.h"


// CAttributeCategorizer
//

class CAttributeCategorizer : public CCategorizer
{
public:
	CAttributeCategorizer(CNSEFolder* parent, CShellColumn column);

protected:
	virtual DWORD OnGetCategory(CNSEItem* child);
	virtual void OnGetCategoryName(CString& categoryName, DWORD categoryID);
	virtual INT OnCompareCategories(DWORD catID1, DWORD catID2);
};


// CCategoryCategorizer
//

class CCategoryCategorizer : public CCategorizer
{
public:
	CCategoryCategorizer(CNSEFolder* parent, CShellColumn column);

protected:
	virtual DWORD OnGetCategory(CNSEItem* child);
	virtual void OnGetCategoryName(CString& categoryName, DWORD categoryID);
	virtual INT OnCompareCategories(DWORD catID1, DWORD catID2);
};


// CFolderCategorizer
//

class CFolderCategorizer : public CCategorizer
{
public:
	CFolderCategorizer(CNSEFolder* parent, CShellColumn column);

protected:
	virtual DWORD OnGetCategory(CNSEItem* child);
	virtual void OnGetCategoryName(CString& categoryName, DWORD categoryID);
	virtual INT OnCompareCategories(DWORD catID1, DWORD catID2);
};


// CRatingCategorizer
//

class CRatingCategorizer : public CCategorizer
{
public:
	CRatingCategorizer(CNSEFolder* parent, CShellColumn column);

protected:
	virtual DWORD OnGetCategory(CNSEItem* child);
	virtual void OnGetCategoryName(CString& categoryName, DWORD categoryID);
	virtual INT OnCompareCategories(DWORD catID1, DWORD catID2);
};


// CSizeCategorizer
//

class CSizeCategorizer : public CCategorizer
{
public:
	CSizeCategorizer(CNSEFolder* parent, CShellColumn column);

protected:
	virtual DWORD OnGetCategory(CNSEItem* child);
	virtual void OnGetCategoryName(CString& categoryName, DWORD categoryID);
	virtual INT OnCompareCategories(DWORD catID1, DWORD catID2);
};
