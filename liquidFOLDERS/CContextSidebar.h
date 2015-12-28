
// CContextSidebar.h: Schnittstelle der Klasse CContextSidebar
//

#pragma once
#include "LFCommDlg.h"


// CContextSidebar
//

#define WM_UPDATECOUNTS     WM_USER+209

class CContextSidebar : public CBackstageSidebar
{
friend class CSidebarCommand;

public:
	CContextSidebar();
	~CContextSidebar();

	virtual CString AppendTooltip(UINT CmdID);

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetSelection(UINT CmdID, CHAR* StoreID);

protected:
	afx_msg void OnUpdateCounts();
	DECLARE_MESSAGE_MAP()

	static CIcons m_LargeIcons;
	static CIcons m_SmallIcons;
	CHAR m_StoreID[LFKeySize];
	LFStatistics* m_pStatistics;

private:
	UINT m_ThreadID;
	BOOL m_Initialized;
};
