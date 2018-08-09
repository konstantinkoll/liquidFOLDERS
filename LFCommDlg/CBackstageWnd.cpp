
// CBackstageWnd.cpp: Implementierung der Klasse CBackstageWnd
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include <dwmapi.h>
#include <propkey.h>
#include <propsys.h>


// CBackstageWnd
//

#define BACKGROUNDTOP     100
#define GRIPPEREDGE       4
#define TOPALPHA          0x10
#define BOTTOMALPHA       0x50

HBRUSH hBackgroundTop = NULL;
HBRUSH hBackgroundBottom = NULL;

const FMTID AppUserModel =  { 0x9F4C2855, 0x9F79, 0x4B39, { 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3 } };
const FMTID IID_ITaskbarList3 = { 0xEA1AFB91, 0x9E28, 0x4B86, {0x90, 0xE9, 0x9E, 0x9F, 0x8A, 0x5E, 0xEF, 0xAF}};

const PROPERTYKEY AppUserModelID = { AppUserModel, 5 };
const PROPERTYKEY AppUserModelPreventPinning = { AppUserModel, 9 };

CBackstageWnd::CBackstageWnd(BOOL IsDialog, BOOL WantsBitmap)
	: CFrontstageWnd()
{
	m_IsDialog = m_ShowCaption = IsDialog;
	m_WantsBitmap = WantsBitmap;

	hAccelerator = NULL;
	m_pDropTarget = NULL;
	m_pSidebarWnd = NULL;
	m_ShowExpireCaption = FALSE;
	m_SidebarWidth = m_BottomDivider = m_BackBufferL = m_BackBufferH = m_RegionWidth = m_RegionHeight = 0;
	hBackgroundBrush = NULL;
	m_pTaskbarList3 = NULL;
}

BOOL CBackstageWnd::Create(DWORD dwStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName, LPCTSTR lpszPlacementPrefix, const CSize& Size, BOOL ShowCaption, CBackstageDropTarget* pDropTarget)
{
	m_PlacementPrefix = lpszPlacementPrefix;
	m_ShowCaption = ShowCaption;
	m_ShowExpireCaption = LFIsSharewareExpired();
	m_pDropTarget = pDropTarget;

	// Get size of work area
	CRect rect;
	SystemParametersInfo(SPI_GETWORKAREA, NULL, &rect, NULL);

	// Window placement
	WINDOWPLACEMENT WindowPlacement;
	LFGetApp()->GetBinary(m_PlacementPrefix+_T("WindowPlacement"), &WindowPlacement, sizeof(WindowPlacement));

	if (WindowPlacement.length==sizeof(WindowPlacement))
	{
		if ((Size.cx>0) && (Size.cy>0))
		{
			if (WindowPlacement.rcNormalPosition.left+Size.cx>rect.right)
				WindowPlacement.rcNormalPosition.left = rect.right-Size.cx;

			if (WindowPlacement.rcNormalPosition.top+Size.cy>rect.bottom)
				WindowPlacement.rcNormalPosition.top = rect.bottom-Size.cy;

			WindowPlacement.rcNormalPosition.right = WindowPlacement.rcNormalPosition.left+Size.cx;
			WindowPlacement.rcNormalPosition.bottom = WindowPlacement.rcNormalPosition.top+Size.cy;
		}

		if (WindowPlacement.showCmd!=SW_MAXIMIZE)
		{
			rect.SetRect(WindowPlacement.rcNormalPosition.left, WindowPlacement.rcNormalPosition.top, WindowPlacement.rcNormalPosition.right, WindowPlacement.rcNormalPosition.bottom);

			WindowPlacement.showCmd = SW_HIDE;
		}
	}
	else
	{
		// Adjust work area
		rect.DeflateRect(50, 50);

		if ((Size.cx<0) || (Size.cy<0))
		{
			rect.left = rect.right-rect.Width()/3;
			rect.top = rect.bottom-rect.Height()/2;

			rect.OffsetRect(16, 16);
		}
		else
			if ((Size.cx>0) && (Size.cy>0))
			{
				rect.right = (rect.left=(rect.left+rect.right)/2-Size.cx)+Size.cx;
				rect.bottom = (rect.top=(rect.top+rect.bottom)/2-Size.cy)+Size.cy;
			}
	}

	// Create window
	if (!CFrontstageWnd::CreateEx(WS_EX_APPWINDOW | WS_EX_CONTROLPARENT | WS_EX_OVERLAPPEDWINDOW, lpszClassName, lpszWindowName,
		dwStyle | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, rect, NULL, 0))
		return FALSE;

	if (WindowPlacement.length==sizeof(WindowPlacement))
		SetWindowPlacement(&WindowPlacement);

	// Adjust layout
	AdjustLayout();

	return TRUE;
}

