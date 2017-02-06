
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
	m_Hover = FALSE;
}

BOOL CFileDropWnd::Create(const CHAR* pStoreID)
{
	strcpy_s(m_StoreID, LFKeySize, pStoreID);

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, theApp.LoadStandardCursor(IDC_ARROW), NULL, theApp.LoadIcon(IDR_FILEDROP));

	CString Caption((LPCSTR)IDR_FILEDROP);

	CSize Size;
	GetCaptionButtonMargins(&Size);

	INT Width = 2*BACKSTAGEBORDER+128-1;
	INT Height = Size.cy+BACKSTAGEBORDER+128-12+MARGIN+theApp.m_DefaultFont.GetFontHeight()+FONTOFFSETY;
	m_rectIcon.SetRect((Width-128)/2+ICONOFFSETX, Size.cy+ICONOFFSETY, (Width-128)/2+ICONOFFSETX+128-9, Height-BACKSTAGEBORDER-FONTOFFSETY);

	return CBackstageWnd::Create(WS_MINIMIZEBOX, className, Caption, _T("FileDrop"), CSize(Width, Height));
}

BOOL CFileDropWnd::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONUP:
		LFGetApp()->HideTooltip();
		break;
	}

	return CBackstageWnd::PreTranslateMessage(pMsg);
}

BOOL CFileDropWnd::GetLayoutRect(LPRECT lpRect) const
{
	CBackstageWnd::GetLayoutRect(lpRect);

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
	theApp.m_CoreImageListJumbo.DrawEx(&dc, m_StoreIcon-1, 
		CPoint(m_rectIcon.left-ICONOFFSETX, m_rectIcon.top-ICONOFFSETY), CSize(128, 128),
		CLR_NONE, CLR_NONE, ((m_StoreType & LFTypeGhosted) ? ILD_BLEND25 : ILD_TRANSPARENT) | (m_StoreType & LFTypeBadgeMask));

	// Text
	CRect rectText(m_rectIcon);
	rectText.top = m_rectIcon.bottom-theApp.m_DefaultFont.GetFontHeight()-1;

	CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DefaultFont);

	if (IsCtrlThemed())
	{
		dc.SetTextColor(0x000000);
		dc.DrawText(m_Store.StoreName, -1, rectText, DT_TOP | DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);

		dc.SetTextColor(m_Hover ? 0xFFFFFF : 0xDACCC4);
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


BEGIN_MESSAGE_MAP(CFileDropWnd, CBackstageWnd)
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(WM_OPENFILEDROP, OnOpenFileDrop)

	ON_COMMAND(IDM_ITEM_OPENNEWWINDOW, OnStoreOpen)
	ON_COMMAND(IDM_STORE_SYNCHRONIZE, OnStoreSynchronize)
	ON_COMMAND(IDM_STORE_MAKEDEFAULT, OnStoreMakeDefault)
	ON_COMMAND(IDM_STORE_IMPORTFOLDER, OnStoreImportFolder)
	ON_COMMAND(IDM_STORE_SHORTCUT, OnStoreShortcut)
	ON_COMMAND(IDM_STORE_DELETE, OnStoreDelete)
	ON_COMMAND(IDM_STORE_PROPERTIES, OnStoreProperties)
	ON_UPDATE_COMMAND_UI(IDM_ITEM_OPENNEWWINDOW, OnUpdateStoreCommands)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORE_SYNCHRONIZE, IDM_STORE_PROPERTIES, OnUpdateStoreCommands)

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
		CString tmpStr((LPCSTR)SC_ALWAYSONTOP);
		pSysMenu->InsertMenu(SC_CLOSE, MF_STRING | MF_BYCOMMAND | (m_AlwaysOnTop ? MF_CHECKED : 0), SC_ALWAYSONTOP, tmpStr);
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

void CFileDropWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	CBackstageWnd::OnMouseMove(nFlags, point);

	BOOL Hover = m_rectIcon.PtInRect(point);
	if (Hover!=m_Hover)
	{
		if (Hover)
		{
			TRACKMOUSEEVENT tme;
			ZeroMemory(&tme, sizeof(tme));
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE | TME_HOVER;
			tme.dwHoverTime = HOVERTIME;
			tme.hwndTrack = m_hWnd;
			TrackMouseEvent(&tme);
		}
		else
		{
			LFGetApp()->HideTooltip();
		}

		m_Hover = Hover;
		Invalidate();
	}
}

