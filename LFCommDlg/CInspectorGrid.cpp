
// CInspectorGrid.cpp: Implementierung der Klasse CInspectorGrid
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CInspectorProperty
//

CInspectorProperty::CInspectorProperty(LFVariantData* pData)
{
	p_Data = pData;
}

void CInspectorProperty::DrawValue(CDC& dc, CRect rect)
{
	WCHAR tmpStr[256];
	LFVariantDataToString(p_Data, tmpStr, 256);

	dc.DrawText(tmpStr, -1, rect, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
}


// CInspectorGrid
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

#define GUTTER     3

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
	hTheme = NULL;
}

BOOL CInspectorGrid::Create(CWnd* pParentWnd, UINT nID, BOOL bBorder)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | (bBorder ? WS_BORDER : 0);
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}

void CInspectorGrid::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	m_TooltipCtrl.Create(this);

	if (p_App->m_ThemeLibLoaded)
	{
		p_App->zSetWindowTheme(GetSafeHwnd(), NULL, NULL);
		hTheme = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
	}

//	ResetScrollbars();

	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&p_App->m_DefaultFont);
	m_FontHeight[1] = dc->GetTextExtent(_T("Wy")).cy;
	dc->SelectStockObject(DEFAULT_GUI_FONT);
	m_FontHeight[0] = dc->GetTextExtent(_T("Wy")).cy;
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	m_RowHeight = max(m_FontHeight[0], 16);
}

void CInspectorGrid::AddProperty(CInspectorProperty* pProperty, UINT Category, WCHAR* Name, BOOL Editable)
{
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
		default:
			pProp = new CInspectorProperty(&pData[a]);
		}

		AddProperty(pProp, attr->Category, attr->Name, !attr->ReadOnly);
	}
}

void CInspectorGrid::SetAlphabeticMode(BOOL SortAlphabetic)
{
	m_SortAlphabetic = SortAlphabetic;
	AdjustLayout();
}

void CInspectorGrid::UpdatePropertyState(UINT nID, BOOL Multiple, BOOL Editable, BOOL Visible)
{
	ASSERT(nID<m_Properties.m_ItemCount);

	m_Properties.m_Items[nID].Editable = Editable;
	m_Properties.m_Items[nID].Visible = Visible;
}

void CInspectorGrid::AdjustLayout()
{
	m_LabelWidth = 0;
	INT Top = 0;

	for (UINT a=0; a<m_Properties.m_ItemCount; a++)
	{
		Property* pProp = &m_Properties.m_Items[a];

		pProp->Top = pProp->Visible ? Top : -1;
		pProp->Bottom = pProp->Visible ? Top+m_RowHeight : -1;

		if (pProp->Visible)
		{
			if (pProp->LabelWidth>m_LabelWidth)
				m_LabelWidth = pProp->LabelWidth;

			Top += m_RowHeight+1;
		}
	}

	m_LabelWidth += GUTTER;

	CRect rect;
	GetClientRect(rect);
	if (m_LabelWidth>rect.Width()/2)
		m_LabelWidth = rect.Width()/2;

	Invalidate();
}


BEGIN_MESSAGE_MAP(CInspectorGrid, CWnd)
	ON_WM_DESTROY()
	ON_WM_THEMECHANGED()
	ON_WM_ERASEBKGND()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CInspectorGrid::OnDestroy()
{
	CWnd::OnDestroy();

	if (hTheme)
		p_App->zCloseThemeData(hTheme);

	for (UINT a=0; a<m_Properties.m_ItemCount; a++)
		delete m_Properties.m_Items[a].pProperty;
}

LRESULT CInspectorGrid::OnThemeChanged()
{
	if (p_App->m_ThemeLibLoaded)
	{
		if (hTheme)
			p_App->zCloseThemeData(hTheme);

		hTheme = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
	}

	return TRUE;
}

BOOL CInspectorGrid::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CInspectorGrid::OnNcPaint()
{
	if (GetStyle() & WS_BORDER)
		DrawControlBorder(this);
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

	dc.SelectStockObject(DEFAULT_GUI_FONT);

	for (UINT a=0; a<m_Properties.m_ItemCount; a++)
	{
		Property* pProp = &m_Properties.m_Items[a];
		if (pProp->Visible)
		{
			CRect rectProp(GUTTER, pProp->Top, m_LabelWidth, pProp->Bottom);
			dc.SetTextColor(pProp->Editable ? Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT) : GetSysColor(COLOR_3DSHADOW));
			dc.DrawText(pProp->Name, -1, rectProp, DT_RIGHT | DT_VCENTER | DT_END_ELLIPSIS);

			rectProp.left = rectProp.right+GUTTER;
			rectProp.right = rect.Width()-GUTTER;
			dc.SetTextColor(Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));
			pProp->pProperty->DrawValue(dc, rectProp);
		}
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CInspectorGrid::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}