BOOL CBackstageWnd::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message==WM_KEYDOWN)
	{
		CWnd* pWnd;
		BOOL WindowManagement = (LFGetApp()->OSVersion<=OS_Vista) && ((GetKeyState(VK_LWIN)<0) || (GetKeyState(VK_RWIN)<0));

		switch (pMsg->wParam)
		{
		case VK_TAB:
			if (!m_IsDialog)
			{
				if ((pWnd=GetNextDlgTabItem(GetFocus(), GetKeyState(VK_SHIFT)<0))!=NULL)
					pWnd->SetFocus();

				return TRUE;
			}

			break;

		// Simulate Aero Snap on Windows XP and Vista

		case VK_UP:
			if (WindowManagement && (IsIconic() || (GetStyle() & WS_THICKFRAME)))
			{
				ShowWindow(IsIconic() ? SW_RESTORE : SW_MAXIMIZE);

				return TRUE;
			}

			break;

		case VK_DOWN:
			if (WindowManagement && (IsZoomed() || (GetStyle() & WS_MINIMIZEBOX)))
			{
				ShowWindow(IsZoomed() ? SW_RESTORE : SW_MINIMIZE);

				return TRUE;
			}

			break;

		case VK_LEFT:
		case VK_RIGHT:
			if (WindowManagement && !IsZoomed() && !IsIconic() && (GetStyle() & WS_THICKFRAME))
			{
				CRect rect;
				if (SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0))
				{
					if (pMsg->wParam==VK_LEFT)
					{
						rect.right -= rect.Width()/2;
					}
					else
					{
						rect.left += rect.Width()/2;
					}

					SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

					return TRUE;
				}
			}

			break;
		}
	}

	// Accelerators
	if (hAccelerator)
		if ((pMsg->message>=WM_KEYFIRST) && (pMsg->message<=WM_KEYLAST))
			if (TranslateAccelerator(m_hWnd, hAccelerator, pMsg))
				return TRUE;

	return CFrontstageWnd::PreTranslateMessage(pMsg);
}

LRESULT CBackstageWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// Prevent default caption bar from being drawn
	if (message==WM_SETTEXT)
	{
		LONG lStyle = GetWindowLong(m_hWnd, GWL_STYLE);
		SetWindowLong(m_hWnd, GWL_STYLE, lStyle & ~WS_VISIBLE);

		CFrontstageWnd::WindowProc(message, wParam, lParam);

		SetWindowLong(m_hWnd, GWL_STYLE, lStyle);

		InvalidateCaption();

		return NULL;
	}

	return CFrontstageWnd::WindowProc(message, wParam, lParam);
}

