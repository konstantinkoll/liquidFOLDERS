
// CLabel.cpp: Implementierung der Klasse CLabel
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CLabel
//

#define GUTTER      3
#define PADDING     2

CLabel::CLabel()
	: CWnd()
{
	p_App = LFGetApp();
	m_FontHeight = 0;
}

BOOL CLabel::Create(CWnd* pParentWnd, UINT nID, CString Text)
{
	m_Text = Text;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_DISABLED;
	CRect rect;
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}

void CLabel::SetText(CString Text)
{
	m_Text = Text;
	Invalidate();
}

INT CLabel::GetPreferredHeight()
{
	return m_FontHeight+2*PADDING;
}


BEGIN_MESSAGE_MAP(CLabel, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

INT CLabel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&p_App->m_DefaultFont);
	m_FontHeight = dc->GetTextExtent(_T("Wy")).cy;
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	return 0;
}

BOOL CLabel::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CLabel::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	HBRUSH brush = (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd);
	if (brush)
		FillRect(dc, rect, brush);

	BOOL Themed = IsCtrlThemed();

	CFont* pOldFont = dc.SelectObject(&p_App->m_DefaultFont);
	dc.SetTextColor(Themed ? 0x993300 : GetSysColor(COLOR_WINDOWTEXT));

	CRect rectText(rect.left+GUTTER, rect.top+PADDING, rect.right, rect.bottom-PADDING);
	dc.DrawText(m_Text, rectText, DT_LEFT | DT_BOTTOM | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

	CRect rectLine(rectText);
	dc.DrawText(m_Text, rectLine, DT_LEFT | DT_BOTTOM | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_CALCRECT);
	rectLine.right += 2*PADDING;

	dc.SelectObject(pOldFont);

	if (rectLine.right<=rectText.right)
	{
		CPen pen(PS_SOLID, 1, Themed ? 0xE2E2E2 : GetSysColor(COLOR_WINDOWTEXT));

		CPen* pOldPen = dc.SelectObject(&pen);
		dc.MoveTo(rectLine.right, rectText.bottom-(m_FontHeight+1)/2);
		dc.LineTo(rectText.right-PADDING, rectText.bottom-(m_FontHeight+1)/2);
		dc.SelectObject(pOldPen);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}