void CFileDropWnd::OnMouseLeave()
{
	CBackstageWnd::OnMouseLeave();

	LFGetApp()->HideTooltip();

	m_Hover = FALSE;
	Invalidate();
}

void CFileDropWnd::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if (!LFGetApp()->IsTooltipVisible() && m_Hover)
		{
			LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptorEx(&m_Store);

			CString Hint;
			GetHintForStore(Hint, pItemDescriptor);

			LFGetApp()->ShowTooltip(this, point, m_Store.StoreName, Hint, LFGetApp()->m_CoreImageListExtraLarge.ExtractIcon(m_StoreIcon-1));

			LFFreeItemDescriptor(pItemDescriptor);
		}
	}
	else
	{
		LFGetApp()->HideTooltip();
	}
}

void CFileDropWnd::OnNcLButtonDblClk(UINT /*nFlags*/, CPoint /*point*/)
{
	if (m_Hover)
		OnStoreOpen();
}

void CFileDropWnd::OnRButtonUp(UINT /*nFlags*/, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CFileDropWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint pos)
{
	if ((pos.x<0) || (pos.y<0))
	{
		CRect rect;
		GetClientRect(rect);

		pos.x = (rect.left+rect.right)/2;
		pos.y = (rect.top+rect.bottom)/2;
		ClientToScreen(&pos);
	}

	CPoint point(pos);
	ScreenToClient(&point);

	if (m_rectIcon.PtInRect(point))
	{
		// Store
		CMenu menu;
		ENSURE(menu.LoadMenu(IDM_STORE));

		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT_VALID(pPopup);

		pPopup->InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);

		CString tmpStr((LPCSTR)IDS_CONTEXTMENU_OPENNEWWINDOW);
		pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_ITEM_OPENNEWWINDOW, tmpStr);

		pPopup->SetDefaultItem(IDM_ITEM_OPENNEWWINDOW);
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, this);

		return;
	}

	// System menu
	CMenu* pMenu = GetSystemMenu(FALSE);
	if (pMenu)
		SendMessage(WM_SYSCOMMAND, (WPARAM)pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pos.x, pos.y, this));
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

	if (strcmp((CHAR*)wParam, m_StoreID)==0)
	{
		if (IsIconic())
			ShowWindow(SW_RESTORE);

		SetForegroundWindow();
		return 24878;
	}

	return NULL;
}


void CFileDropWnd::OnStoreOpen()
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

void CFileDropWnd::OnStoreImportFolder()
{
	LFImportFolder(m_Store.StoreID, this);
}

void CFileDropWnd::OnStoreShortcut()
{
	LFCreateDesktopShortcutForStoreEx(&m_Store);
}

void CFileDropWnd::OnStoreDelete()
{
	LFDeleteStore(m_Store.StoreID, this);
}

void CFileDropWnd::OnStoreProperties()
{
	LFStorePropertiesDlg dlg(m_Store.StoreID, this);
	dlg.DoModal();
}

void CFileDropWnd::OnUpdateStoreCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_STORE_SYNCHRONIZE:
		bEnable = (m_StoreType & LFTypeSynchronizeAllowed);
		break;

	case IDM_STORE_MAKEDEFAULT:
		bEnable = !(m_StoreType & LFTypeDefault);
		break;

	case IDM_STORE_IMPORTFOLDER:
		bEnable = (m_StoreType & LFTypeMounted);
		break;

	case IDM_STORE_SHORTCUT:
		bEnable = (m_StoreType & LFTypeShortcutAllowed);
		break;

	case IDM_STORE_RENAME:
		bEnable = FALSE;
		break;
	}

	pCmdUI->Enable(bEnable);
}


LRESULT CFileDropWnd::OnUpdateStore(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (LFGetStoreSettings(m_StoreID, &m_Store)!=LFOk)
		PostMessage(WM_CLOSE);

	m_StoreIcon = LFGetStoreIcon(&m_Store, &m_StoreType);

	SetWindowText(m_Store.StoreName);
	Invalidate();

	return NULL;
}
