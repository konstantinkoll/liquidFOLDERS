
// LFTabbedDialog.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include <afxpriv.h>


// LFTabbedDialog
//

LFTabbedDialog::LFTabbedDialog(UINT nCaptionID, CWnd* pParentWnd, UINT* pLastTab)
	: LFDialog(IDD_TABBEDDIALOG, pParentWnd)
{
	if (nCaptionID)
		ENSURE(m_DialogCaption.LoadString(nCaptionID));

	p_LastTab = pLastTab;

	m_CurrentTab = m_TabCount = 0;
	ZeroMemory(m_TabHints, sizeof(m_TabHints));
}

BOOL LFTabbedDialog::AddTab(const CString& Caption)
{
	if (m_TabCount>=MAXTABS)
		return FALSE;

	m_wndSidebar.AddCommand(IDD_TABBEDDIALOG+(m_TabCount++), -1, Caption);

	return TRUE;
}

BOOL LFTabbedDialog::AddTab(UINT nResID, LPSIZE pszTabArea)
{
	ASSERT(pszTabArea);

	BOOL Result = FALSE;

	if (m_TabCount<MAXTABS)
	{
		// Load resource
		HRSRC hResource = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nResID), RT_DIALOG);
		if (hResource)
		{
			HGLOBAL hMemory = LoadResource(AfxGetResourceHandle(), hResource);
			if (hMemory)
			{
				LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hMemory);
				if (lpDialogTemplate)
				{
					CFont* pFont = GetFont();

					// Create temporary dialog; window must be alive for some common controls to display properly!
					CWnd wndTemp;
					AfxHookWindowCreate(&wndTemp);
					CreateDialogIndirect(AfxGetResourceHandle(), lpDialogTemplate, GetSafeHwnd(), NULL);
					AfxUnhookWindowCreate();

					if (IsWindow(wndTemp))
					{
						// Adjust size of tab area if neccessary
						CRect rectClient;
						wndTemp.GetClientRect(rectClient);

						if (rectClient.right>pszTabArea->cx)
							pszTabArea->cx = rectClient.right;

						if (rectClient.bottom>pszTabArea->cy)
							pszTabArea->cy = rectClient.bottom;

						// Caption
						CString Caption;
						wndTemp.GetWindowText(Caption);

						// Copy dialog controls to main dialog
						CWnd* pChildWnd = wndTemp.GetWindow(GW_CHILD);

						while (pChildWnd)
						{
							// Clone control
							//

							// Style and ID
							DWORD dwStyle = pChildWnd->GetStyle();
							DWORD dwExStyle = pChildWnd->GetExStyle();
							UINT nID = pChildWnd->GetDlgCtrlID();

							// Class name
							TCHAR szClassName[32];
							GetClassName(pChildWnd->GetSafeHwnd(), szClassName, 32);

							// Window rectangle
							CRect rectWindow;
							pChildWnd->GetWindowRect(rectWindow);
							wndTemp.ScreenToClient(rectWindow);

							// Window text
							TCHAR szWindowText[256];
							pChildWnd->GetWindowText(szWindowText, 256);

							// Recreate window, and save handle for tab switching
							HWND hWnd = CreateWindowEx(dwExStyle, szClassName, szWindowText, dwStyle, rectWindow.left, rectWindow.top, rectWindow.Width(), rectWindow.Height(), GetSafeHwnd(), (HMENU)nID, NULL, NULL);

							AddControl(hWnd, m_TabCount);

							// Set font
							::SendMessage(hWnd, WM_SETFONT, (WPARAM)pFont->GetSafeHandle(), NULL);

							// Set bitmap
							if (CompareClassName(szClassName, _T("Static")) && (dwStyle & SS_BITMAP))
								::SendMessage(hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)pChildWnd->SendMessage(STM_GETIMAGE, IMAGE_BITMAP));

							// Append tab hint
							if (CompareClassName(szClassName, _T("CCategory")))
							{
								WCHAR* pChar = wcschr(szWindowText, L'\n');
								if (pChar)
									*pChar = L'\0';

								if (Caption!=szWindowText)
								{
									if (m_TabHints[m_TabCount][0])
										wcscat_s(m_TabHints[m_TabCount], 4096, L"\n");

									wcscat_s(m_TabHints[m_TabCount], 4096, szWindowText);
								}
							}

							// Next
							pChildWnd = pChildWnd->GetWindow(GW_HWNDNEXT);
						}

						wndTemp.DestroyWindow();

						Result = AddTab(Caption);
					}
				}
			}
		}
	}

	return Result;
}

