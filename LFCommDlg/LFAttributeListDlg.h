
// LFAttributeListDlg.h: Schnittstelle der Klasse LFAttributeListDlg
//

#pragma once
#include "LFDialog.h"


// LFAttributeListDlg
//

class LFAttributeListDlg : public LFDialog
{
public:
	LFAttributeListDlg(UINT nIDTemplate, CWnd* pParentWnd=NULL);

protected:
	virtual void TestAttribute(UINT Attr, BOOL& Add, BOOL& Check);

	void PrepareListCtrl(CExplorerList* pExplorerList, BOOL Check);
	void PrepareListCtrl(INT nID, BOOL Check);
	void FinalizeListCtrl(CExplorerList* pExplorerList, INT Focus=-1, BOOL Sort=TRUE);
	void FinalizeListCtrl(UINT nID, INT Focus=-1, BOOL Sort=TRUE);
	void AddAttribute(CExplorerList* pExplorerList, UINT Attr);
	void AddAttribute(UINT nID, UINT Attr);
	void PopulateListCtrl(CExplorerList* pExplorerList, BOOL Check, INT Focus=-1, BOOL Sort=TRUE);
	void PopulateListCtrl(INT nID, BOOL Check, INT Focus=-1, BOOL Sort=TRUE);
};
