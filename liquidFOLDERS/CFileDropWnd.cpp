
// CFileDropWnd.cpp: Implementierungsdatei der Klasse CFileDropWnd
//

#include "stdafx.h"
#include "CFileDropWnd.h"
#include "liquidFOLDERS.h"


// CFileDropWnd
//

#define BORDER          4
#define MARGIN          15
#define ICONOFFSETX     5
#define ICONOFFSETY     4
#define FONTOFFSETY     3

CFileDropWnd::CFileDropWnd()
	: CBackstageWnd()
{
	m_rectIcon.SetRectEmpty();
}

BOOL CFileDropWnd::Create(const LPCSTR pStoreID)
{
	strcpy_s(m_StoreID, LFKeySize, pStoreID);

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, theApp.LoadStandardCursor(IDC_ARROW), NULL, theApp.LoadIcon(IDR_FILEDROP));

	CSize Size;
	GetCaptionButtonMargins(&Size);

	const INT Width = 2*BACKSTAGEBORDER+128-1;
	const INT Height = Size.cy+BACKSTAGEBORDER+128-12+MARGIN+theApp.m_DefaultFont.GetFontHeight()+FONTOFFSETY;
	m_rectIcon.SetRect((Width-128)/2+ICONOFFSETX, Size.cy+ICONOFFSETY, (Width-128)/2+ICONOFFSETX+128-9, Height-BACKSTAGEBORDER-FONTOFFSETY);

	return CBackstageWnd::Create(WS_MINIMIZEBOX, className, CString((LPCSTR)IDR_FILEDROP), _T("FileDrop"), CSize(Width, Height));
}

BOOL CFileDropWnd::HasDocumentSheet() const
{
	return FALSE;
}

void CFileDropWnd::PaintBackground(CPaintDC& pDC, CRect rect)
{
	PaintCaption(pDC, rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.bottom);
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	// Background
	CBackstageWnd::DefWindowProc(WM_PAINT, NULL, NULL);

	FillRect(dc, rect, (HBRUSH)SendMessage(WM_CTLCOLORSTATIC, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd));

	// Icon
	CPoint pt(m_rectIcon.left-ICONOFFSETX, m_rectIcon.top-ICONOFFSETY);

	theApp.m_CoreImageListJumbo.DrawEx(&dc, m_StoreIcon-1, pt, CSize(128, 128),
		CLR_NONE, CLR_NONE, ((m_StoreType & LFTypeGhosted) ? ILD_BLEND50 : ILD_TRANSPARENT) | (m_StoreType & LFTypeBadgeMask));
	DrawStoreIconShadow(dc, pt, m_StoreIcon);

	// Text
	CRect rectText(m_rectIcon);
	rectText.top = m_rectIcon.bottom-theApp.m_DefaultFont.GetFontHeight()-1;

	CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

	if (IsCtrlThemed())
	{
		dc.SetTextColor(0x000000);
		dc.DrawText(m_Store.StoreName, -1, rectText, DT_TOP | DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);

		dc.SetTextColor((m_HoverItem>=0) ? 0xFFFFFF : 0xDACCC4);
	}
	else
	{
		dc.SetTextColor(0xFFFFFF);
	}

	rectText.OffsetRect(0, 1);
	dc.DrawText(m_Store.StoreName, -1, rectText, DT_TOP | DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);

	dc.SelectObject(pOldFont);

	pDC.BitBlt(0, rect.top, rect.Width(), rect.bottom, &dc, 0, rect.top, SRCCOPY);

	dc.SelectObject(pOldBitmap);

	PaintCaption(pDC, rect);
}

void CFileDropWnd::SetTopMost(BOOL AlwaysOnTop)
{
	theApp.m_FileDropAlwaysOnTop = m_AlwaysOnTop = AlwaysOnTop;

	SetWindowPos(AlwaysOnTop ? &wndTopMost : &wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu)
		pSysMenu->CheckMenuItem(SC_ALWAYSONTOP, MF_BYCOMMAND | (AlwaysOnTop ? MF_CHECKED : MF_UNCHECKED));
}

INT CFileDropWnd::ItemAtPosition(CPoint point) const
{
	return m_rectIcon.PtInRect(point) ? 0 : -1;
}

void CFileDropWnd::ShowTooltip(const CPoint& point)
{
	theApp.ShowTooltip(this, point, m_Store);
}

BOOL CFileDropWnd::GetContextMenu(CMenu& Menu, INT Index)
{
	if (Index!=-1)
	{
		Menu.LoadMenu(IDM_STORE);

		Menu.InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);
		Menu.InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_STORE_OPENNEWWINDOW, CString((LPCSTR)IDS_CONTEXTMENU_OPENNEWWINDOW));

		return TRUE;
	}

	return FALSE;
}


