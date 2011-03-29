
// CInspectorGrid.cpp: Implementierung der Klasse CInspectorGrid
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "resource.h"


// CInspectorProperty
//

CInspectorProperty::CInspectorProperty(LFVariantData* pData)
{
	ASSERT(pData);

	p_Data = pData;
	p_Parent = NULL;
	m_Modified = m_Multiple = FALSE;
}

void CInspectorProperty::ToString(WCHAR* tmpStr, INT nCount)
{
	ASSERT(p_Parent);

	if ((m_Multiple) || (p_Data->Attr>=LFAttributeCount))
	{
		wcscpy_s(tmpStr, nCount, m_Multiple ? p_Parent->m_MultipleValues : p_Data->UnicodeString);
	}
	else
	{
		LFVariantDataToString(p_Data, tmpStr, nCount);
	}
}

void CInspectorProperty::DrawValue(CDC& dc, CRect rect)
{
	ASSERT(p_Parent);

	WCHAR tmpStr[256];
	ToString(tmpStr, 256);

	CFont* pOldFont = m_Multiple ? dc.SelectObject(&p_Parent->m_ItalicFont) : m_Modified ? dc.SelectObject(&p_Parent->m_BoldFont) : NULL;
	dc.DrawText(tmpStr, -1, rect, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS | DT_SINGLELINE);
	if (pOldFont)
		dc.SelectObject(pOldFont);
}

HCURSOR CInspectorProperty::SetCursor(INT /*x*/)
{
	ASSERT(p_Parent);

	return p_Parent->p_App->LoadStandardCursor(IDC_IBEAM);
}

CString CInspectorProperty::GetValidChars()
{
	return _T("");
}

void CInspectorProperty::OnSetString(CString Value)
{
	Value.Trim();
	if (p_Data->Attr==LFAttrLanguage)
		Value.MakeUpper();

	WCHAR tmpStr[256];
	ToString(tmpStr, 256);

	if (((p_Data->Attr!=LFAttrFileName) || (!Value.IsEmpty())) && (wcscmp(tmpStr, Value.GetBuffer())!=0))
	{
		p_Data->IsNull = false;

		switch (p_Data->Type)
		{
		case LFTypeUnicodeString:
		case LFTypeUnicodeArray:
			wcscpy_s(p_Data->UnicodeString, 256, Value);
			break;
		case LFTypeAnsiString:
			WideCharToMultiByte(CP_ACP, 0, Value, -1, p_Data->AnsiString, 256, NULL, NULL);
			break;
		}

		p_Parent->NotifyOwner((SHORT)p_Data->Attr);
	}
}

BOOL CInspectorProperty::OnClickValue(INT /*x*/)
{
	return TRUE;
}

void CInspectorProperty::OnClickButton()
{
}

BOOL CInspectorProperty::CanDelete()
{
	return (p_Data->Attr!=LFAttrFileName) && (!LFIsNullVariantData(p_Data));
}

BOOL CInspectorProperty::HasButton()
{
	return FALSE;
}

void CInspectorProperty::SetParent(CInspectorGrid* pParent)
{
	p_Parent = pParent;
}

void CInspectorProperty::SetMultiple(BOOL Multiple)
{
	m_Multiple = Multiple;
}

void CInspectorProperty::ResetModified()
{
	m_Modified = FALSE;
}

LFVariantData* CInspectorProperty::GetData()
{
	return p_Data;
}


// CInspectorPropertyTags
//

CInspectorPropertyTags::CInspectorPropertyTags(LFVariantData* pData)
	: CInspectorProperty(pData)
{
}

BOOL CInspectorPropertyTags::HasButton()
{
	return TRUE;
}

void CInspectorPropertyTags::OnClickButton()
{
	ASSERT(p_Parent);

	LFEditTagsDlg dlg(p_Parent, m_Multiple ? _T("") : p_Data->UnicodeArray, p_Parent->m_StoreID);

	if (dlg.DoModal()==IDOK)
	{
		p_Data->IsNull = false;
		wcscpy_s(p_Data->UnicodeArray, 256, dlg.m_Tags);
		p_Parent->NotifyOwner((SHORT)p_Data->Attr);
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
	ASSERT(p_Parent);

	HDC hdcMem = CreateCompatibleDC(dc);
	UCHAR level = m_Multiple ? 0 : p_Data->Rating;
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, p_Data->Attr==LFAttrRating ? p_Parent->p_App->m_RatingBitmaps[level] : p_Parent->p_App->m_PriorityBitmaps[level]);

	INT w = min(rect.Width()-6, RatingBitmapWidth);
	INT h = min(rect.Height(), RatingBitmapHeight);
	BLENDFUNCTION BF = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };
	AlphaBlend(dc, rect.left+6, rect.top+(rect.Height()-h)/2, w, h, hdcMem, 0, 0, w, h, BF);

	SelectObject(hdcMem, hbmOld);
	DeleteDC(hdcMem);
}

