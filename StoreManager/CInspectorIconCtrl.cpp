
// CInspectorIconCtrl.cpp: Implementierung der Klasse CInspectorIconCtrl
//

#include "stdafx.h"
#include "CInspectorIconCtrl.h"
#include "StoreManager.h"
#include "Resource.h"


// CInspectorIconCtrl
//

CInspectorIconCtrl::CInspectorIconCtrl()
	: CWnd()
{
	m_Status = IconEmpty;
}

BOOL CInspectorIconCtrl::Create(CWnd* pParentWnd, UINT nID)
{
	m_Empty.Load(IDB_INSPECTOR, _T("PNG"));
	m_Multiple.Load(IDB_MULTIPLE, _T("PNG"));

	ENSURE(m_strUnused.LoadString(IDS_NOITEMSSELECTED));
	m_strDescription = m_strUnused;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, theApp.LoadStandardCursor(IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}

void CInspectorIconCtrl::SetEmpty()
{
	m_Status = IconEmpty;
	m_strDescription = m_strUnused;

	Invalidate();
}

void CInspectorIconCtrl::SetMultiple(CString Description)
{
	m_Status = IconMultiple;
	m_strDescription = Description;

	Invalidate();
}

void CInspectorIconCtrl::SetCoreIcon(INT IconID, CString Description)
{
	m_Status = IconCore;
	m_IconID = IconID;
	m_strDescription = Description;

	Invalidate();
}

void CInspectorIconCtrl::SetFormatIcon(CHAR* FileFormat, CString Description)
{
	ASSERT(FileFormat);

	m_Status = IconExtension;
	strcpy_s(m_FileFormat, LFExtSize, FileFormat);
	m_strDescription = Description;

	Invalidate();
}

INT CInspectorIconCtrl::GetPreferredHeight()
{
	return 128+24;
}


BEGIN_MESSAGE_MAP(CInspectorIconCtrl, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

BOOL CInspectorIconCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CInspectorIconCtrl::OnPaint()
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

	BOOL Themed = IsCtrlThemed();

	dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

	const INT cx = (rect.Width()-128)/2;
	const INT cy = 4;
	CRect rectIcon(cx, cy, cx+128, cy+128);

	switch (m_Status)
	{
	case IconEmpty:
	case IconMultiple:
		{
			Graphics g(dc);
			g.SetCompositingMode(CompositingModeSourceOver);
			g.DrawImage((m_Status==IconEmpty) ? m_Empty.m_pBitmap : m_Multiple.m_pBitmap, cx, cy, 128, 128);
			break;
		}
	case IconCore:
		theApp.m_CoreImageListJumbo.DrawEx(&dc, m_IconID, CPoint(cx, cy), CSize(128, 128), CLR_NONE, 0xFFFFFF, ILD_TRANSPARENT);
		break;
	case IconExtension:
		theApp.m_FileFormats.DrawJumboIcon(dc, rectIcon, m_FileFormat);
		break;
	}

	CFont* pOldFont = (CFont*)dc.SelectStockObject(DEFAULT_GUI_FONT);
	dc.SetTextColor(m_Status==IconEmpty ? GetSysColor(COLOR_3DSHADOW) : Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));

	CRect rectText(rect);
	rectText.top = 128+6;
	dc.DrawText(m_strDescription, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER | DT_CENTER);

	dc.SelectObject(pOldFont);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CInspectorIconCtrl::OnSetFocus(CWnd* /*pOldWnd*/)
{
	GetParent()->SetFocus();
}

void CInspectorIconCtrl::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	SetFocus();
}
