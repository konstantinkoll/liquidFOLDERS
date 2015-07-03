
// CPropetyEdit.cpp: Implementierung der Klasse CProperyEdit
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CPropertyDisplay
//

CPropertyDisplay::CPropertyDisplay()
	: CWnd()
{
	p_Property = NULL;
}

BOOL CPropertyDisplay::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}

void CPropertyDisplay::SetProperty(CProperty* pProperty)
{
	p_Property = pProperty;
	Invalidate();
}


BEGIN_MESSAGE_MAP(CPropertyDisplay, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_GETDLGCODE()
	ON_WM_SETCURSOR()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

BOOL CPropertyDisplay::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CPropertyDisplay::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	BOOL Focused = GetFocus()==this;

	dc.FillSolidRect(rect, GetSysColor(Focused ? COLOR_HIGHLIGHT : COLOR_WINDOW));
	dc.SelectStockObject(DEFAULT_GUI_FONT);
	dc.SetTextColor(GetSysColor(Focused ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

	if (p_Property)
	{
		CRect rectValue(rect);
		rectValue.DeflateRect(1, 0);
		p_Property->DrawValue(dc, rectValue);
	}

	if (Focused)
	{
		dc.SetBkColor(0x000000);
		dc.DrawFocusRect(rect);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	pDC.SelectObject(pOldBitmap);
}

void CPropertyDisplay::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (p_Property)
		if (p_Property->WantsChars())
			if (p_Property->OnPushChar(nChar))
				return;

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CPropertyDisplay::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	SetFocus();
}

void CPropertyDisplay::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	if (p_Property)
		p_Property->OnClickValue(point.x);
}

UINT CPropertyDisplay::OnGetDlgCode()
{
	return DLGC_WANTARROWS | DLGC_WANTCHARS;
}

BOOL CPropertyDisplay::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*Message*/)
{
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	SetCursor(p_Property->SetCursor(point.x));
	return TRUE;
}

void CPropertyDisplay::OnSetFocus(CWnd* /*pOldWnd*/)
{
	Invalidate();
}

void CPropertyDisplay::OnKillFocus(CWnd* /*pNewWnd*/)
{
	Invalidate();
}


// CPropertyEdit
//

CPropertyEdit::CPropertyEdit()
	: CPropertyHolder()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
	wndcls.hIcon = NULL;
	wndcls.hCursor = LFGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.hbrBackground = NULL;
	wndcls.lpszMenuName = NULL;
	wndcls.lpszClassName = L"CPropertyEdit";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CPropertyEdit", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	LFInitVariantData(m_Data, LFAttrFileName);
	p_Property = NULL;
	m_pWndDisplay = NULL;
	m_pWndEdit = NULL;
	m_IsValid = FALSE;
	m_IsEmpty = TRUE;
}

BOOL CPropertyEdit::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE, rect, pParentWnd, nID);
}

void CPropertyEdit::PreSubclassWindow()
{
	CPropertyHolder::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (!pThreadState->m_pWndInit)
		Init();
}

void CPropertyEdit::Init()
{
	ModifyStyle(0, WS_CLIPCHILDREN);
	ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	CreateFonts();

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP;
	CRect rect;
	rect.SetRectEmpty();
	m_wndButton.Create(_T("..."), dwStyle, rect, this, 3);
	m_wndButton.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));
}

BOOL CPropertyEdit::PreTranslateMessage(MSG* pMsg)
{
	return CPropertyHolder::PreTranslateMessage(pMsg);
}