HCURSOR CInspectorPropertyRating::SetCursor(INT x)
{
	ASSERT(p_Parent);

	return p_Parent->p_App->LoadStandardCursor(x<6 ? IDC_HAND : ((x<RatingBitmapWidth+6) && ((x-6)%18<16)) ? IDC_HAND : IDC_ARROW);
}

BOOL CInspectorPropertyRating::CanDelete()
{
	return FALSE;
}

BOOL CInspectorPropertyRating::OnClickValue(INT x)
{
	ASSERT(p_Parent);

	if ((x>=0) && (x<RatingBitmapWidth+6))
		if ((x<6) || ((x-6)%18<16))
		{
			INT Rating = (x<6) ? 0 : 2*((x-6)/18)+((x-6)%18>8)+1;

			if (p_Data->Rating!=(UCHAR)Rating)
			{
				p_Data->IsNull = false;
				p_Data->Rating = (UCHAR)Rating;
				p_Parent->NotifyOwner((SHORT)p_Data->Attr);
			}
		}

	return FALSE;
}


// CInspectorPropertyIATA
//

CInspectorPropertyIATA::CInspectorPropertyIATA(LFVariantData* pData, LFVariantData* pLocationName, LFVariantData* pLocationGPS)
	: CInspectorProperty(pData)
{
	p_LocationName = pLocationName;
	p_LocationGPS = pLocationGPS;
}

CString CInspectorPropertyIATA::GetValidChars()
{
	return _T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
}

BOOL CInspectorPropertyIATA::HasButton()
{
	return TRUE;
}

void CInspectorPropertyIATA::OnSetString(CString Value)
{
	Value.MakeUpper();

	CInspectorProperty::OnSetString(Value);
}

void CInspectorPropertyIATA::OnClickButton()
{
	ASSERT(p_Parent);

	LFSelectLocationIATADlg dlg(IDD_SELECTIATA, p_Parent, &p_Data->AnsiString[0]);

	if (dlg.DoModal()==IDOK)
		if (dlg.m_Airport)
		{
			SHORT Attr2 = -1;
			if ((p_LocationName) && (dlg.m_IATA_OverwriteName))
			{
				ASSERT(p_LocationName->Attr==LFAttrLocationName);
				Attr2 = LFAttrLocationName;

				p_LocationName->IsNull = false;
				MultiByteToWideChar(CP_ACP, 0, dlg.m_Airport->Name, -1, p_LocationName->UnicodeString, 256);
			}

			SHORT Attr3 = -1;
			if ((p_LocationGPS) && (dlg.m_IATA_OverwriteGPS))
			{
				ASSERT(p_LocationGPS->Attr==LFAttrLocationGPS);
				Attr3 = LFAttrLocationGPS;

				p_LocationGPS->IsNull = false;
				p_LocationGPS->GeoCoordinates = dlg.m_Airport->Location;
			}

			p_Data->IsNull = false;
			strcpy_s(p_Data->AnsiString, 256, dlg.m_Airport->Code);
			p_Parent->NotifyOwner((SHORT)p_Data->Attr, Attr2, Attr3);
		}
}


// CInspectorPropertyGPS
//

CInspectorPropertyGPS::CInspectorPropertyGPS(LFVariantData* pData)
	: CInspectorProperty(pData)
{
}

HCURSOR CInspectorPropertyGPS::SetCursor(INT /*x*/)
{
	ASSERT(p_Parent);

	return p_Parent->p_App->LoadStandardCursor(IDC_ARROW);
}

BOOL CInspectorPropertyGPS::HasButton()
{
	return TRUE;
}

BOOL CInspectorPropertyGPS::OnClickValue(INT /*x*/)
{
	return FALSE;
}

