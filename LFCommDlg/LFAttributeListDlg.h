
// LFAttributeListDlg.h: Schnittstelle der Klasse LFAttributeListDlg
//

#pragma once
#include "CImageListTransparent.h"
#include "LFApplication.h"


// LFAttributeListDlg
//

class AFX_EXT_CLASS LFAttributeListDlg : public CDialog
{
public:
	LFAttributeListDlg(UINT nIDTemplate, CWnd* pParentWnd=NULL);

protected:
	virtual void TestAttribute(UINT attr, BOOL& add, BOOL& check);

	void PrepareListCtrl(CListCtrl* li, BOOL check);
	void PrepareListCtrl(int nID, BOOL check);
	void FinalizeListCtrl(CListCtrl* li, int focus=-1, BOOL sort=TRUE);
	void FinalizeListCtrl(UINT nID, int focus=-1, BOOL sort=TRUE);
	void AddAttribute(CListCtrl* li, UINT attr);
	void AddAttribute(UINT nID, UINT attr);
	void PopulateListCtrl(CListCtrl* li, BOOL check, int focus=-1, BOOL sort=TRUE);
	void PopulateListCtrl(int nID, BOOL check, int focus=-1, BOOL sort=TRUE);

	LFApplication* p_App;

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CImageListTransparent m_AttributeIcons;
};
