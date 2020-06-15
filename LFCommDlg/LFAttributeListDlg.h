
// LFAttributeListDlg.h: Schnittstelle der Klasse LFAttributeListDlg
//

#pragma once
#include "LFDialog.h"


// LFAttributeListDlg
//

class LFAttributeListDlg : public LFDialog
{
public:
	LFAttributeListDlg(UINT nIDTemplate, CWnd* pParentWnd=NULL, ITEMCONTEXT Context=LFContextAllFiles);

protected:
	virtual void TestAttribute(ATTRIBUTE Attr, BOOL& Add, BOOL& Check);

	void PrepareListCtrl(CExplorerList* pExplorerList, BOOL Check);
	void PrepareListCtrl(INT nID, BOOL Check);
	void FinalizeListCtrl(CExplorerList* pExplorerList, INT Focus=-1, BOOL Sort=TRUE);
	void FinalizeListCtrl(UINT nID, INT Focus=-1, BOOL Sort=TRUE);
	void AddAttribute(CExplorerList* pExplorerList, ATTRIBUTE Attr);
	void AddAttribute(UINT nID, ATTRIBUTE Attr);
	void PopulateListCtrl(CExplorerList* pExplorerList, BOOL Check, INT Focus=-1, BOOL Sort=TRUE);
	void PopulateListCtrl(INT nID, BOOL Check, INT Focus=-1, BOOL Sort=TRUE);

	LPCWSTR p_AttributeNames[LFAttributeCount];
	ITEMCONTEXT m_Context;
};