void CInspectorPropertyGPS::OnClickButton()
{
	ASSERT(p_Parent);

	LFSelectLocationGPSDlg dlg(p_Parent, p_Data->GeoCoordinates);

	if (dlg.DoModal()==IDOK)
	{
		p_Data->GeoCoordinates = dlg.m_Location;
		p_Data->IsNull = false;
		p_Parent->NotifyOwner((SHORT)p_Data->Attr);
	}
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
extern INT GetAttributeIconIndex(UINT Attr);

#define GUTTER      3
#define PADDING     2

#define PartNone       0
#define PartLabel      1
#define PartValue      2
#define PartButton     3
#define PartReset      4

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
	m_ShowHeader = m_SortAlphabetic = m_Hover = m_PartPressed = FALSE;
	m_pSortArray = NULL;
	m_pHeader = NULL;
	hThemeList = hThemeButton = NULL;
	hIconResetNormal = hIconResetHot = NULL;
	m_VScrollMax = m_VScrollPos = m_IconSize = 0;
	m_HotItem = m_SelectedItem = m_EditItem = -1;
	m_HotPart = PartNone;
	p_Edit = NULL;

	ENSURE(m_MultipleValues.LoadString(IDS_MULTIPLEVALUES));
}

CInspectorGrid::~CInspectorGrid()
{
	DestroyEdit();

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

	m_AttributeIcons.Create(IDB_ATTRIBUTEICONS_32, LFCommDlgDLL.hResource, 0, -1, 32, 32);
	m_TooltipCtrl.Create(this);

	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&p_App->m_DefaultFont);
	m_FontHeight[1] = dc->GetTextExtent(_T("Wy")).cy;
	dc->SelectStockObject(DEFAULT_GUI_FONT);
	m_FontHeight[0] = dc->GetTextExtent(_T("Wy")).cy;
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	m_RowHeight = max(m_FontHeight[0], 16);
	m_IconSize = (m_RowHeight>=32) ? 32 : (m_RowHeight>=24) ? 24 : 16;
	hIconResetNormal = (HICON)LoadImage(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDI_RESET_NORMAL), IMAGE_ICON, m_IconSize, m_IconSize, LR_DEFAULTCOLOR);
	hIconResetHot = (HICON)LoadImage(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDI_RESET_HOT), IMAGE_ICON, m_IconSize, m_IconSize, LR_DEFAULTCOLOR);
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

BOOL CInspectorGrid::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (p_Edit)
			switch (pMsg->wParam)
			{
			case VK_EXECUTE:
			case VK_RETURN:
				DestroyEdit(TRUE);
				return TRUE;
			case VK_ESCAPE:
				DestroyEdit(FALSE);
				return TRUE;
			}
		break;
	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
		if (p_Edit)
			return TRUE;
		break;
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
		m_TooltipCtrl.Deactivate();
		break;
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void CInspectorGrid::AddProperty(CInspectorProperty* pProperty, UINT Category, WCHAR* Name, BOOL Editable)
{
	pProperty->SetParent(this);

	Property prop;
	prop.pProperty = pProperty;
	prop.Category = Category;
	wcscpy_s(prop.Name, 256, Name);
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

	DestroyEdit();
}

void CInspectorGrid::AddAttributes(LFVariantData* pData)
{
/*	case LFTypeTime:
		pAttributes[a] = new CAttributePropertyTime(&AttributeValues[a]);
		break;*/

	for (UINT a=0; a<LFAttributeCount; a++)
	{
		LFAttributeDescriptor* attr = p_App->m_Attributes[a];
		CInspectorProperty* pProp;

		switch (attr->Type)
		{
		case LFTypeUnicodeArray:
			pProp = (a==LFAttrTags) ? new CInspectorPropertyTags(&pData[a]) : new CInspectorProperty(&pData[a]);
			break;
		case LFTypeAnsiString:
			pProp = (a==LFAttrLocationIATA) ? new CInspectorPropertyIATA(&pData[a], &pData[LFAttrLocationName], &pData[LFAttrLocationGPS]) : new CInspectorProperty(&pData[a]);
			break;
		case LFTypeRating:
			pProp = new CInspectorPropertyRating(&pData[a]);
			break;
		case LFTypeGeoCoordinates:
			pProp = new CInspectorPropertyGPS(&pData[a]);
			break;
		default:
			pProp = new CInspectorProperty(&pData[a]);
		}

		AddProperty(pProp, attr->Category, attr->Name, !attr->ReadOnly);
	}
}