BOOL CBackstageWnd::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (CFrontstageWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return LFGetApp()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CBackstageWnd::DisableTaskbarPinning(LPCWSTR UserModelID)
{
	if (LFGetApp()->m_ShellLibLoaded)
	{
		ASSERT(UserModelID);

		IPropertyStore *pPropertyStore;
		if (SUCCEEDED(LFGetApp()->zGetPropertyStoreForWindow(GetSafeHwnd(), IID_PPV_ARGS(&pPropertyStore))))
		{
			// User Model ID
			PROPVARIANT VariantID;
			VariantID.vt = VT_BSTR;
			VariantID.pwszVal = (LPWSTR)UserModelID;

			pPropertyStore->SetValue(AppUserModelID, VariantID);

			// Prevent pinning
			PROPVARIANT VariantPinning;
			VariantPinning.vt = VT_BOOL;
			VariantPinning.boolVal = VARIANT_TRUE;

			pPropertyStore->SetValue(AppUserModelPreventPinning, VariantPinning);

			// Release property store
			pPropertyStore->Release();
		}
	}
}

void CBackstageWnd::SetSidebar(CBackstageSidebar* pSidebarWnd)
{
	ASSERT(pSidebarWnd);
	ASSERT(!m_pSidebarWnd);

	m_pSidebarWnd = pSidebarWnd;
}

void CBackstageWnd::GetCaptionButtonMargins(LPSIZE lpSize) const
{
	ASSERT(lpSize);

	lpSize->cx = 0;
	lpSize->cy = BACKSTAGEBORDER-2;

	m_wndWidgets.AddWidgetSize(lpSize);

	if (lpSize->cy<BACKSTAGERADIUS+1)
		lpSize->cy = BACKSTAGERADIUS+1;
}

INT CBackstageWnd::GetCaptionHeight(BOOL IncludeBottomMargin) const
{
	CSize Size;
	GetCaptionButtonMargins(&Size);

	return max((m_ShowCaption || m_ShowExpireCaption) ? LFGetApp()->m_SmallBoldFont.GetFontHeight()+(IncludeBottomMargin ? 2 : 1)*BACKSTAGECAPTIONMARGIN : 0, Size.cy);
}

BOOL CBackstageWnd::HasDocumentSheet() const
{
	return TRUE;
}

void CBackstageWnd::GetLayoutRect(LPRECT lpRect)
{
	ASSERT(lpRect);

	GetClientRect(lpRect);
	lpRect->top = GetCaptionHeight();

	if (m_pSidebarWnd)
		lpRect->left += (m_SidebarWidth=m_pSidebarWnd->GetPreferredWidth(m_IsDialog ? -1 : HasDocumentSheet() ? lpRect->right/5 : lpRect->right));
}

void CBackstageWnd::AdjustLayout(const CRect& /*rectLayout*/, UINT /*nFlags*/)
{
}

void CBackstageWnd::AdjustLayout(UINT nFlags)
{
	CRect rectClient;
	GetClientRect(rectClient);

	// Sidebar
	CRect rectLayout;
	GetLayoutRect(rectLayout);

	if (m_pSidebarWnd)
	{
		m_pSidebarWnd->SetShadow(HasDocumentSheet());
		m_pSidebarWnd->SetWindowPos(NULL, 0, rectLayout.top, m_SidebarWidth, rectLayout.bottom-rectLayout.top, nFlags | SWP_SHOWWINDOW);
	}

	// Widgets
	CSize Size(0, 0);
	m_wndWidgets.AddWidgetSize(&Size);

	m_wndWidgets.SetWindowPos(NULL, rectClient.right-Size.cx, -1, Size.cx+1, Size.cy+1, nFlags);

	// Frontstage
	AdjustLayout(rectLayout, nFlags);
}

BOOL CBackstageWnd::IsBackstageControl(CWnd* pWnd)
{
	ASSERT(pWnd);

	const DWORD dwStyle = pWnd->GetStyle();

	if (!(dwStyle & WS_VISIBLE))
		return FALSE;

	return ((dwStyle & WS_BORDER) || (pWnd->SendMessage(WM_GETDLGCODE) & DLGC_HASSETSEL));
}

void CBackstageWnd::PaintOnBackground(CDC& /*dc*/, Graphics& /*g*/, const CRect& /*rectLayout*/)
{
}

void CBackstageWnd::UpdateBackground()
{
	CRect rectClient;
	GetClientRect(rectClient);

	if ((m_BackBufferL!=rectClient.Width()) || (m_BackBufferH!=rectClient.Height()))
	{
		m_BackBufferL = rectClient.Width();
		m_BackBufferH = rectClient.Height();

		DeleteObject(hBackgroundBrush);

		const BOOL Themed = IsCtrlThemed();
		if (m_WantsBitmap || Themed)
		{
			CDC dc;
			dc.CreateCompatibleDC(NULL);
			dc.SetBkMode(TRANSPARENT);

			HBITMAP hBitmap = CreateTransparentBitmap(m_BackBufferL, m_BackBufferH);
			HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

			Graphics g(dc);
			g.SetSmoothingMode(SmoothingModeAntiAlias);
			g.SetPixelOffsetMode(PixelOffsetModeHalf);

			if (Themed)
			{
				// Prepare textures
				PrepareBitmaps();

				// Layout
				CRect rectLayout;
				GetLayoutRect(rectLayout);

				// Background
				FillRect(dc, CRect(0, 0, m_BackBufferL, min(m_BackBufferH, BACKGROUNDTOP)), hBackgroundTop);

				if (m_BackBufferH>BACKGROUNDTOP)
					FillRect(dc, CRect(0, BACKGROUNDTOP, m_BackBufferL, m_BackBufferH), hBackgroundBottom);

				if (m_IsDialog)
				{
					dc.FillSolidRect(rectLayout, 0xFFFFFF);

					Bitmap* pDivider = LFGetApp()->GetCachedResourceImage(IDB_DIVDOWN);

					const INT X = (rectLayout.Width()-(INT)pDivider->GetWidth())/2;
					g.DrawImage(pDivider, rectLayout.left+max(0, X), m_BottomDivider, max(0, -X), 0, min(rectLayout.Width(), (INT)pDivider->GetWidth()), pDivider->GetHeight(), UnitPixel);
				}

				// Top border
				const INT LayoutWidth = HasDocumentSheet() ? m_BackBufferL : m_SidebarWidth;
				if (LayoutWidth>0)
				{
					LinearGradientBrush brush(Point(0, rectLayout.top-3), Point(0, rectLayout.top), Color(0x00000000), Color(0x60000000));
					g.FillRectangle(&brush, 0, rectLayout.top-2, LayoutWidth, 2);
				}

				PaintOnBackground(dc, g, rectLayout);

				// Child windows
				CWnd* pChildWnd = GetWindow(GW_CHILD);

				while (pChildWnd)
				{
					if (IsBackstageControl(pChildWnd))
					{
						CRect rectBounds;
						pChildWnd->GetWindowRect(rectBounds);
						ScreenToClient(rectBounds);

						if (!m_IsDialog || (rectBounds.bottom<rectLayout.top))
						{
							if (pChildWnd==&m_wndWidgets)
							{
								rectBounds.top -= BACKSTAGERADIUS;
								rectBounds.right += BACKSTAGERADIUS;
							}

							DrawMilledRectangle(g, rectBounds);
						}
					}

					pChildWnd = pChildWnd->GetWindow(GW_HWNDNEXT);
				}

				// Top reflection
				GraphicsPath path;
				Pen pen(Color(0xFF000000));

				for (UINT a=0; a<3; a++)
				{
					CreateRoundTop(CRect(0, a, m_BackBufferL, BACKSTAGERADIUS-3+a), BACKSTAGERADIUS-3-a, path);

					LinearGradientBrush brush(Point(0, a-1), Point(0, BACKSTAGERADIUS+a-2), Color((0x40000000>>a) | 0xFFFFFF), Color(0x00FFFFFF));

					pen.SetBrush(&brush);
					g.DrawPath(&pen, &path);
				}

				// Outline
				DrawWindowEdge(g, Themed);

				if (IsZoomed())
				{
					CRect rectOutline(-1, -1, m_BackBufferL+1, m_BackBufferH+1);
					CreateRoundTop(rectOutline, BACKSTAGERADIUS, path);

					path.AddLine(m_BackBufferL, -1, -1, -1);
					path.CloseFigure();

					SolidBrush brush(Color(0xFF000000));
					g.FillPath(&brush, &path);
				}
			}
			else
			{
				CRect rectLayout;
				GetLayoutRect(rectLayout);

				if (m_IsDialog)
				{
					FillRect(dc, CRect(0, 0, m_BackBufferL, rectLayout.top), (HBRUSH)GetStockObject(DKGRAY_BRUSH));
					FillRect(dc, CRect(0, rectLayout.top, rectLayout.left, rectLayout.bottom), (HBRUSH)GetStockObject(DKGRAY_BRUSH));

					if (m_BottomDivider)
					{
						dc.FillSolidRect(rectLayout.left, rectLayout.top, rectLayout.Width(), m_BottomDivider-rectLayout.top, GetSysColor(COLOR_WINDOW));
						dc.FillSolidRect(rectLayout.left, m_BottomDivider, rectLayout.Width(), m_BackBufferH-m_BottomDivider, GetSysColor(COLOR_3DFACE));
					}
					else
					{
						dc.FillSolidRect(rectLayout, GetSysColor(COLOR_WINDOW));
					}
				}
				else
				{
					FillRect(dc, CRect(0, 0, m_BackBufferL, m_BackBufferH), (HBRUSH)GetStockObject(DKGRAY_BRUSH));
				}

				PaintOnBackground(dc, g, rectLayout);
			}

			dc.SelectObject(hOldBitmap);

			hBackgroundBrush = CreatePatternBrush(hBitmap);
			DeleteObject(hBitmap);
		}
		else
		{
			hBackgroundBrush = NULL;
		}
	}
}

void CBackstageWnd::PaintBackground(CPaintDC& pDC, CRect rect)
{
	PaintCaption(pDC, rect);

	if (m_IsDialog)
	{
		if (hBackgroundBrush)
		{
			FillRect(pDC, rect, hBackgroundBrush);
		}
		else
		{
			CRect rectLayout;
			GetLayoutRect(rectLayout);

			FillRect(pDC, CRect(0, rect.top, m_BackBufferL, rectLayout.top), (HBRUSH)GetStockObject(BLACK_BRUSH));
			FillRect(pDC, CRect(0, rectLayout.top, rectLayout.left, rectLayout.bottom), (HBRUSH)GetStockObject(DKGRAY_BRUSH));

			if (m_BottomDivider)
			{
				pDC.FillSolidRect(rectLayout.left, rectLayout.top, rectLayout.Width(), m_BottomDivider-rectLayout.top, GetSysColor(COLOR_WINDOW));
				pDC.FillSolidRect(rectLayout.left, m_BottomDivider, rectLayout.Width(), m_BackBufferH-m_BottomDivider, GetSysColor(COLOR_3DFACE));
			}
			else
			{
				pDC.FillSolidRect(rectLayout, GetSysColor(COLOR_WINDOW));
			}
		}
	}
	else
	{
		FillRect(pDC, rect, hBackgroundBrush ? hBackgroundBrush : (HBRUSH)GetStockObject(DKGRAY_BRUSH));
	}
}

void CBackstageWnd::PaintCaption(CPaintDC& pDC, CRect& rect)
{
	if (m_ShowCaption || m_ShowExpireCaption)
	{
		const INT CaptionHeight = GetCaptionHeight();

		CRect rectCaption(0, 0, rect.Width(), CaptionHeight);

		CDC dc;
		dc.CreateCompatibleDC(&pDC);
		dc.SetBkMode(TRANSPARENT);

		CBitmap MemBitmap;
		MemBitmap.CreateCompatibleBitmap(&pDC, rectCaption.Width(), rectCaption.Height());
		CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

		const BOOL Themed = IsCtrlThemed();

		// Background
		FillRect(dc, rectCaption, hBackgroundBrush ? hBackgroundBrush : (HBRUSH)GetStockObject(DKGRAY_BRUSH));

		// Caption
		CString Caption;

		if (m_ShowExpireCaption)
		{
			ENSURE(Caption.LoadString(IDS_EXPIRECAPTION));
		}
		else
		{
			GetWindowText(Caption);
		}

		CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_SmallBoldFont);

		CSize Size;
		GetCaptionButtonMargins(&Size);

		CRect rectText(BACKSTAGEBORDER, -1, rect.Width()-Size.cx-BACKSTAGEBORDER, CaptionHeight-1);

		if (Themed)
		{
			dc.SetTextColor(0x000000);
			dc.DrawText(Caption, rectText, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
		}

		rectText.OffsetRect(0, 1);

		dc.SetTextColor(Themed ? m_ShowExpireCaption ? 0x4840F0 : IsWindowEnabled() ? 0xDACCC4 : 0x998981 : m_ShowExpireCaption ? 0x0000FF : GetSysColor(IsWindowEnabled() ? COLOR_3DFACE : COLOR_3DSHADOW));
		dc.DrawText(Caption, rectText, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

		dc.SelectObject(pOldFont);

		pDC.BitBlt(0, 0, rectCaption.Width(), CaptionHeight, &dc, 0, 0, SRCCOPY);

		dc.SelectObject(pOldBitmap);

		rect.top += CaptionHeight;
	}
}

void CBackstageWnd::InvalidateCaption(BOOL Background)
{
	if (m_ShowCaption)
	{
		CRect rect;
		GetClientRect(rect);

		rect.bottom = GetCaptionHeight();

		if (Background)
			m_BackBufferL = m_BackBufferH = 0;

		InvalidateRect(rect);
	}
}

void CBackstageWnd::UpdateRegion(INT cx, INT cy)
{
	if (IsZoomed() || !IsCtrlThemed())
	{
		SetWindowRgn(CreateRectRgn(0, 0, cx, cy), TRUE);

		m_RegionWidth = m_RegionHeight = -1;
	}
	else
	{
		if ((cx==-1) || (cy==-1))
		{
			CRect rectWindow;
			GetWindowRect(rectWindow);

			cx = rectWindow.Width();
			cy = rectWindow.Height();
		}

		if ((cx!=m_RegionWidth) || (cy!=m_RegionHeight))
		{
			SetWindowRgn(CreateRoundRectRgn(0, 0, cx+1, cy+1, 2*(BACKSTAGERADIUS-3), 2*(BACKSTAGERADIUS-3)), TRUE);

			m_RegionWidth = cx;
			m_RegionHeight = cy;
		}
	}
}

void CBackstageWnd::PostNcDestroy()
{
	if (!m_IsDialog)
		delete this;
}

void CBackstageWnd::PrepareBitmaps()
{
	if (!hBackgroundTop)
	{
		Bitmap* pTexture = LFGetApp()->GetCachedResourceImage(IDB_BACKGROUND_DARKLINEN);

		CDC dc;
		dc.CreateCompatibleDC(NULL);

		CBitmap MemBitmap;
		MemBitmap.CreateBitmap(pTexture->GetWidth(), BACKGROUNDTOP, 1, 32, NULL);
		CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

		Graphics g(dc);

		g.DrawImage(pTexture, 0, 0);

		LinearGradientBrush brush(Point(0, 0), Point(0, BACKGROUNDTOP), Color(TOPALPHA<<24), Color(BOTTOMALPHA<<24));
		g.FillRectangle(&brush, 0, 0, pTexture->GetWidth(), BACKGROUNDTOP);

		dc.SelectObject(pOldBitmap);

		hBackgroundTop = CreatePatternBrush(MemBitmap);
	}

	if (!hBackgroundBottom)
	{
		Bitmap* pTexture = LFGetApp()->GetCachedResourceImage(IDB_BACKGROUND_DARKLINEN);

		CDC dc;
		dc.CreateCompatibleDC(NULL);

		CBitmap MemBitmap;
		MemBitmap.CreateBitmap(pTexture->GetWidth(), pTexture->GetHeight(), 1, 32, NULL);
		CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

		Graphics g(dc);

		g.DrawImage(pTexture, 0, 0);

		SolidBrush brush(Color(BOTTOMALPHA<<24));
		g.FillRectangle(&brush, 0, 0, pTexture->GetWidth(), pTexture->GetHeight());

		dc.SelectObject(pOldBitmap);

		hBackgroundBottom = CreatePatternBrush(MemBitmap);
	}
}


BEGIN_MESSAGE_MAP(CBackstageWnd, CFrontstageWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_NCCALCSIZE, OnNcCalcSize)
	ON_WM_NCHITTEST()
	ON_WM_NCPAINT()
	ON_WM_NCACTIVATE()
	ON_WM_ENABLE()
	ON_MESSAGE(WM_GETTITLEBARINFOEX, OnGetTitleBarInfoEx)
	ON_WM_PAINT()
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_WM_THEMECHANGED()
	ON_WM_DWMCOMPOSITIONCHANGED()
	ON_WM_CTLCOLOR()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_CONTEXTMENU()
	ON_REGISTERED_MESSAGE(LFGetApp()->m_TaskbarButtonCreated, OnTaskbarButtonCreated)
	ON_REGISTERED_MESSAGE(LFGetApp()->m_LicenseActivatedMsg, OnLicenseActivated)
	ON_REGISTERED_MESSAGE(LFGetApp()->m_SetProgressMsg, OnSetProgress)
	ON_REGISTERED_MESSAGE(LFGetApp()->m_WakeupMsg, OnWakeup)
	ON_WM_COPYDATA()
END_MESSAGE_MAP()

INT CBackstageWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	LFGetApp()->HideTooltip();
	LFGetApp()->AddFrame(this);

	ModifyStyle(0, WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	// System menu
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu)
	{
		if (!(GetStyle() & WS_THICKFRAME))
		{
			pSysMenu->DeleteMenu(SC_MAXIMIZE, MF_BYCOMMAND);
			pSysMenu->DeleteMenu(SC_SIZE, MF_BYCOMMAND);
		}
		else
		{
			if (!(GetStyle() & WS_MAXIMIZEBOX))
				pSysMenu->DeleteMenu(SC_MAXIMIZE, MF_BYCOMMAND);
		}

		if (!(GetStyle() & WS_MINIMIZEBOX))
			pSysMenu->DeleteMenu(SC_MINIMIZE, MF_BYCOMMAND);
	}

	// Textures
	if (IsCtrlThemed())
		PrepareBitmaps();

	// Shadow
	m_wndShadow.Create();

	// Widets
	m_wndWidgets.Create(this, (UINT)-1);

	// Message filter
	if (LFGetApp()->m_UserLibLoaded)
		LFGetApp()->zChangeWindowMessageFilter(GetSafeHwnd(), LFGetApp()->m_TaskbarButtonCreated, MSGFLT_ADD);

	// Drop target
	if (m_pDropTarget)
		m_pDropTarget->Register(this);

	// Finish
	OnCompositionChanged();
	UpdateRegion();

	return 0;
}

void CBackstageWnd::OnClose()
{
	if (!m_IsDialog)
	{
		WINDOWPLACEMENT WindowPlacement;
		WindowPlacement.length = sizeof(WindowPlacement);

		if (GetWindowPlacement(&WindowPlacement))
			LFGetApp()->WriteBinary(m_PlacementPrefix+_T("WindowPlacement"), (LPBYTE)&WindowPlacement, sizeof(WindowPlacement));
	}

	CFrontstageWnd::OnClose();
}

void CBackstageWnd::OnDestroy()
{
	LFGetApp()->HideTooltip();

	if (m_pDropTarget)
	{
		m_pDropTarget->Revoke();
		delete m_pDropTarget;
	}

	if (m_pTaskbarList3)
		m_pTaskbarList3->Release();

	DeleteObject(hBackgroundBrush);

	CFrontstageWnd::OnDestroy();

	LFGetApp()->KillFrame(this);
}

LRESULT CBackstageWnd::OnNcCalcSize(WPARAM wParam, LPARAM lParam)
{
	// Valid for !wParam and wParam
	NCCALCSIZE_PARAMS* lpncsp = (NCCALCSIZE_PARAMS*)lParam;

	if (IsZoomed())
	{
		const INT cx = GetSystemMetrics(SM_CXFRAME);
		lpncsp->rgrc[0].left += cx;
		lpncsp->rgrc[0].right -= cx;

		const INT cy = GetSystemMetrics(SM_CYFRAME);
		lpncsp->rgrc[0].top += cy;
		lpncsp->rgrc[0].bottom -= cy;
	}
	else
		if (!IsCtrlThemed())
		{
			lpncsp->rgrc[0].left++;
			lpncsp->rgrc[0].right--;
			lpncsp->rgrc[0].top++;
			lpncsp->rgrc[0].bottom--;
		}

	if (wParam)
		if ((lpncsp->rgrc[0].bottom>=0) && (lpncsp->rgrc[0].right>=0))
		{
			BOOL IsCompositionEnabled = FALSE;
			if (LFGetApp()->m_DwmLibLoaded)
				LFGetApp()->zDwmIsCompositionEnabled(&IsCompositionEnabled);

			if (!IsCompositionEnabled)
			{
				ZeroMemory(&lpncsp->rgrc[1], 2*sizeof(RECT));
				return WVR_VALIDRECTS;
			}
		}

	return 0;
}

LRESULT CBackstageWnd::OnNcHitTest(CPoint point)
{
	LRESULT HitTest = CWnd::OnNcHitTest(point);

	if ((HitTest==HTCLIENT) || (HitTest==HTCLOSE) || (HitTest==HTMINBUTTON) || (HitTest==HTMAXBUTTON))
	{
		if (!IsZoomed() && (GetStyle() & WS_THICKFRAME))
		{
			CRect rectWindow;
			GetWindowRect(rectWindow);

			BOOL Top = (point.y<rectWindow.top+BACKSTAGEGRIPPER);
			BOOL Bottom = (point.y>=rectWindow.bottom-BACKSTAGEGRIPPER);
			BOOL Left = (point.x<rectWindow.left+GRIPPEREDGE*BACKSTAGEGRIPPER);
			BOOL Right = (point.x>=rectWindow.right-GRIPPEREDGE*BACKSTAGEGRIPPER);

			if (Top)
				return (Left==Right) ? HTTOP : Left ? HTTOPLEFT : HTTOPRIGHT;

			if (Bottom)
				return (Left==Right) ? HTBOTTOM : Left ? HTBOTTOMLEFT : HTBOTTOMRIGHT;

			Top = (point.y<rectWindow.top+GRIPPEREDGE*BACKSTAGEGRIPPER);
			Bottom = (point.y>=rectWindow.bottom-GRIPPEREDGE*BACKSTAGEGRIPPER);
			Left = (point.x<rectWindow.left+BACKSTAGEGRIPPER);
			Right = (point.x>=rectWindow.right-BACKSTAGEGRIPPER);

			if (Left)
				return (Top==Bottom) ? HTLEFT : Top ? HTTOPLEFT : HTBOTTOMLEFT;

			if (Right)
				return (Top==Bottom) ? HTRIGHT : Top ? HTTOPRIGHT : HTBOTTOMRIGHT;
		}

		HitTest = HTCLIENT;
		if (GetAsyncKeyState(VK_LBUTTON))
		{
			ScreenToClient(&point);

			CRect rectLayout;
			GetLayoutRect(rectLayout);

			if (!HasDocumentSheet() || (point.x<rectLayout.left) || (point.y<rectLayout.top))
				HitTest = HTCAPTION;
		}
	}

	return HitTest;
}

void CBackstageWnd::OnNcPaint()
{
	if (!IsCtrlThemed())
	{
		CWindowDC dc(this);

		CRect rect;
		GetWindowRect(rect);

		rect.OffsetRect(-rect.left, -rect.top);

		dc.Draw3dRect(rect, 0x000000, 0x000000);
	}
}

BOOL CBackstageWnd::OnNcActivate(BOOL /*bActive*/)
{
	return TRUE;
}

void CBackstageWnd::OnEnable(BOOL bEnable)
{
	m_wndWidgets.SetEnabled(bEnable);

	InvalidateCaption();

	if (m_pSidebarWnd)
		m_pSidebarWnd->Invalidate();
}

LRESULT CBackstageWnd::OnGetTitleBarInfoEx(WPARAM /*wParam*/, LPARAM lParam)
{
	LPTITLEBARINFOEX pTitleBarInfo = (LPTITLEBARINFOEX)lParam;

	if (pTitleBarInfo->cbSize>=sizeof(TITLEBARINFOEX))
	{
		ZeroMemory(&pTitleBarInfo->rcTitleBar, sizeof(RECT));

		pTitleBarInfo->rgstate[2] = pTitleBarInfo->rgstate[3] = pTitleBarInfo->rgstate[5] = STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_UNAVAILABLE;
		ZeroMemory(&pTitleBarInfo->rgrect, sizeof(RECT)*(CCHILDREN_TITLEBAR+1));

		return TRUE;
	}

	return FALSE;
}

void CBackstageWnd::OnPaint()
{
	UpdateBackground();

	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	PaintBackground(pDC, rect);
}

LRESULT CBackstageWnd::OnSetFont(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return NULL;
}

LRESULT CBackstageWnd::OnThemeChanged()
{
	AdjustLayout();

	m_BackBufferL = m_BackBufferH = 0;
	UpdateBackground();

	m_RegionWidth = m_RegionHeight = -1;
	UpdateRegion();

	return NULL;
}

void CBackstageWnd::OnCompositionChanged()
{
	if (LFGetApp()->m_DwmLibLoaded)
	{
		BOOL ForceDisabled = TRUE;
		LFGetApp()->zDwmSetWindowAttribute(m_hWnd, DWMWA_TRANSITIONS_FORCEDISABLED, &ForceDisabled, sizeof(ForceDisabled));

		DWMNCRENDERINGPOLICY NCRP = DWMNCRP_DISABLED;
		LFGetApp()->zDwmSetWindowAttribute(m_hWnd, DWMWA_NCRENDERING_POLICY, &NCRP, sizeof(NCRP));
	}
}

HBRUSH CBackstageWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hBrush = (HBRUSH)CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	if ((nCtlColor==CTLCOLOR_STATIC) || (nCtlColor==CTLCOLOR_BTN) || (nCtlColor==CTLCOLOR_EDIT))
	{
		if (hBackgroundBrush)
		{
			CRect rect;

			if (nCtlColor==CTLCOLOR_EDIT)
			{
				if (!m_IsDialog)
				{
					pDC->SetBkMode(TRANSPARENT);
					pDC->SetTextColor(0xDACCC4);
				}

				pWnd->GetClientRect(rect);
				pWnd->ClientToScreen(rect);
			}
			else
			{
				pDC->SetTextColor(0x000000);

				pWnd->GetWindowRect(rect);
			}

			ScreenToClient(rect);
			pDC->SetBrushOrg(-rect.left, -rect.top);

			hBrush = hBackgroundBrush;
		}
		else
			if ((pWnd==&m_wndWidgets) || (pWnd==m_pSidebarWnd) || !m_IsDialog)
			{
				hBrush = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
			}
			else
			{
				CRect rect;

				pWnd->GetWindowRect(rect);
				ScreenToClient(rect);
				pDC->SetDCBrushColor(GetSysColor(rect.top>=m_BottomDivider ? COLOR_3DFACE : COLOR_WINDOW));

				hBrush = (HBRUSH)GetStockObject(DC_BRUSH);
			}

		if (nCtlColor!=CTLCOLOR_EDIT)
			pDC->SetBkMode(TRANSPARENT);
	}

	return hBrush;
}

