
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
	m_Status = StatusUnused;
	m_Icon = NULL;
	m_IconSize = 128;
	m_Empty = NULL;
	m_Multiple = NULL;
}

CInspectorIconCtrl::~CInspectorIconCtrl()
{
	if (m_Icon)
		DestroyIcon(m_Icon);
	if (m_Empty)
		delete m_Empty;
	if (m_Multiple)
		delete m_Multiple;
}

BOOL CInspectorIconCtrl::Create(CWnd* pParentWnd, UINT nID)
{
	// Bilder laden
	m_Empty = new CGdiPlusBitmapResource();
	m_Empty->Load(IDB_INSPECTOR, _T("PNG"));
	m_Multiple = new CGdiPlusBitmapResource();
	m_Multiple->Load(IDB_MULTIPLE, _T("PNG"));

	m_Description_Unused.LoadString(IDS_NOITEMSSELECTED);
	m_Description = m_Description_Unused;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}

void CInspectorIconCtrl::SetStatus(UINT _status, HICON _icon, CString _description)
{
	ASSERT((_status==StatusUnused) || (_status==StatusUsed) || (_status==StatusMultiple));

	if (m_Icon)
	{
		DestroyIcon(m_Icon);
		m_Icon = NULL;
	}

	m_Status = _status;
	switch (_status)
	{
	case StatusUnused:
		m_Description = m_Description_Unused;
		break;
	case StatusUsed:
		ASSERT(_icon);
		m_Icon = _icon;
	case StatusMultiple:
		m_Description = _description;
		break;
	}

	Invalidate();
}

INT CInspectorIconCtrl::GetPreferredHeight(INT cx)
{
	INT Height = (cx>128 ? 128 : cx);
	if (Height<16)
		Height = 16;

	return Height + 24;
}


BEGIN_MESSAGE_MAP(CInspectorIconCtrl, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_SIZE()
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

	dc.FillSolidRect(rect, afxGlobalData.clrBarFace);

	INT cx = (rect.Width()-m_IconSize)/2;
	const INT cy = 4;

	if (m_Status==StatusUsed)
	{
		DrawIconEx(dc, cx, cy, m_Icon, m_IconSize, m_IconSize, 0, NULL, DI_NORMAL);
	}
	else
	{
		Graphics g(dc);
		g.SetCompositingMode(CompositingModeSourceOver);

		CGdiPlusBitmapResource* i = (m_Status == StatusUnused) ? m_Empty : m_Multiple;
		g.DrawImage(i->m_pBitmap, cx, cy, m_IconSize, m_IconSize);
	}

	CFont* pOldFont = (CFont*)dc.SelectStockObject(DEFAULT_GUI_FONT);
	dc.SetTextColor(m_Status==StatusUnused ? afxGlobalData.clrBarDkShadow : afxGlobalData.clrBarText);

	CRect rectText;
	rectText.top = m_IconSize + 6;
	rectText.left = 0;
	rectText.right = rect.right;
	rectText.bottom = rect.bottom;
	dc.DrawText(m_Description, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER | DT_CENTER);

	dc.SelectObject(pOldFont);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CInspectorIconCtrl::OnSetFocus(CWnd* /*pOldWnd*/)
{
	CWnd* pParent = GetParent();

	if (pParent)
		pParent->SetFocus();
}

void CInspectorIconCtrl::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	OnSetFocus(NULL);
}

void CInspectorIconCtrl::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);

	m_IconSize = (cx>128 ? 128 : cx);
	if (m_IconSize<16)
		m_IconSize = 16;
}