void CInspectorGrid::ShowHeader(BOOL ShowHeader)
{
	DestroyEdit();

	m_ShowHeader = ShowHeader;
	AdjustLayout();
}

void CInspectorGrid::SetAlphabeticMode(BOOL SortAlphabetic)
{
	DestroyEdit();

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
	m_Properties.m_Items[nID].pProperty->ResetModified();
}

void CInspectorGrid::SetStore(CHAR* StoreID)
{
	strcpy_s(m_StoreID, LFKeySize, StoreID ? StoreID : "");
}

CString CInspectorGrid::GetName(UINT nID)
{
	ASSERT(nID<m_Properties.m_ItemCount);

	return m_Properties.m_Items[nID].Name;
}

CString CInspectorGrid::GetValue(UINT nID)
{
	ASSERT(nID<m_Properties.m_ItemCount);

	WCHAR tmpStr[256];
	m_Properties.m_Items[nID].pProperty->ToString(tmpStr, 256);

	return tmpStr;
}

RECT CInspectorGrid::GetItemRect(INT Item)
{
	RECT rect = { 0, 0, 0, 0 };

	if ((Item>=0) && (Item<(INT)m_Properties.m_ItemCount))
		{
			GetClientRect(&rect);
			rect.top = m_Properties.m_Items[Item].Top-1;
			rect.bottom = m_Properties.m_Items[Item].Bottom+1;
			rect.left++;
			rect.right--;
			OffsetRect(&rect, 0, -m_VScrollPos);
		}

	return rect;
}

INT CInspectorGrid::HitTest(CPoint point, UINT* PartID)
{
	for (UINT a=0; a<m_Properties.m_ItemCount; a++)
	{
		RECT rect = GetItemRect(a);
		if (PtInRect(&rect, point))
		{
			if (PartID)
			{
				if (point.x<m_LabelWidth+GUTTER)
				{
					*PartID = PartLabel;
					return a;
				}

				Property* pProp = &m_Properties.m_Items[a];
				CRect rectPart(m_LabelWidth+GUTTER, pProp->Top-m_VScrollPos, rect.right+1, pProp->Bottom-m_VScrollPos);

				if ((pProp->Editable) && (pProp->pProperty->CanDelete()) && ((INT)a!=m_EditItem))
				{
					INT Offs = (rectPart.Height()-m_IconSize)/2;
					CRect rectReset(rectPart.right-m_IconSize-Offs-2, rectPart.top+Offs, rectPart.right-Offs-2, rectPart.top+Offs+m_IconSize);
					if (rectReset.PtInRect(point))
					{
						*PartID = PartReset;
						return a;
					}

					rectPart.right -= m_IconSize+Offs+GUTTER+2;
				}
				else
				{
					rectPart.right -= GUTTER-1;
				}

				if ((pProp->Editable) && (pProp->pProperty->HasButton()) && ((INT)a!=m_EditItem))
				{
					CRect rectButton(rectPart);
					rectButton.left = rectButton.right-rectButton.Height()-m_IconSize/2;

					if (rectButton.PtInRect(point))
					{
						*PartID = PartButton;
						return a;
					}
				}

				*PartID = PartValue;
			}

			return a;
		}
	}

	if (PartID)
		*PartID = PartNone;
	return -1;
}

void CInspectorGrid::InvalidateItem(INT Item)
{
	RECT rect = GetItemRect(Item);
	InvalidateRect(&rect);
}

void CInspectorGrid::EnsureVisible(INT Item)
{
	if (Item==-1)
		return;

	CRect rect;
	GetClientRect(&rect);

	RECT rectItem = GetItemRect(Item);

	SCROLLINFO si;
	INT nInc;

	// Vertikal
	nInc = 0;
	if (rectItem.bottom>rect.Height())
		nInc = rectItem.bottom-rect.Height();
	if (rectItem.top<nInc)
		nInc = rectItem.top;

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
}

