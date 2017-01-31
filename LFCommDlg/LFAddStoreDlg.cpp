
// LFAddStoreDlg.cpp: Implementierung der Klasse LFAddStoreDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFCreateStoreDlg.h"
#include <wininet.h>


// LFAddStoreDlg
//

#define BORDER     10

const UINT LFAddStoreDlg::m_Sources[] = { LFTypeSourceInternal, LFTypeSourceWindows,
	LFTypeSourceDropbox, LFTypeSourceICloud, LFTypeSourceOneDrive };

LFAddStoreDlg::LFAddStoreDlg(CWnd* pParentWnd)
	: LFDialog(IDD_ADDSTORE, pParentWnd)
{
}

BOOL LFAddStoreDlg::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if ((nCode==BN_CLICKED) && (nID>=IDC_ADDSTORE_LIQUIDFOLDERS))
		m_wndExplorerNotification.DismissNotification();

	return LFDialog::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void LFAddStoreDlg::AdjustLayout(const CRect& rectLayout, UINT nFlags)
{
	if (IsWindow(m_wndExplorerNotification))
	{
		const UINT NotificationHeight = m_wndExplorerNotification.GetPreferredHeight();
		m_wndExplorerNotification.SetWindowPos(&wndTop, rectLayout.left+32, rectLayout.bottom-NotificationHeight+1, rectLayout.Width()-64, NotificationHeight, nFlags & ~(SWP_NOZORDER | SWP_NOOWNERZORDER));
	}
}

void LFAddStoreDlg::CheckSources()
{
	GetDlgItem(IDC_ADDSTORE_DROPBOX)->EnableWindow(m_Dropbox.CheckForDropbox());
	GetDlgItem(IDC_ADDSTORE_ICLOUD)->EnableWindow(m_ICloud.CheckForICloud());
	GetDlgItem(IDC_ADDSTORE_ONEDRIVE)->EnableWindow(m_OneDrive.CheckForOneDrive());
}

void LFAddStoreDlg::ShowResult(UINT Result, const CString StoreName)
{
	if (Result>LFCancel)
	{
		LFErrorBox(this, Result);
	}
	else
		if (Result==LFOk)
		{
			CString Text;
			Text.Format(IDS_STORECREATED, StoreName);

			m_wndExplorerNotification.SetNotification(ENT_READY, Text);
		}
		else
		{
			WCHAR Text[256];
			LFGetErrorText(Text, 256, Result);

			m_wndExplorerNotification.SetNotification(ENT_WARNING, Text);
		}
}

BOOL LFAddStoreDlg::InitDialog()
{
	// Categories
	m_wndCategory[0].SetWindowText(LFGetApp()->m_ItemCategories[LFItemCategoryLocal].Caption);
	m_wndCategory[1].SetWindowText(LFGetApp()->m_ItemCategories[LFItemCategoryRemote].Caption);

	// Sources
	CheckSources();

	// Notification
	m_wndExplorerNotification.Create(this, 1);

	// Status
	SetBottomLeftControl(IDC_STATUS);

	OnUpdateStores(NULL, NULL);

	return TRUE;
}

void LFAddStoreDlg::AddWindowsPathAsStore(LPCWSTR Path, LPCWSTR StoreName)
{
	ASSERT(Path);
	ASSERT(StoreName);

	WorkerParameters wp;
	ZeroMemory(&wp, sizeof(wp));
	wcscpy_s(wp.Path, MAX_PATH, Path);
	wcscpy_s(wp.StoreName, MAX_PATH, StoreName);

	LFDoWithProgress(WorkerCreateStoreWindows, (LFWorkerParameters*)&wp, this);

	ShowResult(wp.Result, wp.StoreName);
}


BEGIN_MESSAGE_MAP(LFAddStoreDlg, LFDialog)
	ON_WM_ACTIVATE()
	ON_NOTIFY_RANGE(REQUEST_DRAWBUTTONFOREGROUND, IDC_ADDSTORE_LIQUIDFOLDERS, IDC_ADDSTORE_ONEDRIVE, OnDrawButtonForeground)
	ON_NOTIFY_RANGE(REQUEST_TOOLTIP_DATA, IDC_ADDSTORE_LIQUIDFOLDERS, IDC_ADDSTORE_ONEDRIVE, OnRequestTooltipData)

	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoresChanged, OnUpdateStores)

	ON_BN_CLICKED(IDC_ADDSTORE_LIQUIDFOLDERS, OnBtnLiquidfolders)
	ON_BN_CLICKED(IDC_ADDSTORE_WINDOWS, OnBtnWindows)
	ON_BN_CLICKED(IDC_ADDSTORE_DROPBOX, OnBtnDropbox)
	ON_BN_CLICKED(IDC_ADDSTORE_ICLOUD, OnBtnICloud)
	ON_BN_CLICKED(IDC_ADDSTORE_ONEDRIVE, OnBtnOneDrive)
END_MESSAGE_MAP()

void LFAddStoreDlg::OnActivate(UINT nState, CWnd* /*pWndOther*/, BOOL /*bMinimized*/)
{
#ifndef DEBUG
	if (nState!=WA_INACTIVE)
		CheckSources();
#endif
}

