
// CExplorerHeader.cpp: Implementierung der Klasse CExplorerHeader
//

#include "stdafx.h"
#include "CExplorerHeader.h"
#include "CGlasWindow.h"
#include "LFApplication.h"


// CExplorerHeader
//

#define BORDERLEFT      16
#define BORDER          10

CExplorerHeader::CExplorerHeader()
	: CWnd()
{
	m_CaptionCol = 0x993300;
	m_HintCol = 0x79675A;
	m_BackCol = 0xFFFFFF;
	m_LineCol = 0xF5E5D6;
	m_hBackgroundBrush = NULL;
	m_GradientLine = TRUE;
	m_Design = GWD_DEFAULT;
}

BOOL CExplorerHeader::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW), NULL, NULL);

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T("ExplorerHeader"), dwStyle, rect, pParentWnd, nID);
}

void CExplorerHeader::SetText(CString _Caption, CString _Hint, BOOL Repaint)
{
	m_Caption = _Caption;
	m_Hint = _Hint;

	if (Repaint)
		Invalidate();
}

void CExplorerHeader::SetColors(COLORREF CaptionCol, COLORREF HintCol, COLORREF BackCol, COLORREF LineCol, BOOL Repaint)
{
	m_CaptionCol = CaptionCol;
	m_HintCol = HintCol;
	m_BackCol = BackCol;
	m_LineCol = LineCol;

	if (Repaint)
		Invalidate();
}

void CExplorerHeader::SetLineStyle(BOOL GradientLine, BOOL Repaint)
{
	m_GradientLine = GradientLine;

	if (Repaint)
		Invalidate();
}

void CExplorerHeader::SetDesign(UINT _Design)
{
	m_Design = _Design;
}

UINT CExplorerHeader::GetPreferredHeight()
{
	LOGFONT lf;
	UINT h = 3*BORDER;

	((LFApplication*)AfxGetApp())->m_CaptionFont.GetLogFont(&lf);
	h += abs(lf.lfHeight);

	((LFApplication*)AfxGetApp())->m_DefaultFont.GetLogFont(&lf);
	h += abs(lf.lfHeight);

	return max(h, 60);
}


BEGIN_MESSAGE_MAP(CExplorerHeader, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

int CExplorerHeader::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	m_Background.LoadBitmap(IDB_EXPLORERGRADIENT);
	m_hBackgroundBrush = CreatePatternBrush(m_Background);

	return 0;
}

void CExplorerHeader::OnDestroy()
{
	CWnd::OnDestroy();

	if (m_hBackgroundBrush)
		DeleteObject(m_hBackgroundBrush);
}

BOOL CExplorerHeader::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CExplorerHeader::OnPaint()
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

	if ((m_BackCol==0xFFFFFF) && (m_Design>GWD_DEFAULT))
	{
		CRect rectBackground(rect);
		if (rectBackground.bottom>60)
		{
			rectBackground.bottom = 60;
			dc.FillSolidRect(0, 60, rect.Width(), rect.Height()-60, 0xFFFFFF);
		}
		FillRect(dc, rectBackground, m_hBackgroundBrush);
	}
	else
	{
		dc.FillSolidRect(rect, m_BackCol);
	}

	if (m_Design>GWD_DEFAULT)
		if (m_GradientLine)
		{
			if (rect.Width()>4*BORDERLEFT)
			{
				Graphics g(dc);

				Color c1;
				c1.SetFromCOLORREF(m_BackCol);
				Color c2;
				c2.SetFromCOLORREF(m_LineCol);

				LinearGradientBrush brush1(Point(0, 0), Point(BORDERLEFT*2, 0), c1, c2);
				g.FillRectangle(&brush1, 0, rect.bottom-1, BORDERLEFT*2, 1);

				LinearGradientBrush brush2(Point(rect.right, 0), Point(rect.right-BORDERLEFT*2, 0), c1, c2);
				g.FillRectangle(&brush2, rect.right-BORDERLEFT*2, rect.bottom-1, BORDERLEFT*2, 1);

				dc.FillSolidRect(BORDERLEFT*2, rect.bottom-1, rect.Width()-BORDERLEFT*4, 1, m_LineCol);
			}
		}
		else
		{
			dc.FillSolidRect(0, rect.bottom-1, rect.Width(), 1, m_LineCol);
		}

	CFont* pOldFont = dc.SelectObject(&((LFApplication*)AfxGetApp())->m_CaptionFont);
	CSize sz = dc.GetTextExtent(m_Caption, m_Caption.GetLength());
	CRect rectText(BORDERLEFT, BORDER, rect.right, BORDER+sz.cy);
	dc.SetTextColor((m_Design==GWD_DEFAULT) ? 0x000000 : m_CaptionCol);
	dc.DrawText(m_Caption, rectText, DT_SINGLELINE | DT_END_ELLIPSIS);

	dc.SelectObject(&((LFApplication*)AfxGetApp())->m_DefaultFont);
	sz = dc.GetTextExtent(m_Hint, m_Hint.GetLength());
	rectText.SetRect(BORDERLEFT, rect.bottom-sz.cy-BORDER-1, rect.right, rect.bottom-BORDER-1);
	dc.SetTextColor((m_Design==GWD_DEFAULT) ? 0x000000 : m_HintCol);
	dc.DrawText(m_Hint, rectText, DT_SINGLELINE | DT_END_ELLIPSIS);

	dc.SelectObject(pOldFont);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}