void LFTabbedDialog::AddControl(HWND hWnd, UINT Index)
{
	ASSERT(Index<=m_TabCount);
	ASSERT(Index<MAXTABS);

#ifdef _DEBUG
	for (UINT a=0; a<m_ControlsOnTab.m_ItemCount; a++)
		ASSERT(m_ControlsOnTab[a].hWnd!=hWnd);
#endif

	ControlOnTab Ctrl;
	Ctrl.hWnd = hWnd;
	Ctrl.TabMask = 1<<Index;

	AddControl(Ctrl);
}

void LFTabbedDialog::ShowControlOnTabs(HWND hWnd, USHORT Mask)
{
	for (UINT a=0; a<m_ControlsOnTab.m_ItemCount; a++)
		if (m_ControlsOnTab[a].hWnd==hWnd)
		{
			m_ControlsOnTab[a].TabMask = Mask;

			break;
		}
}

void LFTabbedDialog::ShowTab(UINT Index)
{
	ASSERT(Index<m_TabCount);

	BOOL First = TRUE;
	const USHORT Mask = 1<<Index;

	for (UINT a=0; a<m_ControlsOnTab.m_ItemCount; a++)
	{
		const HWND hWnd = m_ControlsOnTab[a].hWnd;
		const BOOL Show = (m_ControlsOnTab[a].TabMask & Mask);

		::ShowWindow(hWnd, Show ? SW_SHOW : SW_HIDE);

		if (First && Show && (::GetWindowLong(hWnd, GWL_STYLE) & WS_TABSTOP))
			if (::IsWindowEnabled(hWnd))
			{
				::SetFocus(hWnd);
				First = FALSE;
			}
	}
}

void LFTabbedDialog::SelectTab(UINT Index)
{
	ASSERT(Index<m_TabCount);

	m_CurrentTab = Index;

	m_wndSidebar.SetSelection(IDD_TABBEDDIALOG+Index);
	m_wndSidebar.SetFocus();

	ShowTab(Index);

	m_BackBufferL = m_BackBufferH = 0;
	UpdateBackground();

	Invalidate();
}

BOOL LFTabbedDialog::InitSidebar(LPSIZE /*pszTabArea*/)
{
	if (!m_wndSidebar.Create(this, 3))
		return FALSE;

	SetSidebar(&m_wndSidebar);

	return TRUE;
}

BOOL LFTabbedDialog::InitDialog()
{
	// Caption
	SetWindowText(m_DialogCaption);

	// Buttons
	HINSTANCE hInstance = LoadLibrary(_T("USER32.DLL"));
	CString tmpStr;

	ENSURE(tmpStr.LoadString(hInstance, 799+IDOK));
	GetDlgItem(IDOK)->SetWindowText(tmpStr);

	ENSURE(tmpStr.LoadString(hInstance, 799+IDCANCEL));
	GetDlgItem(IDCANCEL)->SetWindowText(tmpStr);

	FreeLibrary(hInstance);

	// First tab
	SelectTab(p_LastTab ? *p_LastTab : 0);

	return FALSE;
}


BEGIN_MESSAGE_MAP(LFTabbedDialog, LFDialog)
	ON_WM_DESTROY()
	ON_NOTIFY(REQUEST_TOOLTIP_DATA, 3, OnRequestTooltipData)

	ON_COMMAND_RANGE(IDD_TABBEDDIALOG, IDD_TABBEDDIALOG+MAXTABS-1, OnSelectTab)
	ON_UPDATE_COMMAND_UI_RANGE(IDD_TABBEDDIALOG, IDD_TABBEDDIALOG+MAXTABS-1, OnUpdateTabCommands)
END_MESSAGE_MAP()

void LFTabbedDialog::OnDestroy()
{
	if (p_LastTab)
		*p_LastTab = m_CurrentTab;

	LFDialog::OnDestroy();
}

void LFTabbedDialog::OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TOOLTIPDATA* pTooltipData = (NM_TOOLTIPDATA*)pNMHDR;

	const UINT nTab = pTooltipData->Item-IDD_TABBEDDIALOG;
	ASSERT(nTab<m_TabCount);

	wcscpy_s(pTooltipData->Hint, 4096, m_TabHints[nTab]);

	*pResult = TRUE;
}


void LFTabbedDialog::OnSelectTab(UINT nCmdID)
{
	SelectTab(nCmdID-IDD_TABBEDDIALOG);
}

void LFTabbedDialog::OnUpdateTabCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable();
}
