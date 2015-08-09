
// CDesktopDimmer.cpp: Implementierung der Klasse CDesktopDimmer
//

#include "stdafx.h"
#include "LFCommDlg.h"


HHOOK hHook = NULL;

LRESULT CALLBACK LowLevelKeyboardProc(INT nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode==HC_ACTION)
	{
		KBDLLHOOKSTRUCT* pKBHS = (KBDLLHOOKSTRUCT*)lParam;

		switch (pKBHS->vkCode)
		{
		case VK_LMENU:
		case VK_LWIN:
		case VK_RMENU:
		case VK_RWIN:
			return 1;

		case VK_ESCAPE:
			if (GetAsyncKeyState (VK_CONTROL)<0)
				return 1;

		case VK_TAB:
			if (pKBHS->flags & LLKHF_ALTDOWN)
				return 1;

			break;
		}
	}

	return CallNextHookEx(hHook, nCode, wParam, lParam);
}


// CDesktopDimmer
//

CDesktopDimmer::CDesktopDimmer()
	: CWnd()
{
}

BOOL CDesktopDimmer::Create(CWnd* pParentWnd)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_NO));

	CRect rect;
	GetDesktopWindow()->GetWindowRect(&rect);
	return CWnd::CreateEx(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, className, _T(""), WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_POPUP, rect, pParentWnd, 0);
}


BEGIN_MESSAGE_MAP(CDesktopDimmer, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
END_MESSAGE_MAP()

INT CDesktopDimmer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	SetLayeredWindowAttributes(0x000000, 128, LWA_ALPHA);

	if (!hHook)
		hHook = SetWindowsHookEx(WH_KEYBOARD_LL, &LowLevelKeyboardProc, GetModuleHandle(NULL), 0);

	return 0;
}

void CDesktopDimmer::OnDestroy()
{
	if (hHook)
	{
		UnhookWindowsHookEx(hHook);
		hHook = NULL;
	}

	CWnd::OnDestroy();
}

BOOL CDesktopDimmer::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	pDC->FillSolidRect(rect, 0x000000);

	return TRUE;
}

LRESULT CDesktopDimmer::OnDisplayChange(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	MONITORINFO mi;
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);

	if (GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY), &mi))
		SetWindowPos(&wndTop, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right-mi.rcMonitor.left, mi.rcMonitor.bottom-mi.rcMonitor.top, SWP_NOOWNERZORDER | SWP_NOACTIVATE);

	return NULL;
}