void LFAddStoreDlg::OnDrawButtonForeground(UINT /*nCtrlID*/, NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_DRAWBUTTONFOREGROUND* pDrawButtonForeground = (NM_DRAWBUTTONFOREGROUND*)pNMHDR;
	LPDRAWITEMSTRUCT lpDrawItemStruct = pDrawButtonForeground->lpDrawItemStruct;

	// Content
	CRect rect(lpDrawItemStruct->rcItem);

	const UINT nID = lpDrawItemStruct->CtlID-IDC_ADDSTORE_LIQUIDFOLDERS;
	ASSERT(nID<=LFSourceCount);

	WCHAR Caption[256];
	LFGetSourceName(Caption, 256, m_Sources[nID], FALSE);

	WCHAR Hint[256];
	::GetWindowText(lpDrawItemStruct->hwndItem, Hint, 256);

	INT Height = rect.Height()-2*BORDER;
	INT IconSize = (Height>=128) ? 128 : (Height>=96) ? 96 : 48;
	CImageList* pIcons = (IconSize==128) ? &LFGetApp()->m_CoreImageListJumbo : (IconSize==96) ? &LFGetApp()->m_CoreImageListHuge : &LFGetApp()->m_CoreImageListExtraLarge;

	// Icon
	CPoint pt(rect.left+BORDER, rect.top+(rect.Height()-IconSize)/2);
	pIcons->DrawEx(pDrawButtonForeground->pDC, m_Sources[nID]-1, pt, CSize(IconSize, IconSize), CLR_NONE, 0xFFFFFF, lpDrawItemStruct->itemState & ODS_DISABLED ? ILD_BLEND25 : ILD_TRANSPARENT);

	rect.left += IconSize+BORDER;
	rect.DeflateRect(BORDER, BORDER);

	// Text
	const INT HeightCaption = LFGetApp()->m_LargeFont.GetFontHeight()*3/2;

	CFont* pOldFont = pDrawButtonForeground->pDC->SelectObject(&LFGetApp()->m_DefaultFont);

	CRect rectHint(rect);
	pDrawButtonForeground->pDC->DrawText(Hint, rectHint, DT_CALCRECT | DT_WORDBREAK | DT_END_ELLIPSIS | DT_NOPREFIX);

	rect.top += (rect.Height()-HeightCaption-rectHint.Height())/2;

	pDrawButtonForeground->pDC->SelectObject(&LFGetApp()->m_LargeFont);
	pDrawButtonForeground->pDC->DrawText(Caption, rect, DT_SINGLELINE | DT_END_ELLIPSIS);
	rect.top += HeightCaption;

	pDrawButtonForeground->pDC->SelectObject(&LFGetApp()->m_DefaultFont);
	pDrawButtonForeground->pDC->DrawText(Hint, rect, DT_WORDBREAK | DT_END_ELLIPSIS | DT_NOPREFIX);

	pDrawButtonForeground->pDC->SelectObject(pOldFont);

	*pResult = TRUE;
}

void LFAddStoreDlg::OnRequestTooltipData(UINT nCtrlID, NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TOOLTIPDATA* pTooltipData = (NM_TOOLTIPDATA*)pNMHDR;

	const UINT nID = nCtrlID-IDC_ADDSTORE_LIQUIDFOLDERS;
	ASSERT(nID<=LFSourceCount);

	LFGetSourceName(pTooltipData->Caption, 256, m_Sources[nID], FALSE);
	ENSURE(LoadString(AfxGetResourceHandle(), nID+IDS_ADDSTORE_LIQUIDFOLDERS, pTooltipData->Hint, 4096));

	*pResult = TRUE;
}


LRESULT LFAddStoreDlg::OnUpdateStores(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	UINT StoreCount = LFGetStoreCount();

	CString Hint;
	Hint.Format(StoreCount==1 ? IDS_STORES_SINGULAR : IDS_STORES_PLURAL, StoreCount);

	GetDlgItem(IDC_STATUS)->SetWindowText(Hint);

	return NULL;
}


void LFAddStoreDlg::OnBtnLiquidfolders()
{
	LFCreateStoreDlg dlg(this);
	if (dlg.DoModal()==IDOK)
		ShowResult(dlg.m_Result, dlg.m_StoreName);
}

void LFAddStoreDlg::OnBtnWindows()
{
	CString Caption;
	GetWindowText(Caption);

	CString Hint;
	GetDlgItem(IDC_ADDSTORE_WINDOWS)->GetWindowText(Hint);

	LFBrowseForFolderDlg dlg(this, Caption, Hint);
	if (dlg.DoModal()==IDOK)
		AddWindowsPathAsStore(dlg.m_FolderPath);
}

void LFAddStoreDlg::OnBtnDropbox()
{
	LFDropboxDlg dlg(m_Dropbox, this);
	if (dlg.DoModal()==IDOK)
		AddWindowsPathAsStore(dlg.m_FolderPath);
}

void LFAddStoreDlg::OnBtnICloud()
{
	LFICloudDlg dlg(m_ICloud, this);
	if (dlg.DoModal()==IDOK)
		AddWindowsPathAsStore(dlg.m_FolderPath, L"iCloud");
}

void LFAddStoreDlg::OnBtnOneDrive()
{
	LFOneDriveDlg dlg(m_OneDrive, this);
	if (dlg.DoModal()==IDOK)
		AddWindowsPathAsStore(dlg.m_FolderPath);
}
