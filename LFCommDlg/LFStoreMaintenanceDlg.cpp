
// LFStoreMaintenanceDlg.cpp: Implementierung der Klasse LFStoreMaintenanceDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CMaintenanceReport
//

CMaintenanceReport::CMaintenanceReport()
	: CFrontstageItemView(FRONTSTAGE_ENABLESCROLLING, sizeof(UsageItemData))
{
	p_MaintenanceList = NULL;

	// Error messages
	for (UINT a=0; a<LFErrorCount; a++)
		LFGetErrorText(m_ErrorText[a], 256, a);

	// Icons
	m_BadgeSize = GetSystemMetrics(SM_CYICON);
	hIconReady = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_READY), IMAGE_ICON, m_BadgeSize, m_BadgeSize, LR_SHARED);
	hIconWarning = (HICON)LoadImage(AfxGetResourceHandle(), IDI_WARNING, IMAGE_ICON, m_BadgeSize, m_BadgeSize, LR_SHARED);
	hIconError = (HICON)LoadImage(AfxGetResourceHandle(), IDI_ERROR, IMAGE_ICON, m_BadgeSize, m_BadgeSize, LR_SHARED);

	m_IconSize = ((m_ItemHeight-2*ITEMVIEWPADDING)>=128) ? 128 : LFGetApp()->m_ExtraLargeIconSize;
	p_StoreIcons = (m_IconSize==128) ? &LFGetApp()->m_CoreImageListJumbo : &LFGetApp()->m_CoreImageListExtraLarge;

	// Item
	SetItemHeight(LFGetApp()->m_ExtraLargeIconSize, 4);
}

void CMaintenanceReport::ShowTooltip(const CPoint& point)
{
	ASSERT(m_HoverItem>=0);

	LFStoreDescriptor StoreDescriptor;
	if (LFGetStoreSettings((*p_MaintenanceList)[m_HoverItem].StoreID, StoreDescriptor, TRUE)==LFOk)
		LFGetApp()->ShowTooltip(this, point, StoreDescriptor);
}

void CMaintenanceReport::SetMaintenanceList(LFMaintenanceList* pMaintenanceList)
{
	SetItemCount((p_MaintenanceList=pMaintenanceList)->m_ItemCount, TRUE);
	ValidateAllItems();

	AdjustLayout();
}

void CMaintenanceReport::AdjustLayout()
{
	AdjustLayoutColumns();
}

void CMaintenanceReport::DrawItem(CDC& dc, Graphics& /*g*/, LPCRECT rectItem, INT Index, BOOL Themed)
{
	const LFMaintenanceListItem* pData = &(*p_MaintenanceList)[Index];

	CRect rect(rectItem);
	rect.DeflateRect(ITEMVIEWPADDING, ITEMVIEWPADDING);

	// Icon
	p_StoreIcons->Draw(&dc, pData->IconID-1, CPoint(rect.left, rect.top+(rect.Height()-m_IconSize)/2), ILD_TRANSPARENT);
	rect.left += m_IconSize+ITEMVIEWPADDING;

	// Badge
	const HICON hIcon = (pData->Result==LFOk) ? hIconReady : (pData->Result<LFFirstFatalError) ? hIconWarning : hIconError;

	DrawIconEx(dc, rect.right-m_BadgeSize+2, rect.top+(rect.Height()-m_BadgeSize)/2, hIcon, m_BadgeSize, m_BadgeSize, 0, NULL, DI_NORMAL);
	rect.right -= m_BadgeSize+ITEMVIEWPADDING-2;

	// Text
	LPCWSTR pDescription = (pData->Result==LFOk) ? pData->Comments : m_ErrorText[pData->Result];

	CRect rectText(rect);
	dc.DrawText(pDescription, -1, rectText, DT_CALCRECT | DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_WORDBREAK);

	if (rectText.Height()>3*m_DefaultFontHeight)
		rectText.bottom = rectText.top+3*m_DefaultFontHeight;

	rect.top += (rect.Height()-rectText.Height()-m_DefaultFontHeight)/2-1;

	dc.DrawText(pData->Name, -1, rect, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_SINGLELINE);
	rect.top += m_DefaultFontHeight;

	SetDarkTextColor(dc, Index, hIcon, Themed);
	dc.DrawText(pDescription, -1, rect, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_WORDBREAK);
}


// LFStoreMaintenanceDlg
//

LFStoreMaintenanceDlg::LFStoreMaintenanceDlg(LFMaintenanceList* pMaintenanceList, CWnd* pParentWnd)
	: LFDialog(IDD_STOREMAINTENANCE, pParentWnd)
{
	ASSERT(pMaintenanceList);

	m_pMaintenanceList = pMaintenanceList;
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

	if (IsWindow(m_wndMaintenanceReport))
		m_wndMaintenanceReport.SetWindowPos(NULL, rectLayout.left, rectLayout.top+ExplorerHeight, rectLayout.Width(), m_BottomDivider-rectLayout.top-ExplorerHeight, nFlags);
}

BOOL LFStoreMaintenanceDlg::InitDialog()
{
	CString Hint;
	Hint.Format(CString((LPCSTR)(m_pMaintenanceList->m_ItemCount==1 ? IDS_STOREMAINTENANCE_HINT_SINGULAR : IDS_STOREMAINTENANCE_HINT_PLURAL)), m_pMaintenanceList->m_ItemCount);

	m_wndHeaderArea.Create(this, IDC_HEADERAREA);
	m_wndHeaderArea.SetHeader(CString((LPCSTR)IDS_STOREMAINTENANCE_CAPTION), Hint, NULL, CPoint(0, 0), FALSE);

	m_wndMaintenanceReport.Create(this, IDC_MAINTENANCEREPORT);
	m_wndMaintenanceReport.SetMaintenanceList(m_pMaintenanceList);
	m_wndMaintenanceReport.SetFocus();

	return FALSE;
}


BEGIN_MESSAGE_MAP(LFStoreMaintenanceDlg, LFDialog)
	ON_WM_DESTROY()
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

void LFStoreMaintenanceDlg::OnDestroy()
{
	LFFreeMaintenanceList(m_pMaintenanceList);

	LFDialog::OnDestroy();
}

void LFStoreMaintenanceDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	LFDialog::OnGetMinMaxInfo(lpMMI);

	lpMMI->ptMinTrackSize.x = max(lpMMI->ptMinTrackSize.x, 450);
	lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, 300);
}
