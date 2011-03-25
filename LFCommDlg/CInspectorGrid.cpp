
// CInspectorGrid.cpp: Implementierung der Klasse CInspectorGrid
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CInspectorProperty
//

CInspectorProperty::CInspectorProperty(LFVariantData* pData)
{
	p_Data = pData;
	p_Parent = NULL;
	m_Modified = m_Multiple = FALSE;
}

void CInspectorProperty::SetParent(CInspectorGrid* pParent)
{
	p_Parent = pParent;
}

void CInspectorProperty::SetMultiple(BOOL Multiple)
{
	m_Multiple = Multiple;
}

void CInspectorProperty::DrawValue(CDC& dc, CRect rect)
{
	ASSERT(p_Parent);

	if (m_Multiple)
	{
		CFont* pOldFont = dc.SelectObject(&p_Parent->m_ItalicFont);
		dc.DrawText(p_Parent->m_MultipleValues, rect, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS | DT_SINGLELINE);
		dc.SelectObject(pOldFont);
	}
	else
	{
		WCHAR tmpStr[256];
		LFVariantDataToString(p_Data, tmpStr, 256);

		CFont* pOldFont = m_Modified ? dc.SelectObject(&p_Parent->m_BoldFont) : NULL;
		dc.DrawText(tmpStr, -1, rect, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS | DT_SINGLELINE);
		if (pOldFont)
			dc.SelectObject(pOldFont);
	}
}


// CInspectorPropertyRating
//

CInspectorPropertyRating::CInspectorPropertyRating(LFVariantData* pData)
	: CInspectorProperty(pData)
{
}

void CInspectorPropertyRating::DrawValue(CDC& dc, CRect rect)
{
	LFApplication* pApp = (LFApplication*)AfxGetApp();

	HDC hdcMem = CreateCompatibleDC(dc);
	UCHAR level = m_Multiple ? 0 : p_Data->Rating;
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, p_Data->Attr==LFAttrRating ? pApp->m_RatingBitmaps[level] : pApp->m_PriorityBitmaps[level]);

	INT w = min(rect.Width()-6, RatingBitmapWidth);
	INT h = min(rect.Height(), RatingBitmapHeight);
	BLENDFUNCTION LF = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };
	AlphaBlend(dc, rect.left+6, rect.top+(rect.Height()-h)/2, w, h, hdcMem, 0, 0, w, h, LF);

	SelectObject(hdcMem, hbmOld);
	DeleteDC(hdcMem);
}


// CInspectorHeader
//

INT CInspectorHeader::GetPreferredHeight()
{
	return 0;
}

void CInspectorHeader::DrawHeader(CDC& /*dc*/, CRect /*rect*/, BOOL /*Themed*/)
{
}


// CInspectorGrid
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

#define GUTTER      3
#define PADDING     2

CInspectorGrid::CInspectorGrid()
	: CWnd()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = LFCommDlgDLL.hModule;

	if (!(::GetClassInfo(hInst, L"CInspectorGrid", &wndcls)))
	{
		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = hInst;
		wndcls.hIcon = NULL;
		wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndcls.hbrBackground = NULL;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = L"CInspectorGrid";

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	p_App = (LFApplication*)AfxGetApp();
	m_ShowHeader = m_SortAlphabetic = FALSE;
	m_pSortArray = NULL;
	m_pHeader = NULL;
	hThemeList = hThemeButton = NULL;
	m_VScrollMax = m_VScrollPos = 0;

	ENSURE(m_MultipleValues.LoadString(IDS_MULTIPLEVALUES));
}

CInspectorGrid::~CInspectorGrid()
{
	if (m_pSortArray)
		delete[] m_pSortArray;
}

BOOL CInspectorGrid::Create(CWnd* pParentWnd, UINT nID, CInspectorHeader* pHeader)
{
	m_pHeader = pHeader;
	m_ShowHeader = (pHeader!=NULL);

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}