void CInspectorGrid::SelectItem(INT Item)
{
	if ((Item==m_SelectedItem) || (Item==-1))
		return;

	if (!m_Properties.m_Items[Item].Visible)
		return;

	InvalidateItem(m_SelectedItem);
	m_SelectedItem = Item;
	EnsureVisible(Item);
	InvalidateItem(Item);

	ReleaseCapture();
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

void CInspectorGrid::NotifyOwner(SHORT Attr1, SHORT Attr2, SHORT Attr3)
{
	if (Attr1!=-1)
	{
		m_Properties.m_Items[Attr1].pProperty->m_Modified = TRUE;
		m_Properties.m_Items[Attr1].pProperty->m_Multiple = FALSE;
		InvalidateItem(Attr1);
	}
	if (Attr2!=-1)
	{
		m_Properties.m_Items[Attr2].pProperty->m_Modified = TRUE;
		m_Properties.m_Items[Attr2].pProperty->m_Multiple = FALSE;
		InvalidateItem(Attr2);
	}
	if (Attr3!=-1)
	{
		m_Properties.m_Items[Attr3].pProperty->m_Modified = TRUE;
		m_Properties.m_Items[Attr3].pProperty->m_Multiple = FALSE;
		InvalidateItem(Attr3);
	}

	m_TooltipCtrl.Deactivate();

	UpdateWindow();
	GetOwner()->PostMessage(WM_PROPERTYCHANGED, Attr1, Attr2 | (Attr3 << 16));
}

void CInspectorGrid::ResetProperty(UINT Attr)
{
	ASSERT(Attr<m_Properties.m_ItemCount);
	ASSERT(m_Properties.m_Items[Attr].Editable);

	LFGetNullVariantData(m_Properties.m_Items[Attr].pProperty->GetData());
	m_Properties.m_Items[Attr].pProperty->m_Modified = TRUE;

	NotifyOwner((SHORT)Attr);
}

void CInspectorGrid::EditProperty(UINT Attr)
{
	ASSERT(Attr<m_Properties.m_ItemCount);
	m_EditItem = -1;

	Property* pProp = &m_Properties.m_Items[Attr];
	if (pProp->Editable)
		if (pProp->pProperty->OnClickValue(-1))
		{
			m_EditItem = Attr;
			InvalidateItem(Attr);
			EnsureVisible(Attr);

			RECT rect = GetItemRect(Attr);
			rect.top += 2;
			rect.bottom -=2;
			rect.left += m_LabelWidth+GUTTER-1;

			p_Edit = new CMFCMaskedEdit();
			p_Edit->Create(WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ES_AUTOHSCROLL, rect, this, 1);
			if (!pProp->pProperty->m_Multiple)
			{
				WCHAR tmpStr[256];
				pProp->pProperty->ToString(tmpStr, 256);
				p_Edit->SetWindowText(tmpStr);
				p_Edit->SetSel(0, (INT)wcslen(tmpStr));
			}
			p_Edit->SetValidChars(pProp->pProperty->GetValidChars());
			if (Attr<LFAttributeCount)
				p_Edit->SetLimitText(p_App->m_Attributes[Attr]->cCharacters);
			if (pProp->pProperty->m_Modified)
			{
				p_Edit->SetFont(&m_BoldFont);
			}
			else
			{
				p_Edit->SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));
			}
			p_Edit->SetFocus();
		}
}

void CInspectorGrid::DestroyEdit(BOOL Accept)
{
	if (p_Edit)
	{
		INT Item = m_EditItem;

		CEdit* victim = p_Edit;
		p_Edit = NULL;

		CString Value;
		victim->GetWindowText(Value);
		victim->DestroyWindow();
		delete victim;

		if ((Accept) && (Item!=-1))
			m_Properties.m_Items[Item].pProperty->OnSetString(Value);
	}

	m_EditItem = -1;
}


