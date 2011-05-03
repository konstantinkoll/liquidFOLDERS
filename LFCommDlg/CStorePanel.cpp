
// CStorePanel.cpp: Implementierung der Klasse CStorePanel
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CStorePanel
//

#define GUTTER     6

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

CStorePanel::CStorePanel()
	: CWnd()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
	wndcls.hIcon = NULL;
	wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndcls.hbrBackground = NULL;
	wndcls.lpszMenuName = NULL;
	wndcls.lpszClassName = L"CStorePanel";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CStorePanel", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}
	if (!(::GetClassInfo(LFCommDlgDLL.hModule, L"CStorePanel", &wndcls)))
	{
		wndcls.hInstance = LFCommDlgDLL.hModule;

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	p_App = (LFApplication*)AfxGetApp();
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
	ImageList_GetIconSize(p_App->m_CoreImageListSmall, &szSmallX, &szSmallY);

	INT szLargeX = 32;
	INT szLargeY = 32;
	ImageList_GetIconSize(p_App->m_CoreImageListLarge, &szLargeX, &szLargeY);

	INT szExtraLargeX = 48;
	INT szExtraLargeY = 48;
	ImageList_GetIconSize(p_App->m_CoreImageListExtraLarge, &szExtraLargeX, &szExtraLargeY);

	CRect rect;
	GetClientRect(rect);

	if (rect.Height()>=szExtraLargeY)
	{
		m_IconSizeX = szExtraLargeX;
		m_IconSizeY = szExtraLargeY;
		p_Icons = &p_App->m_CoreImageListExtraLarge;
	}
	else
		if (rect.Height()>=szLargeY)
		{
			m_IconSizeX = szLargeX;
			m_IconSizeY = szLargeY;
			p_Icons = &p_App->m_CoreImageListLarge;
		}
		else
		{
			m_IconSizeX = szSmallX;
			m_IconSizeY = szSmallY;
			p_Icons = &p_App->m_CoreImageListSmall;
		}

	ModifyStyle(0, WS_DISABLED);
}

void CStorePanel::SetStore(CHAR* Key)
{
	if (p_Item)
	{
		LFFreeItemDescriptor(p_Item);
		p_Item = NULL;
	}

	if (Key)
	{
		LFStoreDescriptor* s = LFAllocStoreDescriptor();

		if (LFGetStoreSettings(Key, s)==LFOk)
			p_Item = LFAllocItemDescriptor(s);

		LFFreeStoreDescriptor(s);
	}

	Invalidate();
}

BOOL CStorePanel::IsValidStore()
{
	return p_Item!=NULL;
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

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	HBRUSH brush = (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd);
	if (brush)
		FillRect(dc, rect, brush);

	if (p_Item)
	{
		p_Icons->DrawEx(&dc, p_Item->IconID-1, CPoint(0, (rect.Height()-m_IconSizeY)/2), CSize(m_IconSizeX, m_IconSizeY), CLR_NONE, dc.GetBkColor(), (p_Item->Type & LFTypeGhosted) ? ILD_BLEND50 : ILD_TRANSPARENT);
		if (p_Item->Type & LFTypeGhosted)
			dc.SetTextColor(GetSysColor(COLOR_GRAYTEXT));

		WCHAR Buffer[256];
		LFAttributeToString(p_Item, LFAttrCreationTime, Buffer, 256);
		CString tmpStr(p_App->m_Attributes[LFAttrCreationTime]->Name);
		tmpStr += _T(": ");
		tmpStr += Buffer;

		CFont* pOldFont = (CFont*)dc.SelectStockObject(DEFAULT_GUI_FONT);
		INT cy = dc.GetTextExtent(_T("Wy")).cy;

		INT rows = 2;
		if (p_Item->CoreAttributes.Comment[0]!=L'\0')
			rows++;
		if (p_Item->Description[0]!=L'\0')
			rows++;

		INT y = max(0, (rect.Height()-rows*cy)/2);
		CRect rectText(m_IconSizeX+GUTTER, y, rect.right, y+cy);

		dc.DrawText(p_Item->CoreAttributes.FileName, -1, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_LEFT);
		rectText.OffsetRect(0, cy);
		if (p_Item->CoreAttributes.Comment[0]!=L'\0')
		{
			dc.DrawText(p_Item->CoreAttributes.Comment, -1, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_LEFT);
			rectText.OffsetRect(0, cy);
		}
		if (p_Item->Description[0]!=L'\0')
		{
			dc.DrawText(p_Item->Description, -1, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_LEFT);
			rectText.OffsetRect(0, cy);
		}
		dc.DrawText(tmpStr, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_LEFT);

		dc.SelectObject(pOldFont);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}