void CBackstageWnd::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	// Windows who are above the upper screen edge minus default caption bar height get repositioned,
	// so do not allow this in the first place (Windows XP)
	if (lpwndpos->y<0)
		lpwndpos->y = 0;

	CFrontstageWnd::OnWindowPosChanging(lpwndpos);

	if (!(lpwndpos->flags & (SWP_NOCLIENTSIZE | SWP_NOSIZE)))
		if ((lpwndpos->x+lpwndpos->cx>0) && (lpwndpos->y+lpwndpos->cy>0) && ((lpwndpos->cx>m_RegionWidth) || (lpwndpos->cy>m_RegionHeight)))
			UpdateRegion(lpwndpos->cx, lpwndpos->cy);

	// Handle Z order for backstage dialog windows
	if (!IsWindowEnabled())
		lpwndpos->flags |= SWP_NOZORDER;
}

void CBackstageWnd::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	m_wndShadow.Update(this, CRect(lpwndpos->x, lpwndpos->y, lpwndpos->x+lpwndpos->cx, lpwndpos->y+lpwndpos->cy));

	if (!IsIconic() && !(lpwndpos->flags & (SWP_NOCLIENTSIZE | SWP_NOSIZE)))
	{
		AdjustLayout(SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOCOPYBITS);

		if (IsWindowVisible())
		{
			UpdateBackground();
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
		}

		UpdateRegion(lpwndpos->cx, lpwndpos->cy);

		if (IsWindowVisible())
			RedrawWindow(NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
}

void CBackstageWnd::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	CFrontstageWnd::OnGetMinMaxInfo(lpMMI);

	if (!m_IsDialog)
		if (GetStyle() & WS_THICKFRAME)
		{
			lpMMI->ptMinTrackSize.x = 650+GetSystemMetrics(SM_CXVSCROLL);
			lpMMI->ptMinTrackSize.y = 384;
		}
		else
		{
			lpMMI->ptMinTrackSize.x = lpMMI->ptMinTrackSize.y = 3*BACKSTAGERADIUS;
		}

	if (m_pSidebarWnd)
	{
		CRect rectLayout;
		GetLayoutRect(rectLayout);

		lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, rectLayout.top+m_pSidebarWnd->GetMinHeight()+BACKSTAGERADIUS);
	}
}