void CInspectorGrid::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (!pThreadState->m_pWndInit)
		Init();
}

void CInspectorGrid::Init()
{
	if (p_App->m_ThemeLibLoaded)
	{
		hThemeButton = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
		if (p_App->OSVersion>=OS_Vista)
		{
			p_App->zSetWindowTheme(GetSafeHwnd(), L"explorer", NULL);
			hThemeList = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
		}
	}

	ResetScrollbars();
	CreateFonts();

	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&p_App->m_DefaultFont);
	m_FontHeight[1] = dc->GetTextExtent(_T("Wy")).cy;
	dc->SelectStockObject(DEFAULT_GUI_FONT);
	m_FontHeight[0] = dc->GetTextExtent(_T("Wy")).cy;
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	m_RowHeight = max(m_FontHeight[0], 16);
}

void CInspectorGrid::CreateFonts()
{
	if (m_BoldFont.GetSafeHandle())
		m_BoldFont.DeleteObject();
	if (m_ItalicFont.GetSafeHandle())
		m_ItalicFont.DeleteObject();

	CFont* pFont = CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	ASSERT_VALID(pFont);

	LOGFONT lf;
	pFont->GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;
	m_BoldFont.CreateFontIndirect(&lf);

	pFont->GetLogFont(&lf);
	lf.lfItalic = TRUE;
	m_ItalicFont.CreateFontIndirect(&lf);
}

void CInspectorGrid::AddProperty(CInspectorProperty* pProperty, UINT Category, WCHAR* Name, BOOL Editable)
{
	pProperty->SetParent(this);

	Property prop;
	prop.pProperty = pProperty;
	prop.Category = Category;
	wcscpy_s(prop.Name, 256, Name);
	wcscat_s(prop.Name, 256, L":");
	prop.Visible = FALSE;
	prop.Editable = Editable;
	prop.Top = prop.Bottom = prop.LabelWidth = -1;

	CDC* dc = GetWindowDC();
	CGdiObject* pOldFont = dc->SelectStockObject(DEFAULT_GUI_FONT);
	prop.LabelWidth = dc->GetTextExtent(prop.Name).cx;
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	m_Properties.AddItem(prop);

	MakeSortArrayDirty();

	if (!IsWindow(m_TooltipCtrl))
		m_TooltipCtrl.Create(this);
}

void CInspectorGrid::AddAttributes(LFVariantData* pData)
{
	
/*	for (UINT a=0; a<AttrCount; a++)
	{
		if (a>=LFAttributeCount)
		{
			pAttributes[a] = new CMFCPropertyGridProperty(AttributeVirtualNames[a-LFAttributeCount], _T(""));
			pAttributes[a]->Enable(FALSE);
			pAttributes[a]->AllowEdit(FALSE);
			AttributeCategory[a] = LFAttrCategoryInternal;
		}
		else
		{
			switch (theApp.m_Attributes[a]->Type)
			{
			case LFTypeUnicodeArray:
				pAttributes[a] = new CAttributePropertyTags(&AttributeValues[a]);
				break;
			case LFTypeAnsiString:
				pAttributes[a] = (a==LFAttrLocationIATA) ? new CAttributePropertyIATA(&AttributeValues[a], (CAttributeProperty**)&pAttributes[LFAttrLocationName], (CAttributeProperty**)&pAttributes[LFAttrLocationGPS]) : new CAttributeProperty(&AttributeValues[a]);
				break;
			case LFTypeRating:
				pAttributes[a] = new CAttributePropertyRating(&AttributeValues[a]);
				break;
			case LFTypeGeoCoordinates:
				pAttributes[a] = new CAttributePropertyGPS(&AttributeValues[a]);
				break;
			case LFTypeTime:
				pAttributes[a] = new CAttributePropertyTime(&AttributeValues[a]);
				break;
			default:
				pAttributes[a] = new CAttributeProperty(&AttributeValues[a]);
			}
			AttributeCategory[a] = theApp.m_Attributes[a]->Category;
		}
	}*/


	for (UINT a=0; a<LFAttributeCount; a++)
	{
		LFAttributeDescriptor* attr = p_App->m_Attributes[a];
		CInspectorProperty* pProp;

		switch (attr->Type)
		{
		case LFTypeRating:
			pProp = new CInspectorPropertyRating(&pData[a]);
			break;
		default:
			pProp = new CInspectorProperty(&pData[a]);
		}

		AddProperty(pProp, attr->Category, attr->Name, !attr->ReadOnly);
	}
}

