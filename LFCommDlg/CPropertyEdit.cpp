
// CPropetyEdit.cpp: Implementierung der Klasse CProperyEdit
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CPropertyEdit
//

#define BORDER      2
#define PADDING     1

CPropertyEdit::CPropertyEdit()
	: CPropertyHolder()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = LFGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CPropertyEdit";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CPropertyEdit", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	LFInitVariantData(m_VData, LFAttrFileName);

	m_pProperty = NULL;
	m_pWndEdit = NULL;
	m_ButtonWidth = 0;
}

BOOL CPropertyEdit::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE, CRect(0, 0, 0, 0), pParentWnd, nID);
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

	m_wndButton.Create(_T("..."), this, 2);
	m_wndButton.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));
}

void CPropertyEdit::AdjustLayout()
{
	if (!m_pProperty)
		return;

	CRect rect;
	GetClientRect(rect);

	// Button
	if (m_pProperty->HasButton())
	{
		m_ButtonWidth = rect.Height()*3/2;

		CRect rectButton(rect.right-m_ButtonWidth, rect.top, rect.right, rect.bottom);
		if (IsCtrlThemed())
			rectButton.InflateRect(1, 1);

		m_wndButton.SetWindowPos(NULL, rectButton.left, rectButton.top, rectButton.Width(), rectButton.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

		m_ButtonWidth += BORDER;
	}
	else
	{
		m_ButtonWidth = 0;

		m_wndButton.ShowWindow(SW_HIDE);
	}

	// Edit control
	if (m_pWndEdit)
	{
		m_pWndEdit->SetWindowPos(NULL, rect.left, rect.top+1, rect.Width()-m_ButtonWidth, rect.Height()-2, SWP_NOZORDER | SWP_NOACTIVATE);
		m_wndButton.SetWindowPos(m_pWndEdit, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	Invalidate();
}

void CPropertyEdit::CreateProperty()
{
	DestroyEdit();

	m_pProperty = CPropertyHolder::CreateProperty(&m_VData);

	if (m_pProperty->OnClickValue(-1))
		m_pWndEdit = m_pProperty->CreateEditControl(CRect(0, 0, 0, 0), this);

	AdjustLayout();
}

void CPropertyEdit::SetInitialData(const LFVariantData& VData)
{
	m_VData = VData;

	CreateProperty();
}

void CPropertyEdit::SetAttribute(UINT Attr)
{
	if ((Attr!=m_VData.Attr) || !m_pProperty)
	{
		const LFAttributeDescriptor* pAttribute = &LFGetApp()->m_Attributes[Attr];

		if (pAttribute->AttrProperties.Type!=m_VData.Type)
		{
			LFInitVariantData(m_VData, Attr);
		}
		else
		{
			m_VData.Attr = Attr;

			switch (pAttribute->AttrProperties.Type)
			{
			case LFTypeUnicodeString:
			case LFTypeUnicodeArray:
				m_VData.UnicodeString[pAttribute->AttrProperties.cCharacters] = L'\0';
				break;

			case LFTypeAnsiString:
			case LFTypeIATACode:
				m_VData.AnsiString[pAttribute->AttrProperties.cCharacters] = '\0';
				break;
			}
		}

		CreateProperty();
	}
}

void CPropertyEdit::DestroyEdit()
{
	if (m_pWndEdit)
	{
		m_pWndEdit->DestroyWindow();
		delete m_pWndEdit;

		m_pWndEdit = NULL;
	}

	delete m_pProperty;
}

void CPropertyEdit::NotifyOwner(SHORT Attr1, SHORT Attr2, SHORT Attr3)
{
	if (Attr1!=-1)
	{
		m_pProperty->UpdateState(FALSE);
		RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW);
	}

	GetOwner()->PostMessage(WM_PROPERTYCHANGED, Attr1, Attr2 | (Attr3 << 16));
}


BEGIN_MESSAGE_MAP(CPropertyEdit, CPropertyHolder)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_GETDLGCODE()
	ON_WM_SETCURSOR()
	ON_WM_SETFOCUS()

	ON_EN_CHANGE(1, OnChange)
	ON_BN_CLICKED(2, OnClick)
	ON_MESSAGE(WM_PROPERTYCHANGED, OnPropertyChanged)
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
	DestroyEdit();

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

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	dc.FillSolidRect(rect, GetSysColor(COLOR_WINDOW));

	if (!m_pWndEdit)
	{
		const BOOL Focused = (GetFocus()==this);

		CRect rectValue(rect.left, rect.top, rect.right-m_ButtonWidth, rect.bottom);
		dc.FillSolidRect(rectValue, GetSysColor(Focused ? COLOR_HIGHLIGHT : COLOR_WINDOW));

		if (m_pProperty)
		{
			dc.SelectStockObject(DEFAULT_GUI_FONT);
			dc.SetTextColor(GetSysColor(Focused ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

			m_pProperty->DrawValue(dc, CRect(rectValue.left+PADDING, rectValue.top, rectValue.right-PADDING, rectValue.bottom));

			if (Focused)
			{
				dc.SetBkColor(0x000000);
				dc.DrawFocusRect(rectValue);
			}
		}
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}

HBRUSH CPropertyEdit::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hBrush = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	if (nCtlColor==CTLCOLOR_BTN)
	{
		pDC->SetDCBrushColor(GetSysColor(COLOR_WINDOW));
		hBrush = (HBRUSH)GetStockObject(DC_BRUSH);
	}

	return hBrush;
}

void CPropertyEdit::OnSize(UINT nType, INT cx, INT cy)
{
	CPropertyHolder::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CPropertyEdit::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	SetFocus();
}

void CPropertyEdit::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	if (m_pProperty && !m_pWndEdit)
		m_pProperty->OnClickValue(point.x-PADDING);
}

void CPropertyEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_pProperty && m_pProperty->WantsChars() && m_pProperty->OnPushChar(nChar))
		return;

	CPropertyHolder::OnKeyDown(nChar, nRepCnt, nFlags);
}

