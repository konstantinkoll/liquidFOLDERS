
// LFAttributeListDlg.h: Schnittstelle der Klasse LFAttributeListDlg
//

#pragma once
#include "CImageListTransparent.h"
#include "LFDialog.h"


// LFAttributeListDlg
//

class LFAttributeListDlg : public LFDialog
{
public:
	LFAttributeListDlg(UINT nIDTemplate, CWnd* pParentWnd=NULL);

protected:
	virtual void TestAttribute(UINT Attr, BOOL& Add, BOOL& Check);

	void PrepareListCtrl(CListCtrl* li, BOOL Check);
	void PrepareListCtrl(INT nID, BOOL Check);
	void FinalizeListCtrl(CListCtrl* li, INT Focus=-1, BOOL Sort=TRUE);
	void FinalizeListCtrl(UINT nID, INT Focus=-1, BOOL Sort=TRUE);
	void AddAttribute(CListCtrl* li, UINT Attr);
	void AddAttribute(UINT nID, UINT Attr);
	void PopulateListCtrl(CListCtrl* li, BOOL Check, INT Focus=-1, BOOL Sort=TRUE);
	void PopulateListCtrl(INT nID, BOOL Check, INT Focus=-1, BOOL Sort=TRUE);

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CImageListTransparent m_AttributeIcons;
};
