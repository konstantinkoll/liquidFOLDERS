
// LFStoreMaintenanceDlg.cpp: Implementierung der Klasse LFStoreMaintenanceDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFStoreMaintenanceDlg
//


LFStoreMaintenanceDlg::LFStoreMaintenanceDlg(LFMaintenanceList* pMaintenanceList, CWnd* pParentWnd)
	: LFDialog(IDD_STOREMAINTENANCE, pParentWnd)
{
	ASSERT(pMaintenanceList);

	m_pMaintenanceList = pMaintenanceList;
}

LFStoreMaintenanceDlg::~LFStoreMaintenanceDlg()
{
	if (m_pMaintenanceList)
		LFFreeMaintenanceList(m_pMaintenanceList);
}

void LFStoreMaintenanceDlg::AdjustLayout(const CRect& rectLayout, UINT nFlags)
{
	LFDialog::AdjustLayout(rectLayout, nFlags);

	UINT ExplorerHeight = 0;
	if (IsWindow(m_wndHeaderArea))
	{
		ExplorerHeight = m_wndHeaderArea.GetPreferredHeight();
		m_wndHeaderArea.SetWindowPos(NULL, rectLayout.left, rectLayout.top, rectLayout.Width(), ExplorerHeight, nFlags);
	}

	m_wndMaintenanceReport.SetWindowPos(NULL, rectLayout.left, rectLayout.top+ExplorerHeight, rectLayout.Width(), m_BottomDivider-rectLayout.top-ExplorerHeight, nFlags);
}

BOOL LFStoreMaintenanceDlg::InitDialog()
{
	CString Caption((LPCSTR)IDS_STOREMAINTENANCE_CAPTION);

	CString Mask;
	ENSURE(Mask.LoadString(m_pMaintenanceList->m_ItemCount==1 ? IDS_STOREMAINTENANCE_HINT_SINGULAR : IDS_STOREMAINTENANCE_HINT_PLURAL));

	CString Hint;
	Hint.Format(Mask, m_pMaintenanceList->m_ItemCount);

	m_wndHeaderArea.Create(this, IDC_HEADERAREA);
	m_wndHeaderArea.SetText(Caption, Hint, FALSE);

	m_wndMaintenanceReport.Create(this, IDC_MAINTENANCEREPORT);
	m_wndMaintenanceReport.SetMaintenanceList(m_pMaintenanceList);
	m_wndMaintenanceReport.SetFocus();

	return FALSE;
}


BEGIN_MESSAGE_MAP(LFStoreMaintenanceDlg, LFDialog)
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

void LFStoreMaintenanceDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	LFDialog::OnGetMinMaxInfo(lpMMI);

	lpMMI->ptMinTrackSize.x = max(lpMMI->ptMinTrackSize.x, 450);
	lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, 300);
}
