
// CContextSidebar.h: Schnittstelle der Klasse CContextSidebar
//

#pragma once
#include "LFCommDlg.h"


// CContextSidebar
//

class CContextSidebar : public CSidebar
{
friend class CSidebarCommand;

public:
	CContextSidebar();

	virtual CString AppendTooltip(UINT CmdID);

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void Reset(UINT CmdID, CHAR* StoreID);

protected:
	CHAR m_StoreID[LFKeySize];
	LFStatistics* m_pStatistics;

	afx_msg LRESULT OnStatistics(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	UINT m_ThreadID;
};
