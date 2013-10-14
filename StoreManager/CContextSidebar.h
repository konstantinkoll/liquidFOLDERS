
// CContextSidebar.h: Schnittstelle der Klasse CContextSidebar
//

#pragma once
#include "LFCommDlg.h"


// CContextSidebar
//

#define WM_UPDATENUMBERS     WM_USER+208
#define WM_SETALERT          WM_USER+209

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
	CHAR m_StoreID[LFKeySize];
	LFStatistics* m_pStatistics;

	afx_msg void OnUpdateNumbers();
	DECLARE_MESSAGE_MAP()

private:
	UINT m_ThreadID;
	BOOL m_Initialized;
};
