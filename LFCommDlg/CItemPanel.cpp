
// CItemPanel.cpp: Implementierung der Klasse CItemPanel
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CItemPanel
//

#define GUTTER     10

CItemPanel::CItemPanel()
	: CWnd()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = LFGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CItemPanel";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CItemPanel", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	Empty();
}

BOOL CItemPanel::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONUP:
		LFGetApp()->HideTooltip();
		break;
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void CItemPanel::Empty()
{
	m_Text = _T("");
	m_Lines = 0;
	p_Icons = NULL;

	if (IsWindow(m_hWnd))
		Invalidate();
}

void CItemPanel::SetItem(CString Text, CImageList* pIcons, INT nID, BOOL Ghosted)
{
	if (pIcons)
	{
		INT cx;
		INT cy;
		ImageList_GetIconSize(*pIcons, &cx, &cy);

		m_IconSize = CSize(cx, cy);
	}

	m_Text = Text;
	m_Lines = 0;
	p_Icons = pIcons;
	m_IconID = nID;
	m_Ghosted = Ghosted;

	// Line count
	CRect rect;
	GetClientRect(rect);

	while (!Text.IsEmpty())
	{
		m_Lines++;

		INT Pos = Text.Find(L'\n');
		if (Pos<0)
			Pos = Text.GetLength();

		Text.Delete(0, Pos+1);
	}

	const UINT MaxLines = (UINT)max(1, rect.Height()/LFGetApp()->m_DialogFont.GetFontHeight());
	if (m_Lines>MaxLines)
		m_Lines = MaxLines;

	Invalidate();
}

BOOL CItemPanel::SetItem(const CHAR* pStoreID)
{
	ASSERT(pStoreID);

	LFStoreDescriptor Store;
	if (LFGetStoreSettings(pStoreID, &Store)==LFOk)
	{
		LFItemDescriptor* pItem = LFAllocItemDescriptorEx(&Store);

		// Text
		CString tmpStr(GetHintForItem(pItem));

		tmpStr.Insert(0, _T("\n"));
		tmpStr.Insert(0, pItem->CoreAttributes.FileName);

		// Icon
		CRect rect;
		GetClientRect(rect);

		CImageList* pImageList = (rect.Height()>=LFGetApp()->m_ExtraLargeIconSize) ? &LFGetApp()->m_CoreImageListExtraLarge : &LFGetApp()->m_CoreImageListSmall;

		SetItem(tmpStr, pImageList, pItem->IconID-1, pItem->Type & LFTypeGhosted);

		LFFreeItemDescriptor(pItem);

		return TRUE;
	}

	Empty();

	return FALSE;
}

BOOL CItemPanel::SetItem(LPITEMIDLIST pidlFQ, LPCWSTR Path, UINT nID, LPCWSTR Hint)
{
	ASSERT(pidlFQ);

	SHFILEINFO sfi;
	if (SUCCEEDED(SHGetFileInfo((LPCTSTR)pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_SYSICONINDEX)))
	{
		// Text
		CString tmpStr(sfi.szDisplayName);

		if (Path)
			if (*Path!=L'\0')
			{
				tmpStr.Append(_T("\n"));
				tmpStr.Append(Path);
			}

		if (Hint)
			if (*Hint!=L'\0')
			{
				tmpStr.Append(_T("\n"));

				if (nID)
				{
					CString Caption((LPCSTR)nID);
					tmpStr.Append(Caption+_T(": "));
				}

				tmpStr.Append(Hint);
			}

		// Icon
		CRect rect;
		GetClientRect(rect);

		CImageList* pImageList = (rect.Height()>=LFGetApp()->m_ExtraLargeIconSize) ? &LFGetApp()->m_SystemImageListExtraLarge : &LFGetApp()->m_SystemImageListSmall;

		SetItem(tmpStr, pImageList, sfi.iIcon);

		return TRUE;
	}

	Empty();

	return FALSE;
}

BOOL CItemPanel::SetItem(LPCWSTR Path, UINT nID, LPCWSTR Hint)
{
	ASSERT(Path);

	BOOL Result = FALSE;

	if (*Path)
	{
		const SFGAOF dwAttributes = SFGAO_HASSUBFOLDER | SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR | SFGAO_HASPROPSHEET | SFGAO_CANRENAME | SFGAO_CANDELETE;

		LPITEMIDLIST pidlFQ;
		if (SUCCEEDED(SHParseDisplayName(Path, NULL, &pidlFQ, dwAttributes, NULL)))
		{
			Result = SetItem(pidlFQ, Path, nID, Hint);
			LFGetApp()->GetShellManager()->FreeItem(pidlFQ);
		}
	}

	if (!Result)
		Empty();

	return Result;
}


BEGIN_MESSAGE_MAP(CItemPanel, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
END_MESSAGE_MAP()

BOOL CItemPanel::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CItemPanel::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	FillRect(dc, rect, (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORSTATIC, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd));

	CRect rectText(rect);

	// Icon
	if (p_Icons && (m_IconID>=0))
	{
		p_Icons->DrawEx(&dc, m_IconID, CPoint(0, (rect.Height()-m_IconSize.cy)/2), m_IconSize, CLR_NONE, dc.GetBkColor(), m_Ghosted ? ILD_BLEND50 : ILD_TRANSPARENT);

		rectText.left += m_IconSize.cx+GUTTER;
	}

	// Text
	CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DialogFont);

	if (m_Ghosted)
		dc.SetTextColor(IsCtrlThemed() ? 0x808080 : GetSysColor(COLOR_GRAYTEXT));

	const INT FontHeight = LFGetApp()->m_DialogFont.GetFontHeight();
	rectText.top = (rectText.Height()-m_Lines*FontHeight)/2;
	rectText.bottom = rectText.top+FontHeight;

	CString Text(m_Text);

	for (UINT a=0; a<m_Lines; a++)
	{
		INT Pos = Text.Find(L'\n');
		if (Pos<0)
			Pos = Text.GetLength();

		dc.DrawText(Text.Left(Pos), rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_LEFT | DT_NOPREFIX);
		Text.Delete(0, Pos+1);

		rectText.OffsetRect(0, FontHeight);
	}

	dc.SelectObject(pOldFont);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}

void CItemPanel::OnMouseMove(UINT /*nFlags*/, CPoint /*point*/)
{
	if (!m_Hover)
	{
		m_Hover = TRUE;

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = HOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
}

void CItemPanel::OnMouseLeave()
{
	LFGetApp()->HideTooltip();

	m_Hover = FALSE;
}

void CItemPanel::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if (!LFGetApp()->IsTooltipVisible() && !m_Text.IsEmpty())
		{
			INT Pos = m_Text.Find(L'\n');
			if (Pos<0)
				Pos = m_Text.GetLength();

			LFGetApp()->ShowTooltip(this, point, m_Text.Left(Pos), m_Text.Mid(Pos+1), p_Icons ? p_Icons->ExtractIcon(m_IconID) : NULL);
		}
	}
	else
	{
		LFGetApp()->HideTooltip();
	}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = HOVERTIME;
	tme.hwndTrack = m_hWnd;
	TrackMouseEvent(&tme);
}