void CPropertyEdit::AdjustLayout()
{
	if (p_Property)
	{
		CRect rect;
		GetClientRect(rect);

		if (p_Property->HasButton())
		{
			INT btnWidth = rect.Height()*3/2;
			CRect rectButton(rect.right-btnWidth, rect.top, rect.right, rect.bottom);
			if (IsCtrlThemed())
				rectButton.InflateRect(1, 1);

			m_wndButton.SetWindowPos(NULL, rectButton.left, rectButton.top, rectButton.Width(), rectButton.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
			rect.right = rectButton.left-2;
		}
		else
		{
			m_wndButton.ShowWindow(SW_HIDE);
		}

		if (m_pWndDisplay)
		{
			m_pWndDisplay->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
			m_wndButton.SetWindowPos(m_pWndDisplay, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
		if (m_pWndEdit)
		{
			m_pWndEdit->SetWindowPos(NULL, rect.left, rect.top+1, rect.Width(), rect.Height()-2, SWP_NOZORDER | SWP_NOACTIVATE);
			m_wndButton.SetWindowPos(m_pWndEdit, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
	}
}

void CPropertyEdit::CreateProperty()
{
	if (m_pWndDisplay)
	{
		m_pWndDisplay->DestroyWindow();
		delete m_pWndDisplay;
		m_pWndDisplay = NULL;
	}
	if (m_pWndEdit)
	{
		m_pWndEdit->DestroyWindow();
		delete m_pWndEdit;
		m_pWndEdit = NULL;
	}

	delete p_Property;

	p_Property = CPropertyHolder::CreateProperty(&m_Data);

	CRect rect;
	rect.SetRectEmpty();

	if (p_Property->OnClickValue(-1))
	{
		m_pWndEdit = new CMFCMaskedEdit();
		m_pWndEdit->Create(WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_TABSTOP | ES_AUTOHSCROLL, rect, this, 1);
		if (!p_Property->m_Multiple)
		{
			WCHAR tmpStr[256];
			p_Property->ToString(tmpStr, 256);
			m_pWndEdit->SetWindowText(tmpStr);
		}
		m_pWndEdit->SetValidChars(p_Property->GetValidChars());
		p_Property->SetEditMask(m_pWndEdit);
		m_pWndEdit->SetLimitText(LFGetApp()->m_Attributes[m_Data.Attr].cCharacters);
		m_pWndEdit->SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));
	}
	else
	{
		m_pWndDisplay = new CPropertyDisplay();
		m_pWndDisplay->Create(this, 2);
		m_pWndDisplay->SetProperty(p_Property);
		m_pWndDisplay->EnableWindow(p_Property->WantsChars());
	}

	AdjustLayout();

	m_IsValid = TRUE;
	m_IsEmpty = LFIsNullVariantData(m_Data);
}

void CPropertyEdit::SetAttribute(UINT Attr)
{
	if ((Attr!=m_Data.Attr) || (!p_Property))
	{
		LFAttributeDescriptor* pAttr = &LFGetApp()->m_Attributes[Attr];
		if (pAttr->Type!=m_Data.Type)
		{
			LFInitVariantData(m_Data, Attr);
		}
		else
		{
			m_Data.Attr = Attr;

			switch (pAttr->Type)
			{
			case LFTypeUnicodeString:
				m_Data.UnicodeString[pAttr->cCharacters] = L'\0';
				break;
			case LFTypeAnsiString:
				m_Data.AnsiString[pAttr->cCharacters] = '\0';
				break;
			}
		}

		CreateProperty();
	}
}

void CPropertyEdit::SetData(LFVariantData* pData)
{
	ASSERT(pData);

	m_Data = *pData;
	CreateProperty();
}

void CPropertyEdit::NotifyOwner(SHORT Attr1, SHORT Attr2, SHORT Attr3)
{
	if (Attr1!=-1)
	{
		p_Property->m_Multiple = FALSE;
		RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW);

		m_IsValid = TRUE;
		m_IsEmpty = LFIsNullVariantData(m_Data);
	}

	GetOwner()->PostMessage(WM_PROPERTYCHANGED, Attr1, Attr2 | (Attr3 << 16));
}


BEGIN_MESSAGE_MAP(CPropertyEdit, CPropertyHolder)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_EN_CHANGE(1, OnChange)
	ON_MESSAGE(WM_PROPERTYCHANGED, OnPropertyChanged)
	ON_BN_CLICKED(3, OnClick)
END_MESSAGE_MAP()

INT CPropertyEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPropertyHolder::OnCreate(lpCreateStruct)==-1)
		return -1;

	Init();

	return 0;
}

void CPropertyEdit::OnDestroy()
{
	if (m_pWndDisplay)
	{
		m_pWndDisplay->DestroyWindow();
		delete m_pWndDisplay;
	}
	if (m_pWndEdit)
	{
		m_pWndEdit->DestroyWindow();
		delete m_pWndEdit;
	}

	delete p_Property;

	CPropertyHolder::OnDestroy();
}

BOOL CPropertyEdit::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CPropertyEdit::OnNcPaint()
{
	DrawControlBorder(this);
}

void CPropertyEdit::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	pDC.FillSolidRect(rect, GetSysColor(COLOR_WINDOW));
}

void CPropertyEdit::OnSize(UINT nType, INT cx, INT cy)
{
	CPropertyHolder::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CPropertyEdit::OnChange()
{
	PostMessage(WM_PROPERTYCHANGED);
}

LRESULT CPropertyEdit::OnPropertyChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	ASSERT(m_pWndEdit);

	WCHAR tmpStr[256];
	m_pWndEdit->GetWindowText(tmpStr, 256);

	LFVariantDataFromString(m_Data, tmpStr);
	m_IsValid = !m_Data.IsNull;
	m_IsEmpty = LFIsNullVariantData(m_Data);

	return GetOwner()->PostMessage(WM_PROPERTYCHANGED, m_Data.Attr);
}

void CPropertyEdit::OnClick()
{
	if (p_Property)
		if (p_Property->HasButton())
		{
			p_Property->OnClickButton();

			if (m_pWndEdit)
			{
				WCHAR tmpStr[256];
				LFVariantDataToString(m_Data, tmpStr, 256);

				m_pWndEdit->SetWindowText(tmpStr);
			}
		}
}
