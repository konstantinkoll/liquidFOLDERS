
// LFStoreNewLocalDlg.h: Schnittstelle der Klasse LFStoreNewLocalDlg
//

#pragma once
#include "LFCore.h"
#include "LFCommDlg.h"


// LFStoreNewLocalDlg
//

#define WM_PATHCHANGED     WM_USER+11

class AFX_EXT_CLASS LFStoreNewLocalDlg : public CPropertySheet
{
public:
	LFStoreNewLocalDlg(CWnd* pParentWnd=NULL, CHAR Volume='\0');

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

protected:
	CPropertyPage* m_pPages[2];
	CHAR m_Volume;
	WCHAR m_Path[MAX_PATH];
	BOOL m_IsRemovable;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnPathChanged();
	afx_msg LRESULT OnShellChange(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	ULONG m_ulSHChangeNotifyRegister;
};
