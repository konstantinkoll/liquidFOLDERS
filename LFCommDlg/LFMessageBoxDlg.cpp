
// LFMessageBoxDlg.cpp: Implementierung der Klasse LFMessageBoxDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFMessageBoxDlg
//

#define TEXTFLAGS     DT_LEFT | DT_NOPREFIX | DT_END_ELLIPSIS | DT_WORDBREAK

LFMessageBoxDlg::LFMessageBoxDlg(CWnd* pParentWnd, CString Text, CString Caption, UINT Type)
	: LFDialog(IDD_MESSAGEBOX, pParentWnd)
{
	ASSERT((Type & (MB_HELP | MB_TASKMODAL | MB_DEFAULT_DESKTOP_ONLY | MB_RIGHT | MB_RTLREADING | MB_SERVICE_NOTIFICATION))==0);
	ASSERT((Type & MB_TYPEMASK)<=MB_CANCELTRYCONTINUE);

	m_Text = Text;
	m_Caption = Caption;
	m_Type = Type;

	m_IconSize = 0;
	hIcon = NULL;
}

BOOL LFMessageBoxDlg::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message==WM_KEYDOWN) && ((pMsg->wParam==VK_ESCAPE) || (pMsg->wParam==VK_CANCEL)))
	{
		if (GetDlgItem(IDCANCEL))
			EndDialog(IDCANCEL);

		if (GetDlgItem(IDOK))
			EndDialog(IDOK);

		return TRUE;
	}

	return LFDialog::PreTranslateMessage(pMsg);
}

void LFMessageBoxDlg::OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect)
{
	LFDialog::OnEraseBkgnd(dc, g, rect);

	// Icon
	if (hIcon)
		DrawIconEx(dc, m_IconPos.x, m_IconPos.y, hIcon, m_IconSize, m_IconSize, 0, NULL, DI_NORMAL);

	// Text
	CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DefaultFont);

	dc.SetTextColor(0x000000);
	dc.DrawText(m_Text, m_rectText, TEXTFLAGS);

	dc.SelectObject(pOldFont);
}

__forceinline void LFMessageBoxDlg::SetButton(UINT nResID, HINSTANCE hInstance, UINT nCommand, UINT& cButtons)
{
	CWnd* pWnd = GetDlgItem(nResID);

	ASSERT(pWnd);

	if (nCommand)
	{
		CString tmpStr;
		ENSURE(tmpStr.LoadString(hInstance, 799+nCommand));

		pWnd->SetWindowText(tmpStr);
		pWnd->SetDlgCtrlID(nCommand);

		if ((m_Type & MB_DEFMASK)>>8==cButtons++)
		{
			SetDefID(nCommand);
			pWnd->SetFocus();
		}
	}
	else
	{
		pWnd->ModifyStyle(WS_VISIBLE, WS_DISABLED);
	}
}


BEGIN_MESSAGE_MAP(LFMessageBoxDlg, LFDialog)
	ON_WM_DESTROY()
	ON_COMMAND_RANGE(IDOK, IDCONTINUE, OnButtonClicked)
END_MESSAGE_MAP()