void CBackstageWnd::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	// Handle click position
	if ((pWnd!=this) || (pos.x<0) || (pos.y<0))
		return;

	CPoint posClient(pos);
	ScreenToClient(&posClient);

	// Do not show context menu on document sheet
	if (HasDocumentSheet())
	{
		CRect rectLayout;
		GetLayoutRect(rectLayout);

		if ((posClient.x>=rectLayout.left) && (posClient.y>=rectLayout.top))
			return;
	}

	// Get menu
	CMenu Menu;
	BOOL SetDefaultItem = FALSE;

	SetDefaultItem |= GetContextMenu(Menu, ItemAtPosition(posClient));
	SetDefaultItem |= GetContextMenu(Menu, PtrAtPosition(posClient));
	SetDefaultItem |= GetContextMenu(Menu, PointAtPosition(posClient));

	if (IsMenu(Menu))
	{
		TrackPopupMenu(Menu, pos, this, SetDefaultItem);
	}
	else
	{
		// System menu
		CMenu* pSystemMenu = GetSystemMenu(FALSE);

		if (pSystemMenu)
		{
			pSystemMenu->EnableMenuItem(SC_MAXIMIZE, MF_BYCOMMAND | (IsZoomed() ? MF_GRAYED : MF_ENABLED));
			pSystemMenu->EnableMenuItem(SC_MINIMIZE, MF_BYCOMMAND | (IsIconic() ? MF_GRAYED : MF_ENABLED));
			pSystemMenu->EnableMenuItem(SC_MOVE, MF_BYCOMMAND | (IsZoomed() || IsIconic() ? MF_GRAYED : MF_ENABLED));
			pSystemMenu->EnableMenuItem(SC_RESTORE, MF_BYCOMMAND | (IsZoomed() || IsIconic() ? MF_ENABLED : MF_GRAYED));
			pSystemMenu->EnableMenuItem(SC_SIZE, MF_BYCOMMAND | (IsZoomed() || IsIconic() ? MF_GRAYED : MF_ENABLED));

			SendMessage(WM_SYSCOMMAND, (WPARAM)pSystemMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pos.x, pos.y, this));
		}
	}
}