BEGIN_MESSAGE_MAP(CInspectorGrid, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_THEMECHANGED()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SETCURSOR()
	ON_EN_KILLFOCUS(1, OnDestroyEdit)
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
	if (hIconResetNormal)
		DestroyIcon(hIconResetNormal);
	if (hIconResetHot)
		DestroyIcon(hIconResetHot);

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
			RECT rectProp = GetItemRect(a);

			COLORREF clr1 = pProp->Editable ? Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT) : GetSysColor(COLOR_3DSHADOW);
			COLORREF clr2 = Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT);
			if ((INT)a!=m_EditItem)
				if (hThemeList)
				{
					const INT StateIDs[4] = { LISS_NORMAL, LISS_HOT, GetFocus()!=this ? LISS_SELECTEDNOTFOCUS : LISS_SELECTED, LISS_HOTSELECTED };
					UINT State = 0;
					if ((INT)a==m_HotItem)
						State |= 1;
					if ((GetFocus()==this) && ((INT)a==m_SelectedItem))
						State |= 2;
					if (State)
						p_App->zDrawThemeBackground(hThemeList, dc, LVP_LISTITEM, StateIDs[State], &rectProp, &rectProp);
				}
				else
				{
					if ((GetFocus()==this) && ((INT)a==m_SelectedItem))
					{
						dc.FillSolidRect(&rectProp, GetSysColor(COLOR_HIGHLIGHT));
						dc.SetTextColor(0x000000);
						dc.SetBkColor(0xFFFFFF);
						dc.DrawFocusRect(&rectProp);

						clr1 = clr2 = GetSysColor(COLOR_HIGHLIGHTTEXT);
					}
				}

			CRect rectLabel(GUTTER, pProp->Top-m_VScrollPos, m_LabelWidth, pProp->Bottom-m_VScrollPos);
			dc.SetTextColor(clr1);
			dc.DrawText(CString(pProp->Name)+_T(":"), rectLabel, DT_RIGHT | DT_VCENTER | DT_END_ELLIPSIS | DT_SINGLELINE);

			rectLabel.left = rectLabel.right+GUTTER;
			rectLabel.right = rect.Width();

			if ((pProp->Editable) && (pProp->pProperty->CanDelete()) && ((INT)a==m_HotItem) && ((INT)a!=m_EditItem))
			{
				INT Offs = (rectLabel.Height()-m_IconSize)/2;
				DrawIconEx(dc, rectLabel.right-m_IconSize-Offs-2, rectLabel.top+Offs, ((INT)a==m_HotItem) && (m_HotPart==PartReset) ? hIconResetHot : hIconResetNormal, m_IconSize, m_IconSize, 0, NULL, DI_NORMAL);
				rectLabel.right -= m_IconSize+Offs+GUTTER+2;
			}
			else
			{
				rectLabel.right -= GUTTER-1;
			}

			if ((pProp->Editable) && (pProp->pProperty->HasButton()) && ((INT)a==m_HotItem) && ((INT)a!=m_EditItem))
			{
				CRect rectButton(rectLabel);
				rectButton.left = rectButton.right-rectButton.Height()-m_IconSize/2;
				rectLabel.right -= rectButton.Width()+GUTTER;

				BOOL Hot = ((INT)a==m_HotItem) && (m_HotPart==PartButton);
				BOOL Pressed = Hot && m_PartPressed;

				if (hThemeButton)
				{
					rectButton.InflateRect(1, 1);

					UINT State = Pressed ? PBS_PRESSED : Hot ? PBS_HOT : PBS_NORMAL;
					p_App->zDrawThemeBackground(hThemeButton, dc, BP_PUSHBUTTON, State, &rectButton, NULL);
				}
				else
				{
					COLORREF c1 = GetSysColor(COLOR_3DHIGHLIGHT);
					COLORREF c2 = GetSysColor(COLOR_3DFACE);
					COLORREF c3 = GetSysColor(COLOR_3DSHADOW);
					COLORREF c4 = 0x000000;

					dc.FillSolidRect(rectButton, c2);

					if (Pressed)
					{
						std::swap(c1, c4);
						std::swap(c2, c3);
					}

					CRect rectBorder(rectButton);
					dc.Draw3dRect(rectBorder, c1, c4);
					rectBorder.DeflateRect(1, 1);
					dc.Draw3dRect(rectBorder, c2, c3);
				}

				rectButton.bottom -= 4;
				if (Pressed)
					rectButton.OffsetRect(1, 1);

				dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
				dc.DrawText(_T("..."), rectButton, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
			}

			dc.SetTextColor(clr2);
			pProp->pProperty->DrawValue(dc, rectLabel);
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

void CInspectorGrid::OnMouseMove(UINT nFlags, CPoint point)
{
	BOOL Dragging = (GetCapture()==this);
	UINT Part;
	INT Item = HitTest(point, &Part);

	if (!m_Hover)
	{
		m_Hover = TRUE;

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = LFHOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
	else
		if ((m_TooltipCtrl.IsWindowVisible()) && (Item!=m_HotItem))
			m_TooltipCtrl.Deactivate();

	if (!Dragging)
	{
		if ((m_HotItem!=Item) || (m_HotPart!=Part))
		{
			InvalidateItem(m_HotItem);
			m_HotItem = Item;
			m_HotPart = Part;
			InvalidateItem(m_HotItem);
		}

		if (nFlags & MK_RBUTTON)
			SetFocus();
	}
	else
	{
		BOOL PartPressed = (Item==m_SelectedItem) && (Part==m_HotPart);
		if (m_PartPressed!=PartPressed)
		{
			m_PartPressed = PartPressed;
			InvalidateItem(m_SelectedItem);
		}
	}
}

void CInspectorGrid::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();
	InvalidateItem(m_HotItem);

	m_Hover = FALSE;
	m_HotItem = -1;
	m_HotPart = PartNone;
}

void CInspectorGrid::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if ((m_HotItem!=-1) && (!p_Edit))
			if (!m_TooltipCtrl.IsWindowVisible())
			{
				Property* pProp = &m_Properties.m_Items[m_HotItem];
				ASSERT(pProp);

				INT idx = GetAttributeIconIndex(m_HotItem);
				HICON hIcon = (idx!=-1) ? m_AttributeIcons.ExtractIcon(idx) : NULL;

				WCHAR tmpStr[256];
				pProp->pProperty->ToString(tmpStr, 256);

				ClientToScreen(&point);
				m_TooltipCtrl.Track(point, hIcon, hIcon ? CSize(32, 32) : CSize(0, 0), pProp->Name, tmpStr);
			}
	}
	else
	{
		m_TooltipCtrl.Deactivate();
	}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = LFHOVERTIME;
	tme.hwndTrack = m_hWnd;
	TrackMouseEvent(&tme);
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

