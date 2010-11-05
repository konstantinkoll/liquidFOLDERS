
// LFStoreDeleteDlg.h: Schnittstelle der Klasse LFStoreDeleteDlg
//

#pragma once
#include "LFDialog.h"


// LFStoreDeleteDlg
//

class AFX_EXT_CLASS LFStoreDeleteDlg : public LFDialog
{
public:
	LFStoreDeleteDlg(CWnd* pParentWnd, WCHAR* _StoreName);

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void SetOkButton();
	DECLARE_MESSAGE_MAP()

private:
	WCHAR* StoreName;
	CFont BoldFont;
};