BOOL LFMessageBoxDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	AddBottomRightControl(IDC_BUTTON1);
	AddBottomRightControl(IDC_BUTTON2);
	AddBottomRightControl(IDC_BUTTON3);

	// Titel
	SetWindowText(m_Caption);

	// Icon
	CRect rectLayout;
	GetLayoutRect(rectLayout);

	if (m_Type & MB_ICONMASK)
	{
		m_IconSize = (rectLayout.bottom>=48) ? 48 : 32;

		LPCWSTR IconName = IDI_INFORMATION;

		switch (m_Type & MB_ICONMASK)
		{
		case MB_ICONERROR:
			LFGetApp()->PlayErrorSound();
			IconName = IDI_ERROR;

			break;

		case MB_ICONQUESTION:
			LFGetApp()->PlayQuestionSound();
			IconName = IDI_QUESTION;

			break;

		case MB_ICONWARNING:
			LFGetApp()->PlayWarningSound();
			IconName = IDI_WARNING;

			break;

		case MB_ICONREADY:
			IconName = MAKEINTRESOURCE(IDI_READY);

		case MB_ICONINFORMATION:
			LFGetApp()->PlayAsteriskSound();

			break;

		case MB_ICONSHIELD:
			LFGetApp()->PlayNotificationSound();
			IconName = (LFGetApp()->OSVersion==OS_Vista) ? MAKEINTRESOURCE(IDI_SHIELD_VISTA) : IDI_SHIELD;

			break;

		default:
			LFGetApp()->PlayDefaultSound();
		}

		hIcon = (HICON)LoadImage(AfxGetResourceHandle(), IconName, IMAGE_ICON, m_IconSize, m_IconSize, LR_SHARED);
	}

	// Layout
	CRect rectScreen;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);

	m_rectText.SetRect(14, 14, 292, 10000);
	MapDialogRect(m_rectText);

	CPoint rectBorders(m_rectText.left/2, m_rectText.top);

	if (hIcon)
	{
		m_IconPos = m_rectText.TopLeft();
		m_rectText.left += m_IconSize+rectBorders.x;
	}

	if (2*rectBorders.x+m_rectText.right>rectScreen.Width()*5/8)
		m_rectText.right = rectScreen.Width()*5/8-2*rectBorders.x;

	CDC* pDC = GetDC();
	CFont* pOldFont = pDC->SelectObject(&LFGetApp()->m_DefaultFont);
	pDC->DrawText(m_Text, m_rectText, TEXTFLAGS | DT_CALCRECT);
	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);

	if (rectBorders.y+m_rectText.bottom>rectScreen.Height()*3/4)
		m_rectText.bottom = rectScreen.Height()*3/4-rectBorders.y;

	// Fenstergröße anpassen
	CRect rectClient;
	GetClientRect(rectClient);

	INT DiffX = max(0, m_rectText.right+2*rectBorders.x-rectClient.Width());
	INT DiffY = max(0, max(m_IconSize+2*rectBorders.y, m_rectText.bottom+rectBorders.y)-rectLayout.bottom);

	CRect rectWindow;
	GetWindowRect(rectWindow);
	SetWindowPos(NULL, 0, 0, rectWindow.Width()+DiffX, rectWindow.Height()+DiffY, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

	if (m_IconSize>m_rectText.Height())
	{
		m_rectText.OffsetRect(0, (m_IconSize-m_rectText.Height())/2);
	}
	else
	{
		m_IconPos.y += (m_rectText.Height()-m_IconSize)/2;
	}

	// Buttons
	static UINT Buttons[7][3] = {
		{ 0, 0, IDOK }, { 0, IDOK, IDCANCEL }, { IDABORT, IDRETRY, IDIGNORE }, { IDYES, IDNO, IDCANCEL },
		{ 0, IDYES, IDNO }, { 0, IDRETRY, IDCANCEL }, { IDCANCEL, IDTRYAGAIN, IDCONTINUE }
	};

	HINSTANCE hInstance = LoadLibrary(_T("USER32.DLL"));

	UINT cButtons = 0;
	for (UINT a=0; a<3; a++)
		SetButton(IDC_BUTTON1+a, hInstance, Buttons[m_Type & MB_TYPEMASK][a], cButtons);

	FreeLibrary(hInstance);

	// Mode
	if (((m_Type & MB_MODEMASK)==MB_SYSTEMMODAL) || (m_Type & MB_TOPMOST))
		SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);

	if (m_Type & MB_SETFOREGROUND)
		SetForegroundWindow();

	// Systemmenü
	if (((m_Type & MB_TYPEMASK)==MB_ABORTRETRYIGNORE) || ((m_Type & MB_TYPEMASK)==MB_YESNO))
	{
		CMenu* pMenu = GetSystemMenu(FALSE);

		if (pMenu)
			pMenu->EnableMenuItem (SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
	}

	return FALSE;
}

void LFMessageBoxDlg::OnDestroy()
{
	DestroyIcon(hIcon);

	LFDialog::OnDestroy();
}

void LFMessageBoxDlg::OnButtonClicked(UINT nID)
{
	EndDialog(nID);
}