void CInspectorGrid::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	SetFocus();

	UINT Part;
	INT Item = HitTest(point, & Part);
	if (Item!=-1)
	{
		SelectItem(Item);

		if (Part>=PartButton)
		{
			m_HotPart = Part;
			m_PartPressed = TRUE;
			SetCapture();
			InvalidateItem(m_SelectedItem);
		}
	}
}

void CInspectorGrid::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	UINT Part;
	INT Item = HitTest(point, &Part);

	if (GetCapture()==this)
	{
		m_PartPressed = FALSE;
		ReleaseCapture();
		InvalidateItem(Item);

		if ((Item==m_SelectedItem) && (Item!=-1))
			switch (Part)
			{
			case PartButton:
				m_Properties.m_Items[Item].pProperty->OnClickButton();
				break;
			case PartReset:
				ResetProperty(Item);
				break;
			}
	}
	else
		if ((Item==m_SelectedItem) && (Item!=-1) && (Part==PartValue))
		{
			Property* pProp = &m_Properties.m_Items[Item];

			if (pProp->Editable)
				if (pProp->pProperty->OnClickValue(point.x-m_LabelWidth-GUTTER))
					EditProperty(Item);
		}
}

void CInspectorGrid::OnLButtonDblClk(UINT /*nFlags*/, CPoint point)
{
	m_PartPressed = FALSE;
	ReleaseCapture();

	UINT Part;
	INT Item = HitTest(point, &Part);
	if ((Item!=-1) && (Part<=PartButton))
	{
		SelectItem(Item);
		EditProperty(Item);
	}
}

void CInspectorGrid::OnRButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	SetFocus();
}

void CInspectorGrid::OnSetFocus(CWnd* /*pOldWnd*/)
{
	Invalidate();
}

void CInspectorGrid::OnKillFocus(CWnd* /*pNewWnd*/)
{
	Invalidate();
}

BOOL CInspectorGrid::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	UINT Part;
	INT Item = HitTest(point, &Part);

	if ((Item!=-1) && (Part==PartValue))
	{
		Property* pProp = &m_Properties.m_Items[Item];

		if (pProp->Editable)
		{
			SetCursor(pProp->pProperty->SetCursor(point.x-m_LabelWidth-GUTTER));
			return TRUE;
		}
	}

	SetCursor(p_App->LoadStandardCursor(IDC_ARROW));
	return TRUE;
}

void CInspectorGrid::OnDestroyEdit()
{
	DestroyEdit(TRUE);
}