void CInspectorGrid::ShowHeader(BOOL ShowHeader)
{
	m_ShowHeader = ShowHeader;
	AdjustLayout();
}

void CInspectorGrid::SetAlphabeticMode(BOOL SortAlphabetic)
{
	m_SortAlphabetic = SortAlphabetic;
	MakeSortArrayDirty();
	AdjustLayout();
}

void CInspectorGrid::UpdatePropertyState(UINT nID, BOOL Multiple, BOOL Editable, BOOL Visible)
{
	ASSERT(nID<m_Properties.m_ItemCount);

	m_Properties.m_Items[nID].Editable = Editable;
	m_Properties.m_Items[nID].Visible = Visible;
	m_Properties.m_Items[nID].pProperty->SetMultiple(Multiple);
}

void CInspectorGrid::ResetScrollbars()
{
	ScrollWindowEx(0, m_VScrollPos, NULL, NULL, NULL, NULL, SW_INVALIDATE);
	m_VScrollPos = 0;
	SetScrollPos(SB_VERT, m_VScrollPos, TRUE);
}

void CInspectorGrid::AdjustScrollbars()
{
	CRect rect;
	GetClientRect(rect);

	INT oldVScrollPos = m_VScrollPos;
	m_VScrollMax = max(0, m_ScrollHeight-rect.Height());
	m_VScrollPos = min(m_VScrollPos, m_VScrollMax);

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rect.Height();
	si.nMin = 0;
	si.nMax = m_ScrollHeight-1;
	si.nPos = m_VScrollPos;
	SetScrollInfo(SB_VERT, &si);

	if (oldVScrollPos!=m_VScrollPos)
		Invalidate();
}

INT CInspectorGrid::Compare(INT eins, INT zwei)
{
	Property* pEins = &m_Properties.m_Items[m_pSortArray[eins]];
	Property* pZwei = &m_Properties.m_Items[m_pSortArray[zwei]];

	if (m_SortAlphabetic)
		return wcscmp(pEins->Name, pZwei->Name);

	return (pEins->Category==pZwei->Category) ? m_pSortArray[eins]-m_pSortArray[zwei] : (INT)pEins->Category-(INT)pZwei->Category;
}

void CInspectorGrid::Heap(INT wurzel, INT anz)
{
	while (wurzel<=anz/2-1)
	{
		INT idx = (wurzel+1)*2-1;
		if (idx+1<anz)
			if (Compare(idx, idx+1)<0)
				idx++;

		if (Compare(wurzel, idx)<0)
		{
			std::swap(m_pSortArray[wurzel], m_pSortArray[idx]);
			wurzel = idx;
		}
		else
		{
			break;
		}
	}
}

inline void CInspectorGrid::CreateSortArray()
{
	if (m_pSortArray)
		return;

	ASSERT(m_Properties.m_ItemCount);
	m_pSortArray = new INT[m_Properties.m_ItemCount];

	for (UINT a=0; a<m_Properties.m_ItemCount; a++)
		m_pSortArray[a] = a;

	if (m_Properties.m_ItemCount>1)
	{
		for (INT a=m_Properties.m_ItemCount/2-1; a>=0; a--)
			Heap(a, m_Properties.m_ItemCount);
		for (INT a=m_Properties.m_ItemCount-1; a>0; )
		{
			std::swap(m_pSortArray[0], m_pSortArray[a]);
			Heap(0, a--);
		}
	}
}

