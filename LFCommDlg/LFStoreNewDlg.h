
// LFStoreNewLocalDlg.h: Schnittstelle der Klasse LFStoreNewLocalDlg
//

#pragma once


// LFStoreNewLocalDlg
//

#define WM_PATHCHANGED     WM_USER+5

class LFStoreNewDlg : public CPropertySheet
{
public:
	LFStoreNewDlg(CWnd* pParentWnd=NULL);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnPathChanged();
	DECLARE_MESSAGE_MAP()

	CPropertyPage* m_pPages[2];
	WCHAR m_Path[MAX_PATH];
	BOOL m_IsRemovable;
};
