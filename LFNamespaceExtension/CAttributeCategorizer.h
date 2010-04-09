
#pragma once
#include "liquidFOLDERS.h"
#include "LFCore.h"

class CAttributeCategorizer : public CCategorizer
{
public:
	CAttributeCategorizer(CNSEFolder* parent, CShellColumn column);

protected:
	virtual DWORD OnGetCategory(CNSEItem* child);
	virtual void OnGetCategoryName(CString& categoryName, DWORD categoryID);
	virtual int OnCompareCategories(DWORD catID1, DWORD catID2);
};
