
// CPropetyEdit.cpp: Implementierung der Klasse CProperyEdit
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CPropertyEdit
//

#define PADDING     1

#define WM_TEXTCHANGED     WM_USER+10

CPropertyEdit::CPropertyEdit()
	: CFrontstageWnd()
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

	// Data
	LFInitVariantData(m_VData, LFAttrFileName);

	m_pProperty = NULL;
	m_pWndEdit = NULL;
	m_ButtonWidth = 0;
}

void CPropertyEdit::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	// Style is neccessary!
	ASSERT(GetStyle() & WS_CLIPCHILDREN);

	m_wndButton.Create(_T("..."), this, 1, TRUE);
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

		m_ButtonWidth += ITEMCELLPADDINGY;
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

void CPropertyEdit::CreateProperty()
{
	DestroyEdit();

	m_pProperty = CProperty::CreateProperty(&m_VData, this);

	if (m_pProperty->OnClickValue(-1))
		m_pWndEdit = m_pProperty->CreateEditControl(CRect(0, 0, 0, 0));

	AdjustLayout();
}

void CPropertyEdit::SetInitialData(const LFVariantData& VData, const STOREID& StoreID)
{
	m_VData = VData;
	m_StoreID = StoreID;

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

			// Cut off any strings that are too long
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


BEGIN_MESSAGE_MAP(CPropertyEdit, CFrontstageWnd)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_SETCURSOR()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_GETDLGCODE()

	ON_MESSAGE(WM_PROPERTYCHANGED, OnPropertyChanged)
	ON_MESSAGE(WM_TEXTCHANGED, OnTextChanged)

	ON_BN_CLICKED(1, OnClick)
	ON_EN_CHANGE(2, OnChange)
END_MESSAGE_MAP()

void CPropertyEdit::OnDestroy()
{
	DestroyEdit();

	CFrontstageWnd::OnDestroy();
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

	// Background
	dc.FillSolidRect(rect, GetSysColor(COLOR_WINDOW));

	// Property
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

			// Focus rectangle
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

void CPropertyEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_pProperty && m_pProperty->WantsChars() && m_pProperty->OnPushChar(nChar))
		return;

	CFrontstageWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CPropertyEdit::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	if (GetFocus()!=this)
		SetFocus();
}

void CPropertyEdit::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	if (m_pProperty && !m_pWndEdit)
		m_pProperty->OnClickValue(point.x-PADDING);
}

void CPropertyEdit::OnRButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	if (GetFocus()!=this)
		SetFocus();
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

void CPropertyEdit::OnSetFocus(CWnd* /*pOldWnd*/)
{
	if (m_pWndEdit)
	{
		m_pWndEdit->SetFocus();
	}
	else
	{
		if (m_ButtonWidth)
			m_wndButton.SetFocus();
	}

	Invalidate();
}

void CPropertyEdit::OnKillFocus(CWnd* /*pOldWnd*/)
{
	Invalidate();
}

UINT CPropertyEdit::OnGetDlgCode()
{
	return DLGC_WANTARROWS | DLGC_WANTCHARS;
}


LRESULT CPropertyEdit::OnPropertyChanged(WPARAM wParam, LPARAM lParam)
{
	// Redraw property
	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW);

	return GetOwner()->SendMessage(WM_PROPERTYCHANGED, wParam, lParam);
}

LRESULT CPropertyEdit::OnTextChanged(WPARAM wParam, LPARAM lParam)
{
	if (m_pWndEdit)
	{
		WCHAR tmpStr[256];
		m_pWndEdit->GetWindowText(tmpStr, 256);

		LFVariantDataFromString(m_VData, tmpStr);

		return GetOwner()->SendMessage(WM_PROPERTYCHANGED, wParam, lParam);
	}

	return NULL;
}


void CPropertyEdit::OnClick()
{
	if (m_pProperty && m_pProperty->HasButton())
	{
		m_pProperty->OnClickButton(m_StoreID);

		// Update edit control
		if (m_pWndEdit)
		{
			WCHAR tmpStr[256];
			LFVariantDataToString(m_VData, tmpStr, 256);

			m_pWndEdit->SetWindowText(tmpStr);
		}
	}
}

void CPropertyEdit::OnChange()
{
	PostMessage(WM_TEXTCHANGED);
}
