
// MigrateWnd.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "Migrate.h"
#include "MigrateWnd.h"
#include "Resource.h"
#include "LFCore.h"
#include <io.h>


CMigrateWnd::CMigrateWnd()
	: CGlasWindow()
{
	m_hIcon = NULL;
}

CMigrateWnd::~CMigrateWnd()
{
	if (m_hIcon)
		DestroyIcon(m_hIcon);
}

BOOL CMigrateWnd::Create()
{
	m_hIcon = theApp.LoadIcon(IDR_APPLICATION);

	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW), NULL, m_hIcon);

	CRect rect;
	SystemParametersInfo(SPI_GETWORKAREA, NULL, &rect, NULL);
	rect.DeflateRect(32, 32);

	CString caption;
	ENSURE(caption.LoadString(IDR_APPLICATION));

	return CGlasWindow::Create(WS_MINIMIZEBOX | WS_MAXIMIZEBOX, className, caption, rect);
}

void CMigrateWnd::AdjustLayout()
{
	if (!IsWindow(m_wndFolder.GetSafeHwnd()))
		return;
	if (!IsWindow(m_wndStore.GetSafeHwnd()))
		return;
	if (!IsWindow(m_wndBottomArea.GetSafeHwnd()))
		return;
	if (!IsWindow(m_wndMainView.GetSafeHwnd()))
		return;

	CRect rect;
	GetLayoutRect(rect);

	const UINT Border = 4;
	const UINT SelectorHeight = m_wndFolder.GetPreferredHeight();
	m_wndFolder.SetWindowPos(NULL, rect.left, rect.top+4, (rect.Width()-Border)/2, SelectorHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndStore.SetWindowPos(NULL, rect.right-(rect.Width()-Border)/2, rect.top+4, (rect.Width()-Border)/2, SelectorHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	const UINT BottomHeight = MulDiv(45, LOWORD(GetDialogBaseUnits()), 8);
	m_wndBottomArea.SetWindowPos(NULL, rect.left, rect.bottom-BottomHeight, rect.Width(), BottomHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	m_wndMainView.SetDesign(GetDesign());
	m_wndMainView.SetWindowPos(NULL, rect.left, rect.top+m_Margins.cyTopHeight, rect.Width(), rect.bottom-BottomHeight-m_Margins.cyTopHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}


BEGIN_MESSAGE_MAP(CMigrateWnd, CGlasWindow)
	ON_WM_CREATE()
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
/*	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnStoresChanged)*/
END_MESSAGE_MAP()

int CMigrateWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlasWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Folder selector
	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_FOLDER_HINT));
	m_wndFolder.Create(tmpStr, this, 1);

	// Store selector
	ENSURE(tmpStr.LoadString(IDS_STORE_HINT));
	m_wndStore.Create(tmpStr, this, 2);

	// Bottom area
	m_wndBottomArea.Create(this, MAKEINTRESOURCE(IDD_BOTTOMAREA), CBRS_BOTTOM, 3);

	// Main view
	m_wndMainView.Create(this, 4);

	// Aero
	MARGINS Margins = { 0, 0, m_wndFolder.GetPreferredHeight()+11, 0 };
	UseGlasBackground(Margins);

	m_wndBottomArea.SetFocus();
	AdjustLayout();
	return 0;
}

void CMigrateWnd::OnIdleUpdateCmdUI()
{
}

/*
LRESULT CMigrateWnd::OnStoresChanged(WPARAM wParam, LPARAM lParam)
{
	return NULL;
}*/