inline void CInspectorGrid::MakeSortArrayDirty()
{
	if (m_pSortArray)
	{
		delete[] m_pSortArray;
		m_pSortArray = NULL;
	}
}

void CInspectorGrid::AdjustLayout()
{
	if (!m_Properties.m_ItemCount)
		return;

	CreateSortArray();
	ASSERT(m_pSortArray);

	for (UINT a=0; a<LFAttrCategoryCount; a++)
		m_Categories[a].Top = m_Categories[a].Bottom = -1;

	m_LabelWidth = 0;
	m_ScrollHeight = (m_pHeader && m_ShowHeader) ? m_pHeader->GetPreferredHeight()+1 : 1;
	INT Category = -1;

	for (UINT a=0; a<m_Properties.m_ItemCount; a++)
	{
		Property* pProp = &m_Properties.m_Items[m_pSortArray[a]];

		if ((!m_SortAlphabetic) && (pProp->Visible) && ((INT)pProp->Category!=Category))
		{
			Category = pProp->Category;
			INT Spacer = (m_ScrollHeight==1) ? -PADDING : 8;
			m_Categories[Category].Top = m_ScrollHeight;
			m_Categories[Category].Bottom = m_ScrollHeight+m_FontHeight[1]+2*PADDING+Spacer;
			m_ScrollHeight += m_FontHeight[1]+2*PADDING+Spacer+1;
		}

		pProp->Top = pProp->Visible ? m_ScrollHeight : -1;
		pProp->Bottom = pProp->Visible ? m_ScrollHeight+m_RowHeight : -1;

		if (pProp->Visible)
		{
			if (pProp->LabelWidth>m_LabelWidth)
				m_LabelWidth = pProp->LabelWidth;

			m_ScrollHeight += m_RowHeight+1;
		}
	}

	m_LabelWidth += GUTTER;

	CRect rect;
	GetClientRect(rect);
	if (m_LabelWidth>rect.Width()/2)
		m_LabelWidth = rect.Width()/2;

	AdjustScrollbars();
	Invalidate();
}

void CInspectorGrid::DrawCategory(CDC& dc, CRect& rect, WCHAR* Text)
{
	rect.DeflateRect(0, PADDING);
	rect.left += GUTTER;

	dc.DrawText(Text, rect, DT_LEFT | DT_BOTTOM | DT_SINGLELINE | DT_END_ELLIPSIS);

	CRect rectLine(rect);
	dc.DrawText(Text, rectLine, DT_LEFT | DT_BOTTOM | DT_SINGLELINE | DT_END_ELLIPSIS | DT_CALCRECT);
	rectLine.right += 2*PADDING;

	if (rectLine.right<=rect.right)
	{
		CPen pen(PS_SOLID, 1, 0xE2E2E2);

		CPen* pOldPen = dc.SelectObject(&pen);
		dc.MoveTo(rectLine.right, rect.bottom-(m_FontHeight[1]+1)/2);
		dc.LineTo(rect.right-PADDING, rect.bottom-(m_FontHeight[1]+1)/2);
		dc.SelectObject(pOldPen);
	}
}


BEGIN_MESSAGE_MAP(CInspectorGrid, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_THEMECHANGED()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

INT CInspectorGrid::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	Init();

	return 0;
}

void CInspectorGrid::OnDestroy()
{
	CWnd::OnDestroy();

	if (hThemeButton)
		p_App->zCloseThemeData(hThemeButton);
	if (hThemeList)
		p_App->zCloseThemeData(hThemeList);

	for (UINT a=0; a<m_Properties.m_ItemCount; a++)
		delete m_Properties.m_Items[a].pProperty;
}

