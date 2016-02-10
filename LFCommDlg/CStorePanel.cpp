
// CStorePanel.cpp: Implementierung der Klasse CStorePanel
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CStorePanel
//

#define GUTTER     10

CStorePanel::CStorePanel()
	: CWnd()
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
	wndcls.lpszClassName = L"CStorePanel";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CStorePanel", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	p_Icons = NULL;
	p_Item = NULL;
}

CStorePanel::~CStorePanel()
{
	if (p_Item)
		LFFreeItemDescriptor(p_Item);
}

void CStorePanel::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	INT szSmallX = 16;
	INT szSmallY = 16;
	ImageList_GetIconSize(LFGetApp()->m_CoreImageListSmall, &szSmallX, &szSmallY);

	INT szLargeX = 32;
	INT szLargeY = 32;
	ImageList_GetIconSize(LFGetApp()->m_CoreImageListLarge, &szLargeX, &szLargeY);

	INT szExtraLargeX = 48;
	INT szExtraLargeY = 48;
	ImageList_GetIconSize(LFGetApp()->m_CoreImageListExtraLarge, &szExtraLargeX, &szExtraLargeY);

	INT szHugeX = 96;
	INT szHugeY = 96;
	ImageList_GetIconSize(LFGetApp()->m_CoreImageListHuge, &szHugeX, &szHugeY);

	CRect rect;
	GetClientRect(rect);

	if (rect.Height()>=szHugeY)
	{
		m_IconSizeX = szHugeX;
		m_IconSizeY = szHugeY;
		p_Icons = &LFGetApp()->m_CoreImageListHuge;
	}
	else
		if (rect.Height()>=szExtraLargeY)
		{
			m_IconSizeX = szExtraLargeX;
			m_IconSizeY = szExtraLargeY;
			p_Icons = &LFGetApp()->m_CoreImageListExtraLarge;
		}
		else
			if (rect.Height()>=szLargeY)
			{
				m_IconSizeX = szLargeX;
				m_IconSizeY = szLargeY;
				p_Icons = &LFGetApp()->m_CoreImageListLarge;
			}
			else
			{
				m_IconSizeX = szSmallX;
				m_IconSizeY = szSmallY;
				p_Icons = &LFGetApp()->m_CoreImageListSmall;
			}

	ModifyStyle(0, WS_DISABLED);
}

void CStorePanel::SetStore(const CHAR* StoreID)
{
	if (p_Item)
	{
		LFFreeItemDescriptor(p_Item);
		p_Item = NULL;
	}

	if (StoreID)
	{
		LFStoreDescriptor Store;
		if (LFGetStoreSettings(StoreID, &Store)==LFOk)
			p_Item = LFAllocItemDescriptorEx(&Store);
	}

	Invalidate();
}


BEGIN_MESSAGE_MAP(CStorePanel, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CStorePanel::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CStorePanel::OnPaint()
{
	ASSERT(p_Icons);

	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	FillRect(dc, rect, (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORSTATIC, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd));

	if (p_Item)
	{
		p_Icons->DrawEx(&dc, p_Item->IconID-1, CPoint(0, (rect.Height()-m_IconSizeY)/2+1), CSize(m_IconSizeX, m_IconSizeY), CLR_NONE, dc.GetBkColor(), (p_Item->Type & LFTypeGhosted) ? ILD_BLEND50 : ILD_TRANSPARENT);

		if (p_Item->Type & LFTypeGhosted)
			dc.SetTextColor(GetSysColor(COLOR_GRAYTEXT));

		WCHAR Buffer[256];
		LFAttributeToString(p_Item, LFAttrCreationTime, Buffer, 256);
		CString tmpStr(LFGetApp()->m_Attributes[LFAttrCreationTime].Name);
		tmpStr += _T(": ");
		tmpStr += Buffer;

		CONST INT FontHeight = LFGetApp()->m_DialogFont.GetFontHeight();

		INT Rows = 2;

		if (p_Item->CoreAttributes.Comments[0]!=L'\0')
			Rows++;

		if (p_Item->Description[0]!=L'\0')
			Rows++;

		const INT Height = max(0, (rect.Height()-Rows*FontHeight)/2);
		CRect rectText(m_IconSizeX+GUTTER, Height, rect.right, Height+FontHeight);

		CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DialogFont);

		dc.DrawText(p_Item->CoreAttributes.FileName, -1, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_LEFT | DT_NOPREFIX);
		rectText.OffsetRect(0, FontHeight);

		if (p_Item->CoreAttributes.Comments[0]!=L'\0')
		{
			dc.DrawText(p_Item->CoreAttributes.Comments, -1, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_LEFT | DT_NOPREFIX);
			rectText.OffsetRect(0, FontHeight);
		}

		if (p_Item->Description[0]!=L'\0')
		{
			dc.DrawText(p_Item->Description, -1, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_LEFT | DT_NOPREFIX);
			rectText.OffsetRect(0, FontHeight);
		}

		dc.DrawText(tmpStr, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_LEFT | DT_NOPREFIX);

		dc.SelectObject(pOldFont);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}