BEGIN_MESSAGE_MAP(CFileDropWnd, CBackstageWnd)
	ON_WM_CREATE()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(WM_OPENFILEDROP, OnOpenFileDrop)

	ON_COMMAND(IDM_STORE_OPENNEWWINDOW, OnStoreOpenNewWindow)
	ON_COMMAND(IDM_STORE_SYNCHRONIZE, OnStoreSynchronize)
	ON_COMMAND(IDM_STORE_MAKEDEFAULT, OnStoreMakeDefault)
	ON_COMMAND(IDM_STORE_SHORTCUT, OnStoreShortcut)
	ON_COMMAND(IDM_STORE_DELETE, OnStoreDelete)
	ON_COMMAND(IDM_STORE_PROPERTIES, OnStoreProperties)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORE_OPENNEWWINDOW, IDM_STORE_PROPERTIES, OnUpdateStoreCommands)

	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StatisticsChanged, OnUpdateStore)
END_MESSAGE_MAP()

INT CFileDropWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBackstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	// SC_xxx muss sich im Bereich der Systembefehle befinden.
	ASSERT((SC_ALWAYSONTOP & 0xFFF0)==SC_ALWAYSONTOP);
	ASSERT(SC_ALWAYSONTOP<0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu)
	{
		// Always on top
		pSysMenu->InsertMenu(SC_CLOSE, MF_STRING | MF_BYCOMMAND | (m_AlwaysOnTop ? MF_CHECKED : 0), SC_ALWAYSONTOP, CString((LPCSTR)SC_ALWAYSONTOP));
		pSysMenu->InsertMenu(SC_CLOSE, MF_SEPARATOR | MF_BYCOMMAND);
	}

	// TopMost
	SetTopMost(theApp.m_FileDropAlwaysOnTop);

	// Store
	OnUpdateStore(NULL, NULL);

	// Initialize DropTarget
	m_DropTarget.SetOwner(this);
	m_DropTarget.SetStore(m_StoreID, FALSE);
	RegisterDragDrop(GetSafeHwnd(), &m_DropTarget);

	return 0;
}

void CFileDropWnd::OnNcLButtonDblClk(UINT /*nFlags*/, CPoint /*point*/)
{
	if (m_HoverItem>=0)
		OnStoreOpenNewWindow();
}

void CFileDropWnd::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0)==SC_ALWAYSONTOP)
	{
		SetTopMost(!m_AlwaysOnTop);
	}
	else
	{
		CBackstageWnd::OnSysCommand(nID, lParam);
	}
}

LRESULT CFileDropWnd::OnOpenFileDrop(WPARAM wParam, LPARAM /*lParam*/)
{
	ASSERT(wParam);

	if (strcmp((LPCSTR)wParam, m_StoreID)==0)
	{
		if (IsIconic())
			ShowWindow(SW_RESTORE);

		SetForegroundWindow();
		return 24878;
	}

	return NULL;
}


void CFileDropWnd::OnStoreOpenNewWindow()
{
	CMainWnd* pFrame = new CMainWnd();
	pFrame->CreateStore(m_Store.StoreID);
	pFrame->ShowWindow(SW_SHOW);
}

void CFileDropWnd::OnStoreSynchronize()
{
	LFRunSynchronize(m_StoreID, this);
}

void CFileDropWnd::OnStoreMakeDefault()
{
	LFErrorBox(this, LFSetDefaultStore(m_Store.StoreID));
}

void CFileDropWnd::OnStoreShortcut()
{
	LFErrorBox(this, LFCreateDesktopShortcutForStore(m_Store));
}

void CFileDropWnd::OnStoreDelete()
{
	LFDeleteStore(m_Store.StoreID, this);
}

void CFileDropWnd::OnStoreProperties()
{
	LFStorePropertiesDlg(m_Store.StoreID, this).DoModal();
}

void CFileDropWnd::OnUpdateStoreCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_STORE_SYNCHRONIZE:
		bEnable = ((m_StoreType & (LFTypeSynchronizeAllowed | LFTypeMounted | LFTypeWriteable))==(LFTypeSynchronizeAllowed | LFTypeMounted | LFTypeWriteable));
		break;

	case IDM_STORE_MAKEDEFAULT:
		bEnable = !(m_StoreType & LFTypeDefault);
		break;

	case IDM_STORE_SHORTCUT:
		bEnable = (m_StoreType & LFTypeShortcutAllowed);
		break;

	case IDM_STORE_DELETE:
		bEnable = (m_StoreType & LFTypeManageable);
		break;

	case IDM_STORE_RENAME:
		bEnable = FALSE;
		break;
	}

	pCmdUI->Enable(bEnable);
}


LRESULT CFileDropWnd::OnUpdateStore(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (LFGetStoreSettings(m_StoreID, m_Store)!=LFOk)
		PostMessage(WM_CLOSE);

	m_StoreIcon = LFGetStoreIcon(&m_Store, &m_StoreType);

	SetWindowText(m_Store.StoreName);
	Invalidate();

	return NULL;
}
