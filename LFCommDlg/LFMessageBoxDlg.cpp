
// LFMessageBoxDlg.cpp: Implementierung der Klasse LFMessageBoxDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFMessageBoxDlg
//

#define TEXTFLAGS     DT_LEFT | DT_NOPREFIX | DT_END_ELLIPSIS | DT_WORDBREAK

LFMessageBoxDlg::LFMessageBoxDlg(CWnd* pParentWnd, const CString& Text, const CString& Caption, UINT Type)
	: LFDialog(IDD_MESSAGEBOX, pParentWnd, TRUE)
{
	ASSERT((Type & (MB_HELP | MB_TASKMODAL | MB_DEFAULT_DESKTOP_ONLY | MB_RIGHT | MB_RTLREADING | MB_SERVICE_NOTIFICATION))==0);
	ASSERT((Type & MB_TYPEMASK)<=MB_CANCELTRYCONTINUE);

	m_Text = Text;
	m_Caption = Caption;
	m_Type = Type;

	m_IconSize = 0;
	m_hIcon = NULL;
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

void LFMessageBoxDlg::PaintOnBackground(CDC& dc, Graphics& g, const CRect& rectLayout)
{
	LFDialog::PaintOnBackground(dc, g, rectLayout);

	// Icon
	if (m_hIcon)
		DrawIconEx(dc, rectLayout.left+m_IconPos.x, rectLayout.top+m_IconPos.y, m_hIcon, m_IconSize, m_IconSize, 0, NULL, DI_NORMAL);

	// Text
	CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DefaultFont);

	CRect rectText(m_RectText);
	rectText.OffsetRect(rectLayout.TopLeft());

	dc.SetTextColor(IsCtrlThemed() ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));
	dc.DrawText(m_Text, rectText, TEXTFLAGS);

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

BOOL LFMessageBoxDlg::InitDialog()
{
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

		m_hIcon = (HICON)LoadImage(AfxGetResourceHandle(), IconName, IMAGE_ICON, m_IconSize, m_IconSize, LR_SHARED);
	}

	// Layout
	CRect rectScreen;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);

	m_RectText.SetRect(14, 14, 292, 10000);
	MapDialogRect(m_RectText);

	CPoint rectBorders(m_RectText.left/2, m_RectText.top);

	if (m_hIcon)
	{
		m_IconPos = m_RectText.TopLeft();
		m_RectText.left += m_IconSize+rectBorders.x;
	}

	if (2*rectBorders.x+m_RectText.right>rectScreen.Width()*5/8)
		m_RectText.right = rectScreen.Width()*5/8-2*rectBorders.x;

	CDC* pDC = GetDC();
	CFont* pOldFont = pDC->SelectObject(&LFGetApp()->m_DefaultFont);
	pDC->DrawText(m_Text, m_RectText, TEXTFLAGS | DT_CALCRECT);
	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);

	if (2*rectBorders.y+m_RectText.bottom>rectScreen.Height()*3/4)
		m_RectText.bottom = rectScreen.Height()*3/4-rectBorders.y;

	// Fenstergröße anpassen
	CRect rectClient;
	GetClientRect(rectClient);

	INT DiffX = max(0, m_RectText.right+2*rectBorders.x-rectLayout.Width());
	INT DiffY = max(0, max(m_IconSize, m_RectText.Height())+2*rectBorders.y-rectLayout.Height()+(rectClient.Height()-m_BottomDivider));

	CRect rectWindow;
	GetWindowRect(rectWindow);
	SetWindowPos(NULL, 0, 0, rectWindow.Width()+DiffX, rectWindow.Height()+DiffY, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);

	if (m_IconSize>m_RectText.Height())
	{
		m_RectText.OffsetRect(0, (m_IconSize-m_RectText.Height())/2);
	}
	else
	{
		m_IconPos.y += (m_RectText.Height()-m_IconSize)/2;
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

	return TRUE;
}


BEGIN_MESSAGE_MAP(LFMessageBoxDlg, LFDialog)
	ON_WM_DESTROY()
	ON_COMMAND_RANGE(IDOK, IDCONTINUE, OnButtonClicked)
END_MESSAGE_MAP()

void LFMessageBoxDlg::OnDestroy()
{
	DestroyIcon(m_hIcon);

	LFDialog::OnDestroy();
}

void LFMessageBoxDlg::OnButtonClicked(UINT nID)
{
	EndDialog(nID);
}
