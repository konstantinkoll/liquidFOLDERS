
// CContextSidebar.h: Schnittstelle der Klasse CContextSidebar
//

#pragma once
#include "LFCommDlg.h"


// CContextSidebar
//

#define WM_UPDATENUMBERS     WM_USER+209
#define WM_SETALERT          WM_USER+210

class CContextSidebar : public CSidebar
{
friend class CSidebarCommand;

public:
	CContextSidebar();
	~CContextSidebar();

	virtual CString AppendTooltip(UINT CmdID);

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetSelection(UINT CmdID, CHAR* StoreID);

protected:
	afx_msg void OnUpdateNumbers();
	DECLARE_MESSAGE_MAP()

	CHAR m_StoreID[LFKeySize];
	LFStatistics* m_pStatistics;

private:
	UINT m_ThreadID;
	BOOL m_Initialized;
};
