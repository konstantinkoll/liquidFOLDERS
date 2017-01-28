
// LFDialog.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include <afxpriv.h>


// LFDialog
//

#define DELETE_EXCEPTION(e) do { if(e) { e->Delete(); } } while(0)

LFDialog::LFDialog(UINT nIDTemplate, CWnd* pParentWnd, BOOL WantsBitmap, BOOL UAC)
	: CBackstageWnd(TRUE, WantsBitmap | UAC)
{
	hWndTop = NULL;
	p_ParentWnd = pParentWnd;
	m_lpszTemplateName = MAKEINTRESOURCE(nIDTemplate);

	if (UAC)
	{
		m_wndDesktopDimmer.Create(this);
		p_ParentWnd = &m_wndDesktopDimmer;
	}

	m_UAC = UAC;

	hIconShield = NULL;
	hBackgroundBrush = NULL;
	m_BackBufferL = m_BackBufferH = m_UACHeight = 0;
	ZeroMemory(&m_BottomLeftControl, sizeof(m_BottomLeftControl));
}

BOOL LFDialog::Create()
{
	ASSERT(m_lpszTemplateName);
	ASSERT(!m_hWnd);

	BOOL Result = FALSE;

	if (!p_ParentWnd)
		p_ParentWnd = CWnd::GetDesktopWindow();

	// Load resource
	HRSRC hResource = FindResource(AfxGetResourceHandle(), m_lpszTemplateName, RT_DIALOG);
	if (hResource)
	{
		HGLOBAL hMemory = LoadResource(AfxGetResourceHandle(), hResource);
		if (hMemory)
		{
			LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hMemory);
			if (lpDialogTemplate)
			{
				// Create dialog
				AfxHookWindowCreate(this);
				Result = CreateDlgIndirect(lpDialogTemplate, p_ParentWnd, AfxGetResourceHandle());
				AfxUnhookWindowCreate();
			}
		}
	}

	return Result;
}

BOOL LFDialog::PreTranslateMessage(MSG* pMsg)
{
	// Main window
	if (CBackstageWnd::PreTranslateMessage(pMsg))
		return TRUE;

	// Fix for VK_ESCAPE in a multiline edit control that is on a dialog
	// that doesn't have a cancel button, or the cancel button is disabled.
	if ((pMsg->message==WM_KEYDOWN) && ((pMsg->wParam==VK_ESCAPE) || (pMsg->wParam==VK_CANCEL)) &&
		(::GetWindowLong(pMsg->hwnd, GWL_STYLE) & ES_MULTILINE) && CompareClassName(pMsg->hwnd, _T("Edit")))
	{
		HWND hItem = ::GetDlgItem(m_hWnd, IDCANCEL);
		if (!hItem || ::IsWindowEnabled(hItem))
		{
			SendMessage(WM_COMMAND, IDCANCEL, 0);

			return TRUE;
		}
	}

	// Filter both messages to dialog and from children
	return PreTranslateInput(pMsg);
}

void LFDialog::DoDataExchange(CDataExchange* pDX)
{
	for (UINT a=0; a<12; a++)
		if (GetDlgItem(IDC_CATEGORY1+a))
			DDX_Control(pDX, IDC_CATEGORY1+a, m_wndCategory[a]);
}

CWnd* LFDialog::GetBottomWnd() const
{
	return m_BottomRightControls.m_ItemCount ? m_BottomRightControls[0].pChildWnd : NULL;
}