LRESULT CBackstageWnd::OnTaskbarButtonCreated(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (LFGetApp()->OSVersion>=OS_Seven)
	{
		if (m_pTaskbarList3)
		{
			m_pTaskbarList3->Release();
			m_pTaskbarList3 = NULL;
		}

		CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, (void**)&m_pTaskbarList3);

		if (m_pTaskbarList3)
			if (FAILED(m_pTaskbarList3->HrInit()))
			{
				m_pTaskbarList3->Release();
				m_pTaskbarList3 = NULL;
			}
	}

	return NULL;
}

LRESULT CBackstageWnd::OnLicenseActivated(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_ShowExpireCaption = LFIsSharewareExpired();

	AdjustLayout(SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOCOPYBITS);

	m_BackBufferL = m_BackBufferH = 0;
	UpdateBackground();

	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);

	return NULL;
}

LRESULT CBackstageWnd::OnSetProgress(WPARAM /*wParam*/, LPARAM lParam)
{
	PROGRESSDATA* pProgress = (PROGRESSDATA*)lParam;

	if (m_pTaskbarList3)
		if (pProgress)
		{
			m_pTaskbarList3->SetProgressState(GetSafeHwnd(), pProgress->tbpFlags);

			if (pProgress->tbpFlags>=TBPF_NORMAL)
				m_pTaskbarList3->SetProgressValue(GetSafeHwnd(), pProgress->ullCompleted, pProgress->ullTotal);
		}
		else
		{
			m_pTaskbarList3->SetProgressState(GetSafeHwnd(), TBPF_NOPROGRESS);
		}

	return NULL;
}

LRESULT CBackstageWnd::OnWakeup(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return 24878;
}

BOOL CBackstageWnd::OnCopyData(CWnd* /*pWnd*/, COPYDATASTRUCT* pCopyDataStruct)
{
	if (pCopyDataStruct->cbData!=sizeof(CDSWAKEUP))
		return FALSE;

	CDSWAKEUP* pCDS = (CDSWAKEUP*)pCopyDataStruct->lpData;
	if (pCDS->AppID!=LFGetApp()->m_AppID)
		return FALSE;

	LFGetApp()->OpenCommandLine(pCDS->Command[0] ? pCDS->Command : NULL);

	return TRUE;
}