UINT CPropertyEdit::OnGetDlgCode()
{
	return DLGC_WANTARROWS | DLGC_WANTCHARS;
}

BOOL CPropertyEdit::OnSetCursor(CWnd* pWnd, UINT /*nHitTest*/, UINT /*Message*/)
{
	if (m_pProperty)
	{
		CPoint point;
		GetCursorPos(&point);
		ScreenToClient(&point);

		SetCursor((pWnd!=&m_wndButton) && (point.x>PADDING) ? m_pProperty->SetCursor(point.x-PADDING) : LFGetApp()->LoadStandardCursor(IDC_ARROW));
	}

	return TRUE;
}

void CPropertyEdit::OnSetFocus(CWnd* pOldWnd)
{
	if (m_pWndEdit)
	{
		m_pWndEdit->SetFocus();
	}
	else
		if (m_ButtonWidth)
		{
			m_wndButton.SetFocus();
		}

	CPropertyHolder::OnSetFocus(pOldWnd);
}


void CPropertyEdit::OnChange()
{
	PostMessage(WM_PROPERTYCHANGED);
}

void CPropertyEdit::OnClick()
{
	if (m_pProperty)
		if (m_pProperty->HasButton())
		{
			m_pProperty->OnClickButton();

			// Update edit control
			if (m_pWndEdit)
			{
				WCHAR tmpStr[256];
				LFVariantDataToString(m_VData, tmpStr, 256);

				m_pWndEdit->SetWindowText(tmpStr);
			}
		}
}

LRESULT CPropertyEdit::OnPropertyChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	ASSERT(m_pWndEdit);

	WCHAR tmpStr[256];
	m_pWndEdit->GetWindowText(tmpStr, 256);

	LFVariantDataFromString(m_VData, tmpStr);

	return GetOwner()->PostMessage(WM_PROPERTYCHANGED, m_VData.Attr);
}