void LFDialog::SetBottomLeftControl(CWnd* pChildWnd)
{
	ASSERT(pChildWnd);

	m_BottomLeftControl.pChildWnd = pChildWnd;

	pChildWnd->ModifyStyle(0, WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	pChildWnd->GetWindowRect(&m_BottomLeftControl.rectClient);
	ScreenToClient(&m_BottomLeftControl.rectClient);
}

void LFDialog::SetBottomLeftControl(UINT nID)
{
	CWnd* pChildWnd = GetDlgItem(nID);
	if (pChildWnd)
		SetBottomLeftControl(pChildWnd);
}

void LFDialog::AddBottomRightControl(CWnd* pChildWnd)
{
	ASSERT(pChildWnd);

	DialogControl Control;
	Control.pChildWnd = pChildWnd;

	pChildWnd->GetWindowRect(&Control.rectClient);
	ScreenToClient(&Control.rectClient);

	if (!m_BottomDivider)
	{
		CRect rectBorders(0, 0, 7, 7);
		MapDialogRect(&rectBorders);

		m_BottomDivider = Control.rectClient.top-rectBorders.Height()-2;
	}

	m_BottomRightControls.AddItem(Control);
}

void LFDialog::AddBottomRightControl(UINT nID)
{
	CWnd* pChildWnd = GetDlgItem(nID);

	if (pChildWnd)
		AddBottomRightControl(pChildWnd);
}

void LFDialog::AdjustLayout(const CRect& rectLayout, UINT nFlags)
{
	CPoint ptDiff(rectLayout.Width()-m_LastSize.x, rectLayout.Height()-m_LastSize.y);
	INT MaxRight = m_LastSize.x = rectLayout.Width();
	m_LastSize.y = rectLayout.Height();

	// Move bottom right controls
	for (UINT a=0; a<m_BottomRightControls.m_ItemCount; a++)
	{
		DialogControl* pDialogControl = &m_BottomRightControls[a];

		OffsetRect(&pDialogControl->rectClient, ptDiff.x, ptDiff.y);
		pDialogControl->pChildWnd->SetWindowPos(NULL, pDialogControl->rectClient.left, pDialogControl->rectClient.top, 0, 0, nFlags | SWP_NOSIZE);

		MaxRight = min(MaxRight, pDialogControl->rectClient.left);
	}

	// Resize bottom left control
	if (m_BottomLeftControl.pChildWnd)
	{
		m_BottomLeftControl.rectClient.right = MaxRight-8;
		OffsetRect(&m_BottomLeftControl.rectClient, 0, ptDiff.y);

		m_BottomLeftControl.pChildWnd->SetWindowPos(NULL, m_BottomLeftControl.rectClient.left, m_BottomLeftControl.rectClient.top, m_BottomLeftControl.rectClient.right-m_BottomLeftControl.rectClient.left, m_BottomLeftControl.rectClient.bottom-m_BottomLeftControl.rectClient.top, nFlags);
	}

	// Move divider
	if (m_BottomDivider)
		m_BottomDivider += ptDiff.y;
}

void LFDialog::PaintOnBackground(CDC& dc, Graphics& g, const CRect& rectLayout)
{
	BOOL Themed = IsCtrlThemed();

	// Child windows
	if (Themed)
	{
		g.SetPixelOffsetMode(PixelOffsetModeNone);

		for (UINT a=0; a<m_Buttons.m_ItemCount; a++)
			if (m_Buttons[a]->IsWindowVisible())
			{
				CRect rectBounds;
				m_Buttons[a]->GetWindowRect(rectBounds);
				ScreenToClient(rectBounds);

				DrawWhiteButtonBorder(g, rectBounds);
			}
	}

	// UAC
	if (m_UAC)
	{
		if (Themed)
		{
			g.SetPixelOffsetMode(PixelOffsetModeHalf);

			LinearGradientBrush brush2(Point(rectLayout.left, 0), Point(rectLayout.right, 0), Color(0xFF045082), Color(0xFF1C7885));
			g.FillRectangle(&brush2, rectLayout.left, rectLayout.top, rectLayout.Width(), m_UACHeight);
			dc.SetTextColor(0xFFFFFF);
		}
		else
		{
			dc.FillSolidRect(rectLayout.left, rectLayout.top, rectLayout.Width(), m_UACHeight, GetSysColor(COLOR_HIGHLIGHT));
			dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
		}

		CRect rectBorders(0, 0, 7, 7);
		MapDialogRect(&rectBorders);

		DrawIconEx(dc, rectLayout.left+rectBorders.right-m_ShieldSize/16, rectLayout.top+(m_UACHeight-m_ShieldSize)/2, hIconShield, m_ShieldSize, m_ShieldSize, 0, NULL, DI_NORMAL);

		CRect rectText(rectLayout);
		rectText.left += rectBorders.right+rectBorders.right/4+m_ShieldSize;
		rectText.bottom = rectText.top+m_UACHeight;

		CString tmpStr((LPCSTR)IDS_UACMESSAGE);

		CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_UACFont);
		dc.DrawText(tmpStr, rectText, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_LEFT | DT_NOPREFIX);
		dc.SelectObject(pOldFont);
	}
}