LRESULT CInspectorGrid::OnThemeChanged()
{
	if (p_App->m_ThemeLibLoaded)
	{
		if (hThemeButton)
			p_App->zCloseThemeData(hThemeButton);
		if (hThemeList)
			p_App->zCloseThemeData(hThemeList);

		hThemeButton = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
		if (p_App->OSVersion>=OS_Vista)
			hThemeList = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
	}

	return TRUE;
}

BOOL CInspectorGrid::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CInspectorGrid::OnPaint()
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

	// Categories
	CFont* pOldFont = dc.SelectObject(&p_App->m_DefaultFont);
	dc.SetTextColor(Themed ? 0x993300 : GetSysColor(COLOR_WINDOWTEXT));

	for (UINT a=0; a<LFAttrCategoryCount; a++)
		if (m_Categories[a].Top!=-1)
		{
			CRect rect(0, m_Categories[a].Top-m_VScrollPos, rect.Width(), m_Categories[a].Bottom-m_VScrollPos);
			DrawCategory(dc, rect, p_App->m_AttrCategories[a]);
		}

	// Items
	dc.SelectStockObject(DEFAULT_GUI_FONT);

	for (UINT a=0; a<m_Properties.m_ItemCount; a++)
	{
		Property* pProp = &m_Properties.m_Items[a];
		if (pProp->Visible)
		{
			CRect rectProp(GUTTER, pProp->Top-m_VScrollPos, m_LabelWidth, pProp->Bottom-m_VScrollPos);
			dc.SetTextColor(pProp->Editable ? Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT) : GetSysColor(COLOR_3DSHADOW));
			dc.DrawText(pProp->Name, -1, rectProp, DT_RIGHT | DT_VCENTER | DT_END_ELLIPSIS | DT_SINGLELINE);

			rectProp.left = rectProp.right+GUTTER;
			rectProp.right = rect.Width()-GUTTER;
			dc.SetTextColor(Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));
			pProp->pProperty->DrawValue(dc, rectProp);
		}
	}

	//Header
	if (m_pHeader && m_ShowHeader)
	{
		CRect rect(0, -m_VScrollPos, rect.Width(), m_pHeader->GetPreferredHeight()-m_VScrollPos);
		m_pHeader->DrawHeader(dc, rect, Themed);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}

void CInspectorGrid::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CInspectorGrid::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CRect rect;
	GetClientRect(&rect);

	SCROLLINFO si;

	INT nInc = 0;
	switch (nSBCode)
	{
	case SB_TOP:
		nInc = -m_VScrollPos;
		break;
	case SB_BOTTOM:
		nInc = m_VScrollMax-m_VScrollPos;
		break;
	case SB_LINEUP:
		nInc = -m_RowHeight-1;
		break;
	case SB_LINEDOWN:
		nInc = m_RowHeight+1;
		break;
	case SB_PAGEUP:
		nInc = min(-1, -rect.Height());
		break;
	case SB_PAGEDOWN:
		nInc = max(1, rect.Height());
		break;
	case SB_THUMBTRACK:
		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(SB_VERT, &si);

		nInc = si.nTrackPos-m_VScrollPos;
		break;
	}

	nInc = max(-m_VScrollPos, min(nInc, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindowEx(0, -nInc, NULL, NULL, NULL, NULL, SW_INVALIDATE);

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = m_VScrollPos;
		SetScrollInfo(SB_VERT, &si);
	}

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL CInspectorGrid::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(&rect);
	if (!rect.PtInRect(pt))
		return FALSE;

	INT nInc = max(-m_VScrollPos, min(-zDelta*(INT)(m_RowHeight+1)/WHEEL_DELTA, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_TooltipCtrl.Deactivate();

		m_VScrollPos += nInc;
		ScrollWindowEx(0, -nInc, NULL, NULL, NULL, NULL, SW_INVALIDATE);
		SetScrollPos(SB_VERT, m_VScrollPos, TRUE);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}

	return TRUE;
}
