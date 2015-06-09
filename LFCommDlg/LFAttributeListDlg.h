
// LFAttributeListDlg.h: Schnittstelle der Klasse LFAttributeListDlg
//

#pragma once
#include "CImageListTransparent.h"
#include "LFApplication.h"
#include "LFDialog.h"


// LFAttributeListDlg
//

class LFAttributeListDlg : public LFDialog
{
public:
	LFAttributeListDlg(UINT nIDTemplate, CWnd* pParentWnd=NULL);

protected:
	LFApplication* p_App;

	virtual void TestAttribute(UINT attr, BOOL& add, BOOL& check);

	void PrepareListCtrl(CListCtrl* li, BOOL check);
	void PrepareListCtrl(INT nID, BOOL check);
	void FinalizeListCtrl(CListCtrl* li, INT focus=-1, BOOL sort=TRUE);
	void FinalizeListCtrl(UINT nID, INT focus=-1, BOOL sort=TRUE);
	void AddAttribute(CListCtrl* li, UINT attr);
	void AddAttribute(UINT nID, UINT attr);
	void PopulateListCtrl(CListCtrl* li, BOOL check, INT focus=-1, BOOL sort=TRUE);
	void PopulateListCtrl(INT nID, BOOL check, INT focus=-1, BOOL sort=TRUE);

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CImageListTransparent m_AttributeIcons;
};