BOOL LFDialog::InitSidebar(LPSIZE /*pszTabArea*/)
{
	return TRUE;
}

BOOL LFDialog::InitDialog()
{
	return TRUE;
}

INT_PTR LFDialog::DoModal()
{
	ASSERT(m_lpszTemplateName);
	ASSERT(!m_hWnd);

	m_nModalResult = -1;

	// Load resource
	HRSRC hResource = FindResource(AfxGetResourceHandle(), m_lpszTemplateName, RT_DIALOG);
	if (hResource)
	{
		HGLOBAL hMemory = LoadResource(AfxGetResourceHandle(), hResource);
		if (hMemory)
		{
			LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hMemory);
			if (lpDialogTemplate)
			{
				// Handle parent window
				HWND hWndParent = CBackstageWnd::GetSafeOwner_(p_ParentWnd->GetSafeHwnd(), &hWndTop);

				BOOL EnableDisableParent = (hWndParent && (hWndParent!=::GetDesktopWindow()) && ::IsWindowEnabled(hWndParent));
				if (EnableDisableParent)
					::EnableWindow(hWndParent, FALSE);

				TRY
				{
					// Create dialog
					AfxHookWindowCreate(this);
					if (CreateDlgIndirect(lpDialogTemplate, CWnd::FromHandle(hWndParent), AfxGetResourceHandle()))
					{
						if (m_nFlags & WF_CONTINUEMODAL)
						{
							// Enter modal loop
							DWORD dwFlags = MLF_SHOWONIDLE;
							if (GetStyle() & DS_NOIDLEMSG)
								dwFlags |= MLF_NOIDLEMSG;

							VERIFY(RunModalLoop(dwFlags)==m_nModalResult);
						}

						// Hide the window before enabling the parent
						if (m_hWnd)
							SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_HIDEWINDOW);
					}
				}
				CATCH_ALL(e)
				{
					DELETE_EXCEPTION(e);
					m_nModalResult = -1;
				}
				END_CATCH_ALL

				// Handle parent window
				if (EnableDisableParent)
					::EnableWindow(hWndParent, TRUE);

				if (hWndParent && (::GetActiveWindow()==m_hWnd))
					::SetActiveWindow(hWndParent);

				// Destroy modal window
				AfxUnhookWindowCreate();
				DestroyWindow();

				// Enable windows
				if (::IsWindow(hWndTop))
				{
					::EnableWindow(hWndTop, TRUE);
					hWndTop = NULL;
				}
			}
		}
	}

	return m_nModalResult;
}

void LFDialog::EndDialog(INT nResult)
{
	ASSERT(::IsWindow(m_hWnd));

	if (m_nFlags & (WF_MODALLOOP | WF_CONTINUEMODAL))
		EndModalLoop(nResult);

	::EndDialog(m_hWnd, nResult);
}

BOOL LFDialog::IsPushbutton(CWnd* pWnd)
{
	if (pWnd->SendMessage(WM_GETDLGCODE) & (DLGC_BUTTON | DLGC_DEFPUSHBUTTON | DLGC_UNDEFPUSHBUTTON))
	{
		const DWORD dwStyle = pWnd->GetStyle() & BS_TYPEMASK;

		if ((dwStyle==BS_PUSHBUTTON) || (dwStyle==BS_DEFPUSHBUTTON) || (dwStyle==BS_OWNERDRAW))
			return TRUE;
	}

	return FALSE;
}

BOOL LFDialog::CompareClassName(LPCTSTR lpszClassName1, LPCTSTR lpszClassName2)
{
	return CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE, lpszClassName1, -1, lpszClassName2, -1)==CSTR_EQUAL;
}

BOOL LFDialog::CompareClassName(HWND hWnd, LPCTSTR lpszClassName)
{
	ASSERT(IsWindow(hWnd));

	TCHAR szTemp[32];
	GetClassName(hWnd, szTemp, 32);

	return CompareClassName(szTemp, lpszClassName);
}


BEGIN_MESSAGE_MAP(LFDialog, CBackstageWnd)
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog)
	ON_WM_DESTROY()
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
END_MESSAGE_MAP()

