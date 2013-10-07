
// CContextSidebar.h: Schnittstelle der Klasse CContextSidebar
//

#pragma once
#include "LFCommDlg.h"


// CContextSidebar
//

#define WM_UPDATENUMBERS     WM_USER+104
#define WM_STATISTICS        WM_USER+105

class CContextSidebar : public CSidebar
{
friend class CSidebarCommand;

public:
	CContextSidebar();
	~CContextSidebar();

	virtual CString AppendTooltip(UINT CmdID);

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void Reset(UINT CmdID, CHAR* StoreID);

protected:
	CHAR m_StoreID[LFKeySize];
	LFStatistics* m_pStatistics;

	afx_msg void OnUpdateNumbers();
	afx_msg LRESULT OnStatistics(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	UINT m_ThreadID;
};