LRESULT LFDialog::OnInitDialog(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	// Execute dialog RT_DLGINIT resource
	if (!ExecuteDlgInit(m_lpszTemplateName))
	{
		EndDialog(-1);

		return FALSE;
	}

	// Sidebar
	CSize szTabArea(0, 0);
	if (!InitSidebar(&szTabArea))
	{
		EndDialog(-1);

		return FALSE;
	}

	// Transfer data into the dialog from member variables
	if (!UpdateData(FALSE))
	{
		EndDialog(-1);

		return FALSE;
	}

	// Is UAC dialog?
	if (m_UAC)
	{
		// Shield
		m_UACHeight = MulDiv(40, LOWORD(GetDialogBaseUnits()), 8);
		m_ShieldSize = (m_UACHeight<24) ? 16 : (m_UACHeight<32) ? 24 : (m_UACHeight<48) ? 32 : 48;
		hIconShield = (HICON)LoadImage(AfxGetResourceHandle(), (LFGetApp()->OSVersion==OS_Vista) ? MAKEINTRESOURCE(IDI_SHIELD_VISTA) : IDI_SHIELD, IMAGE_ICON, m_ShieldSize, m_ShieldSize, LR_SHARED);
	}

	// Subclass all buttons
	const INT CaptionHeight = GetCaptionHeight();
	const INT SidebarWidth = m_pSidebarWnd ? m_pSidebarWnd->GetPreferredWidth() : 0;
	CRect rectWnd;

	CWnd* pChildWnd = GetWindow(GW_CHILD);

	while (pChildWnd)
	{
		if ((pChildWnd!=&m_wndWidgets) && (pChildWnd!=m_pSidebarWnd))
		{
			pChildWnd->GetWindowRect(rectWnd);
			ScreenToClient(rectWnd);

			pChildWnd->SetWindowPos(NULL, rectWnd.left+SidebarWidth, rectWnd.top+CaptionHeight, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
		}

		if (IsPushbutton(pChildWnd))
		{
			CHoverButton* pButton = new CHoverButton();
			pButton->SubclassWindow(pChildWnd->GetSafeHwnd());
			pButton->ModifyStyle(BS_TYPEMASK, BS_OWNERDRAW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

			m_Buttons.AddItem(pButton);
		}
		else
		{
			pChildWnd->ModifyStyle(0, WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		}

		pChildWnd = pChildWnd->GetWindow(GW_HWNDNEXT);
	}

	// Adjust dialog size for new non-client area
	const INT BorderSize = IsCtrlThemed() ? 0 : 2;
	const BOOL ThickFrame = GetStyle() & WS_SIZEBOX;
	const INT DiffX = -2*GetSystemMetrics(ThickFrame ? SM_CXFRAME : SM_CXFIXEDFRAME)+SidebarWidth;
	const INT DiffY = -2*GetSystemMetrics(ThickFrame ? SM_CYFRAME : SM_CYFIXEDFRAME)-GetSystemMetrics(SM_CYCAPTION)+CaptionHeight;

	GetWindowRect(rectWnd);
	SetWindowPos(NULL, 0, 0, rectWnd.Width()+DiffX+BorderSize, rectWnd.Height()+DiffY+BorderSize, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);

	// Bottom area
	AddBottomRightControl(IDCANCEL);
	AddBottomRightControl(IDOK);

	if (szTabArea.cx || szTabArea.cy)
	{
		GetWindowRect(rectWnd);
		SetWindowPos(NULL, 0, 0, SidebarWidth+szTabArea.cx+BorderSize, rectWnd.Height()+szTabArea.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
	}

	CRect rectLayout;
	GetLayoutRect(rectLayout);
	m_LastSize = CPoint(rectLayout.Width(), rectLayout.Height());

	// Call derived virtual function
	BOOL bSetFocus = InitDialog();

	// Layout
	CBackstageWnd::AdjustLayout();

	return bSetFocus;
}

void LFDialog::OnDestroy()
{
	for (UINT a=0; a<m_Buttons.m_ItemCount; a++)
	{
		CHoverButton* pButton = m_Buttons[a];

		pButton->UnsubclassWindow();
		delete pButton;
	}

	if (IsWindow(m_wndDesktopDimmer))
		m_wndDesktopDimmer.SendMessage(WM_DESTROY);

	CBackstageWnd::OnDestroy();
}

void LFDialog::OnOK()
{
	if (UpdateData(TRUE))
		EndDialog(IDOK);
}

void LFDialog::OnCancel()
{
	EndDialog(IDCANCEL);
}
