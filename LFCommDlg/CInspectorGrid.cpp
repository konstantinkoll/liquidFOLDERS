
// CInspectorGrid.cpp: Implementierung der Klasse CInspectorGrid
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFEditTagsDlg.h"
#include "LFEditTimeDlg.h"


// CPropertyHolder
//

CPropertyHolder::CPropertyHolder()
	: CWnd()
{
	m_StoreIDValid = FALSE;

	ENSURE(m_MultipleValues.LoadString(IDS_MULTIPLEVALUES));
}

void CPropertyHolder::SetStore(CHAR* StoreID)
{
	m_StoreIDValid = (StoreID!=NULL);
	strcpy_s(m_StoreID, LFKeySize, m_StoreIDValid ? StoreID : "");
}

void CPropertyHolder::CreateFonts()
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

CProperty* CPropertyHolder::CreateProperty(LFVariantData* pData)
{
	LFAttributeDescriptor* pAttr = &LFGetApp()->m_Attributes[pData->Attr];
	CProperty* pProperty;

	switch (pAttr->Type)
	{
	case LFTypeUnicodeArray:
		pProperty = (pData->Attr==LFAttrHashtags) ? new CPropertyTags(pData) : new CProperty(pData);
		break;
	case LFTypeAnsiString:
		pProperty = (pData->Attr==LFAttrLocationIATA) ? new CPropertyIATA(pData, NULL, NULL) : new CProperty(pData);
		break;
	case LFTypeRating:
		pProperty = new CPropertyRating(pData);
		break;
	case LFTypeUINT:
	case LFTypeSize:
		pProperty = new CPropertyNumber(pData);
		break;
	case LFTypeDuration:
		pProperty = new CPropertyDuration(pData);
		break;
	case LFTypeGeoCoordinates:
		pProperty = new CPropertyGPS(pData);
		break;
	case LFTypeTime:
		pProperty = new CPropertyTime(pData);
		break;
	default:
		pProperty = new CProperty(pData);
	}

	pProperty->SetParent(this);
	return pProperty;
}


BEGIN_MESSAGE_MAP(CPropertyHolder, CWnd)
	ON_WM_RBUTTONDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

void CPropertyHolder::OnRButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	SetFocus();
}

void CPropertyHolder::OnSetFocus(CWnd* /*pOldWnd*/)
{
	Invalidate();
}

void CPropertyHolder::OnKillFocus(CWnd* /*pNewWnd*/)
{
	Invalidate();
}


// CProperty
//

CProperty::CProperty(LFVariantData* pData)
{
	ASSERT(pData);

	p_Data = pData;
	p_Parent = NULL;
	m_Modified = m_Multiple = m_ShowRange = FALSE;
	ZeroMemory(&m_RangeFirst, sizeof(m_RangeFirst));
	ZeroMemory(&m_RangeSecond, sizeof(m_RangeSecond));
}

void CProperty::ToString(WCHAR* tmpStr, INT nCount)
{
	ASSERT(p_Parent);

	if (p_Data->Attr>=LFAttributeCount)
	{
		wcscpy_s(tmpStr, nCount, p_Data->UnicodeString);
	}
	else
		if (m_Multiple)
		{
			if (m_ShowRange)
			{
				WCHAR tmpBuf[256];
				LFVariantDataToString(m_RangeSecond, tmpBuf, 256);

				LFVariantDataToString(m_RangeFirst, tmpStr, nCount);
				wcscat_s(tmpStr, nCount, L" – ");
				wcscat_s(tmpStr, nCount, tmpBuf);
			}
			else
			{
				wcscpy_s(tmpStr, nCount, m_Multiple ? p_Parent->m_MultipleValues : p_Data->UnicodeString);
			}
		}
		else
		{
			LFVariantDataToString(*p_Data, tmpStr, nCount);
		}
}

void CProperty::DrawValue(CDC& dc, CRect rect)
{
	ASSERT(p_Parent);

	WCHAR tmpStr[256];
	ToString(tmpStr, 256);

	CFont* pOldFont = m_Multiple ? dc.SelectObject(&p_Parent->m_ItalicFont) : m_Modified ? dc.SelectObject(&p_Parent->m_BoldFont) : NULL;
	dc.DrawText(tmpStr, -1, rect, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS | DT_SINGLELINE | DT_NOPREFIX);
	if (pOldFont)
		dc.SelectObject(pOldFont);
}

HCURSOR CProperty::SetCursor(INT /*x*/)
{
	ASSERT(p_Parent);

	return LFGetApp()->LoadStandardCursor(IDC_IBEAM);
}

CString CProperty::GetValidChars()
{
	return _T("");
}

void CProperty::SetEditMask(CMFCMaskedEdit* /*pEdit*/)
{
}

void CProperty::OnSetString(CString Value)
{
	Value.Trim();
	if (p_Data->Attr==LFAttrLanguage)
		Value.MakeUpper();

	WCHAR tmpStr[256];
	ToString(tmpStr, 256);

	if (((p_Data->Attr!=LFAttrFileName) || (!Value.IsEmpty())) && (wcscmp(tmpStr, Value.GetBuffer())!=0))
	{
		p_Data->IsNull = FALSE;

		switch (p_Data->Type)
		{
		case LFTypeUnicodeString:
			wcsncpy_s(p_Data->UnicodeString, 256, Value, 255);
			break;
		case LFTypeUnicodeArray:
			wcsncpy_s(p_Data->UnicodeArray, 256, Value, 255);
			LFSanitizeUnicodeArray(p_Data->UnicodeArray, 256);
			break;
		case LFTypeAnsiString:
			WideCharToMultiByte(CP_ACP, 0, Value, -1, p_Data->AnsiString, 256, NULL, NULL);
			break;
		}

		p_Parent->NotifyOwner((SHORT)p_Data->Attr);
	}
}

BOOL CProperty::OnClickValue(INT /*x*/)
{
	return TRUE;
}

void CProperty::OnClickButton()
{
}

BOOL CProperty::CanDelete()
{
	return (p_Data->Attr!=LFAttrFileName) && ((!LFIsNullVariantData(*p_Data)) || m_Multiple);
}

BOOL CProperty::HasButton()
{
	return FALSE;
}

BOOL CProperty::WantsChars()
{
	return FALSE;
}

void CProperty::SetParent(CPropertyHolder* pParentWnd)
{
	p_Parent = pParentWnd;
}

void CProperty::SetMultiple(BOOL Multiple, LFVariantData* pRangeFirst, LFVariantData* pRangeSecond)
{
	m_Multiple = Multiple;

	m_ShowRange = Multiple && (pRangeFirst!=NULL) && (pRangeSecond!=NULL);

	if (m_ShowRange)
	{
		UINT Type = LFGetApp()->m_Attributes[p_Data->Attr].Type;

		m_ShowRange &= (Type!=LFTypeUnicodeString) && (Type!=LFTypeUnicodeArray) && (Type!=LFTypeAnsiString) && (Type!=LFTypeFourCC) && (Type!=LFTypeFraction) && (Type!=LFTypeFlags);
	}

	if (m_ShowRange)
	{
		m_RangeFirst = *pRangeFirst;
		m_RangeSecond = *pRangeSecond;
	}
	else
	{
		ZeroMemory(&m_RangeFirst, sizeof(m_RangeFirst));
		ZeroMemory(&m_RangeSecond, sizeof(m_RangeSecond));
	}
}

void CProperty::ResetModified()
{
	m_Modified = FALSE;
}

LFVariantData* CProperty::GetData()
{
	return p_Data;
}

BOOL CProperty::OnPushChar(UINT /*ch*/)
{
	return FALSE;
}


// CPropertyTags
//

CPropertyTags::CPropertyTags(LFVariantData* pData)
	: CProperty(pData)
{
}

BOOL CPropertyTags::HasButton()
{
	return TRUE;
}

void CPropertyTags::OnClickButton()
{
	ASSERT(p_Parent);

	LFEditTagsDlg dlg(m_Multiple ? _T("") : p_Data->UnicodeArray, p_Parent, p_Parent->m_StoreIDValid ? p_Parent->m_StoreID : NULL);

	if (dlg.DoModal()==IDOK)
	{
		wcscpy_s(p_Data->UnicodeArray, 256, dlg.m_Tags);
		p_Data->IsNull = FALSE;

		LFSanitizeUnicodeArray(p_Data->UnicodeArray, 256);

		p_Parent->NotifyOwner((SHORT)p_Data->Attr);
	}
}


// CPropertyRating
//

CPropertyRating::CPropertyRating(LFVariantData* pData)
	: CProperty(pData)
{
}

void CPropertyRating::DrawValue(CDC& dc, CRect rect)
{
	ASSERT(p_Parent);

	HDC hdcMem = CreateCompatibleDC(dc);
	UCHAR level = m_Multiple ? m_ShowRange ? m_RangeSecond.Rating : 0 : p_Data->Rating;
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, p_Data->Attr==LFAttrRating ? LFGetApp()->m_RatingBitmaps[level] : LFGetApp()->m_PriorityBitmaps[level]);

	INT w = min(rect.Width()-6, RatingBitmapWidth);
	INT h = min(rect.Height(), RatingBitmapHeight);
	BLENDFUNCTION BF = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };
	AlphaBlend(dc, rect.left+6, rect.top+(rect.Height()-h)/2, w, h, hdcMem, 0, 0, w, h, BF);

	SelectObject(hdcMem, hOldBitmap);
	DeleteDC(hdcMem);
}

HCURSOR CPropertyRating::SetCursor(INT x)
{
	ASSERT(p_Parent);

	return LFGetApp()->LoadStandardCursor(x<6 ? IDC_HAND : ((x<RatingBitmapWidth+6) && ((x-6)%18<16)) ? IDC_HAND : IDC_ARROW);
}

BOOL CPropertyRating::CanDelete()
{
	return FALSE;
}

BOOL CPropertyRating::WantsChars()
{
	return TRUE;
}

BOOL CPropertyRating::OnClickValue(INT x)
{
	ASSERT(p_Parent);

	if ((x>=0) && (x<RatingBitmapWidth+6))
		if ((x<6) || ((x-6)%18<16))
		{
			INT Rating = (x<6) ? 0 : 2*((x-6)/18)+((x-6)%18>8)+1;

			if (p_Data->Rating!=(UCHAR)Rating)
			{
				p_Data->IsNull = FALSE;
				p_Data->Rating = (UCHAR)Rating;
				p_Parent->NotifyOwner((SHORT)p_Data->Attr);
			}
		}

	return FALSE;
}

BOOL CPropertyRating::OnPushChar(UINT nChar)
{
	INT Rating = m_Multiple ? 0 : p_Data->Rating;

	switch (nChar)
	{
	case 0x27:
	case 0x6B:
	case 0xBB:
		Rating++;
		break;
	case 0x25:
	case 0x6D:
	case 0xBD:
		Rating--;
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
		Rating = (nChar-'0')*2;
		break;
	default:
		return FALSE;
	}

	if (Rating<0)
		Rating = 0;
	if (Rating>LFMaxRating)
		Rating = LFMaxRating;

	if (p_Data->Rating!=(UCHAR)Rating)
	{
		p_Data->IsNull = FALSE;
		p_Data->Rating = (UCHAR)Rating;
		p_Parent->NotifyOwner((SHORT)p_Data->Attr);
	}

	return TRUE;
}


// CPropertyIATA
//

CPropertyIATA::CPropertyIATA(LFVariantData* pData, LFVariantData* pLocationName, LFVariantData* pLocationGPS)
	: CProperty(pData)
{
	p_LocationName = pLocationName;
	p_LocationGPS = pLocationGPS;
}

CString CPropertyIATA::GetValidChars()
{
	return _T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
}

BOOL CPropertyIATA::HasButton()
{
	return TRUE;
}

void CPropertyIATA::OnSetString(CString Value)
{
	LFAirport* pAirportOld = NULL;
	if (!m_Multiple)
		LFIATAGetAirportByCode(p_Data->AnsiString, &pAirportOld);

	Value.Trim();
	Value.MakeUpper();

	WCHAR tmpStr[256];
	ToString(tmpStr, 256);
	if ((wcslen(tmpStr)!=0) && (wcslen(tmpStr)!=3))
		return;

	p_Data->IsNull = FALSE;
	WideCharToMultiByte(CP_ACP, 0, Value, -1, p_Data->AnsiString, 256, NULL, NULL);
	LFAirport* pAirportNew = NULL;
	LFIATAGetAirportByCode(p_Data->AnsiString, &pAirportNew);

	SHORT Attr2 = -1;
	if (p_LocationName && pAirportNew && !m_Multiple)
	{
		ASSERT(p_LocationName->Attr==LFAttrLocationName);

		BOOL Set = LFIsNullVariantData(*p_LocationName);
		if (pAirportOld)
		{
			MultiByteToWideChar(CP_ACP, 0, pAirportOld->Name, -1, tmpStr, 256);
			Set |= (wcscmp(p_LocationName->UnicodeString, tmpStr)==0);
		}

		if (Set)
		{
			Attr2 = LFAttrLocationName;

			p_LocationName->IsNull = FALSE;
			MultiByteToWideChar(CP_ACP, 0, pAirportNew->Name, -1, p_LocationName->UnicodeString, 256);
		}
	}

	SHORT Attr3 = -1;
	if (p_LocationGPS && pAirportNew && !m_Multiple)
	{
		ASSERT(p_LocationGPS->Attr==LFAttrLocationGPS);

		BOOL Set = LFIsNullVariantData(*p_LocationGPS);
		if (pAirportOld)
			Set |= (p_LocationGPS->GeoCoordinates.Latitude==pAirportOld->Location.Latitude) && (p_LocationGPS->GeoCoordinates.Longitude==pAirportOld->Location.Longitude);

		if (Set)
		{
			Attr3 = LFAttrLocationGPS;

			p_LocationGPS->IsNull = FALSE;
			p_LocationGPS->GeoCoordinates = pAirportNew->Location;
		}
	}

	p_Parent->NotifyOwner((SHORT)p_Data->Attr, Attr2, Attr3);
}

void CPropertyIATA::OnClickButton()
{
	ASSERT(p_Parent);

	LFSelectLocationIATADlg dlg(IDD_SELECTIATA, NULL, &p_Data->AnsiString[0], p_LocationName!=NULL, p_LocationGPS!=NULL);

	if (dlg.DoModal()==IDOK)
		if (dlg.p_Airport)
		{
			SHORT Attr2 = -1;
			if ((p_LocationName) && (dlg.m_OverwriteName))
			{
				ASSERT(p_LocationName->Attr==LFAttrLocationName);
				Attr2 = LFAttrLocationName;

				p_LocationName->IsNull = FALSE;
				MultiByteToWideChar(CP_ACP, 0, dlg.p_Airport->Name, -1, p_LocationName->UnicodeString, 256);
			}

			SHORT Attr3 = -1;
			if ((p_LocationGPS) && (dlg.m_OverwriteGPS))
			{
				ASSERT(p_LocationGPS->Attr==LFAttrLocationGPS);
				Attr3 = LFAttrLocationGPS;

				p_LocationGPS->IsNull = FALSE;
				p_LocationGPS->GeoCoordinates = dlg.p_Airport->Location;
			}

			p_Data->IsNull = FALSE;
			strcpy_s(p_Data->AnsiString, 256, dlg.p_Airport->Code);
			p_Parent->NotifyOwner((SHORT)p_Data->Attr, Attr2, Attr3);
		}
}


// CPropertyGPS
//

CPropertyGPS::CPropertyGPS(LFVariantData* pData)
	: CProperty(pData)
{
}

HCURSOR CPropertyGPS::SetCursor(INT /*x*/)
{
	ASSERT(p_Parent);

	return LFGetApp()->LoadStandardCursor(IDC_ARROW);
}

BOOL CPropertyGPS::HasButton()
{
	return TRUE;
}

BOOL CPropertyGPS::OnClickValue(INT /*x*/)
{
	return FALSE;
}

void CPropertyGPS::OnClickButton()
{
	ASSERT(p_Parent);

	LFSelectLocationGPSDlg dlg(p_Data->GeoCoordinates, p_Parent);

	if (dlg.DoModal()==IDOK)
	{
		p_Data->GeoCoordinates = dlg.m_Location;
		p_Data->IsNull = FALSE;
		p_Parent->NotifyOwner((SHORT)p_Data->Attr);
	}
}


// CPropertyTime
//

CPropertyTime::CPropertyTime(LFVariantData* pData)
	: CProperty(pData)
{
}

HCURSOR CPropertyTime::SetCursor(INT /*x*/)
{
	ASSERT(p_Parent);

	return LFGetApp()->LoadStandardCursor(IDC_ARROW);
}

BOOL CPropertyTime::HasButton()
{
	return TRUE;
}

BOOL CPropertyTime::OnClickValue(INT /*x*/)
{
	return FALSE;
}

void CPropertyTime::OnClickButton()
{
	ASSERT(p_Parent);

	LFEditTimeDlg dlg(p_Data, p_Parent);

	if (dlg.DoModal()==IDOK)
		p_Parent->NotifyOwner((SHORT)p_Data->Attr);
}


// CPropertyNumber
//

CPropertyNumber::CPropertyNumber(LFVariantData* pData)
	: CProperty(pData)
{
}

CString CPropertyNumber::GetValidChars()
{
	return _T("0123456789");
}


// CPropertyDuration
//

CPropertyDuration::CPropertyDuration(LFVariantData* pData)
	: CPropertyNumber(pData)
{
}

void CPropertyDuration::SetEditMask(CMFCMaskedEdit *pEdit)
{
	pEdit->EnableMask(_T("DD DD DD"), _T("__:__:__"), _T('0'), _T("0123456789"));
	pEdit->EnableGetMaskedCharsOnly(FALSE);
	pEdit->SetWindowText(_T("00:00:00"));
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

extern INT GetAttributeIconIndex(UINT Attr);

#define GUTTER         3
#define PADDING        2

#define NOPART         0
#define PARTLABEL      1
#define PARTVALUE      2
#define PARTBUTTON     3
#define PARTRESET      4

CInspectorGrid::CInspectorGrid()
	: CPropertyHolder()
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
	wndcls.lpszClassName = L"CInspectorGrid";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CInspectorGrid", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	m_SortAlphabetic = m_Hover = m_PartPressed = FALSE;
	m_pSortArray = NULL;
	m_pHeader = NULL;
	hThemeList = hThemeButton = NULL;
	hIconResetNormal = hIconResetHot = hIconResetPressed = NULL;
	m_VScrollMax = m_VScrollPos = m_IconSize = 0;
	m_HotItem = m_SelectedItem = m_EditItem = -1;
	m_HotPart = NOPART;
	p_Edit = NULL;
}

CInspectorGrid::~CInspectorGrid()
{
	DestroyEdit();

	delete[] m_pSortArray;
}

BOOL CInspectorGrid::Create(CWnd* pParentWnd, UINT nID, CInspectorHeader* pHeader)
{
	m_pHeader = pHeader;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL, rect, pParentWnd, nID);
}

void CInspectorGrid::PreSubclassWindow()
{
	CPropertyHolder::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (!pThreadState->m_pWndInit)
		Init();
}

void CInspectorGrid::Init()
{
	if (LFGetApp()->m_ThemeLibLoaded)
	{
		hThemeButton = LFGetApp()->zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
		if (LFGetApp()->OSVersion>=OS_Vista)
		{
			LFGetApp()->zSetWindowTheme(GetSafeHwnd(), L"EXPLORER", NULL);
			hThemeList = LFGetApp()->zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
		}
	}

	ResetScrollbars();
	CreateFonts();

	m_AttributeIcons.Create(IDB_ATTRIBUTEICONS_32, 32, 32);
	m_TooltipCtrl.Create(this);

	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&LFGetApp()->m_LargeFont);
	m_FontHeight[1] = dc->GetTextExtent(_T("Wy")).cy;
	dc->SelectStockObject(DEFAULT_GUI_FONT);
	m_FontHeight[0] = dc->GetTextExtent(_T("Wy")).cy;
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	m_RowHeight = max(m_FontHeight[0]+2, 16);
	m_IconSize = (m_RowHeight>=27) ? 27 : (m_RowHeight>=22) ? 22 : 16;
	hIconResetNormal = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_RESET_NORMAL), IMAGE_ICON, m_IconSize, m_IconSize, LR_SHARED);
	hIconResetHot = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_RESET_HOT), IMAGE_ICON, m_IconSize, m_IconSize, LR_SHARED);
	hIconResetPressed = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_RESET_PRESSED), IMAGE_ICON, m_IconSize, m_IconSize, LR_SHARED);
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

	return CPropertyHolder::PreTranslateMessage(pMsg);
}

void CInspectorGrid::AddProperty(CProperty* pProperty, UINT Category, WCHAR* Name, BOOL Editable)
{
	pProperty->SetParent(this);

	Property prop;
	prop.pProperty = pProperty;
	prop.Category = Category;
	wcscpy_s(prop.Name, 256, Name);
	prop.Visible = FALSE;
	prop.Editable = Editable;
	prop.Top = prop.Bottom = prop.LabelWidth = -1;

	WCHAR tmpName[256];
	wcscpy_s(tmpName, 256, Name);
	wcscat_s(tmpName, 256, L":");

	CDC* dc = GetWindowDC();
	CGdiObject* pOldFont = dc->SelectStockObject(DEFAULT_GUI_FONT);
	prop.LabelWidth = dc->GetTextExtent(tmpName).cx;
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
	for (UINT a=0; a<LFAttributeCount; a++)
	{
		CProperty* pProp = CreateProperty(&pData[a]);
		if (a==LFAttrLocationIATA)
		{
			((CPropertyIATA*)pProp)->p_LocationName = &pData[LFAttrLocationName];
			((CPropertyIATA*)pProp)->p_LocationGPS = &pData[LFAttrLocationGPS];
		}

		LFAttributeDescriptor* pAttr = &LFGetApp()->m_Attributes[a];
		AddProperty(pProp, pAttr->Category, pAttr->Name, !pAttr->ReadOnly);
	}
}

void CInspectorGrid::SetAlphabeticMode(BOOL SortAlphabetic)
{
	DestroyEdit();

	m_SortAlphabetic = SortAlphabetic;
	MakeSortArrayDirty();
	AdjustLayout();
	EnsureVisible(m_SelectedItem);
}

void CInspectorGrid::UpdatePropertyState(UINT nID, BOOL Multiple, BOOL Editable, BOOL Visible, LFVariantData* pRangeFirst, LFVariantData* pRangeSecond)
{
	ASSERT(nID<m_Properties.m_ItemCount);

	DestroyEdit();

	m_Properties.m_Items[nID].Editable = Editable;
	m_Properties.m_Items[nID].Visible = Visible;
	m_Properties.m_Items[nID].pProperty->SetMultiple(Multiple, pRangeFirst, pRangeSecond);
	m_Properties.m_Items[nID].pProperty->ResetModified();
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
					*PartID = PARTLABEL;
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
						*PartID = PARTRESET;
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
						*PartID = PARTBUTTON;
						return a;
					}
				}

				*PartID = PARTVALUE;
			}

			return a;
		}
	}

	if (PartID)
		*PartID = NOPART;
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
		ScrollWindow(0, -nInc);

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
	ScrollWindow(0, m_VScrollPos);
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

INT CInspectorGrid::Compare(INT Eins, INT Zwei)
{
	Property* pEins = &m_Properties.m_Items[m_pSortArray[Eins]];
	Property* pZwei = &m_Properties.m_Items[m_pSortArray[Zwei]];

	if (m_SortAlphabetic)
		return wcscmp(pEins->Name, pZwei->Name);

	return (pEins->Category==pZwei->Category) ? m_pSortArray[Eins]-m_pSortArray[Zwei] : (INT)pEins->Category-(INT)pZwei->Category;
}

void CInspectorGrid::Heap(INT Wurzel, INT Anz)
{
	while (Wurzel<=Anz/2-1)
	{
		INT Index = (Wurzel+1)*2-1;
		if (Index+1<Anz)
			if (Compare(Index, Index+1)<0)
				Index++;

		if (Compare(Wurzel, Index)<0)
		{
			INT Temp = m_pSortArray[Wurzel];
			m_pSortArray[Wurzel] = m_pSortArray[Index];
			m_pSortArray[Index] = Temp;

			Wurzel = Index;
		}
		else
		{
			break;
		}
	}
}

__forceinline void CInspectorGrid::CreateSortArray()
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
		for (INT a=m_Properties.m_ItemCount-1; a>0; a--)
		{
			INT Temp = m_pSortArray[0];
			m_pSortArray[0] = m_pSortArray[a];
			m_pSortArray[a] = Temp;

			Heap(0, a);
		}
	}
}

__forceinline void CInspectorGrid::MakeSortArrayDirty()
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
	m_ScrollHeight = m_pHeader ? m_pHeader->GetPreferredHeight()+1 : PADDING+1;
	INT Category = -1;

	for (UINT a=0; a<m_Properties.m_ItemCount; a++)
	{
		Property* pProp = &m_Properties.m_Items[m_pSortArray[a]];

		if ((!m_SortAlphabetic) && (pProp->Visible) && ((INT)pProp->Category!=Category))
		{
			Category = pProp->Category;
			INT Spacer = (m_ScrollHeight==1) ? -PADDING : 8;
			m_Categories[Category].Top = m_ScrollHeight+Spacer;
			m_Categories[Category].Bottom = m_ScrollHeight+m_FontHeight[1]+2*LFCategoryPadding+Spacer;
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

	if (m_SelectedItem==-1)
		m_SelectedItem = 0;

	if (!m_Properties.m_Items[m_SelectedItem].Visible)
	{
		m_SelectedItem = -1;

		for (UINT a=0; a<m_Properties.m_ItemCount; a++)
			if (m_Properties.m_Items[m_pSortArray[a]].Visible)
			{
				m_SelectedItem = m_pSortArray[a];
				break;
			}
	}

	AdjustScrollbars();
	Invalidate();
}

void CInspectorGrid::ScrollWindow(INT dx, INT dy)
{
	CWnd::ScrollWindow(dx, dy);
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

	if ((m_Properties.m_Items[Attr].Editable) && (Attr!=LFAttrFileName))
	{
		LFClearVariantData(*m_Properties.m_Items[Attr].pProperty->GetData());
		m_Properties.m_Items[Attr].pProperty->m_Modified = TRUE;

		NotifyOwner((SHORT)Attr);
	}
}

void CInspectorGrid::EditProperty(UINT Attr)
{
	ASSERT(Attr<m_Properties.m_ItemCount);
	m_EditItem = -1;

	Property* pProp = &m_Properties.m_Items[Attr];
	if (pProp->Editable)
	{
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
			}
			p_Edit->SetValidChars(pProp->pProperty->GetValidChars());
			pProp->pProperty->SetEditMask(p_Edit);
			if (Attr<LFAttributeCount)
				p_Edit->SetLimitText(LFGetApp()->m_Attributes[Attr].cCharacters);
			p_Edit->SetFont(&m_BoldFont);
			p_Edit->SetFocus();
			return;
		}

		if (pProp->pProperty->HasButton())
			pProp->pProperty->OnClickButton();
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


BEGIN_MESSAGE_MAP(CInspectorGrid, CPropertyHolder)
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
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_GETDLGCODE()
	ON_WM_SETCURSOR()
	ON_EN_KILLFOCUS(1, OnDestroyEdit)
END_MESSAGE_MAP()

INT CInspectorGrid::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPropertyHolder::OnCreate(lpCreateStruct)==-1)
		return -1;

	Init();

	return 0;
}

void CInspectorGrid::OnDestroy()
{
	CPropertyHolder::OnDestroy();

	if (hThemeButton)
		LFGetApp()->zCloseThemeData(hThemeButton);
	if (hThemeList)
		LFGetApp()->zCloseThemeData(hThemeList);

	for (UINT a=0; a<m_Properties.m_ItemCount; a++)
		delete m_Properties.m_Items[a].pProperty;
}

LRESULT CInspectorGrid::OnThemeChanged()
{
	if (LFGetApp()->m_ThemeLibLoaded)
	{
		if (hThemeButton)
			LFGetApp()->zCloseThemeData(hThemeButton);
		if (hThemeList)
			LFGetApp()->zCloseThemeData(hThemeList);

		hThemeButton = LFGetApp()->zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
		if (LFGetApp()->OSVersion>=OS_Vista)
			hThemeList = LFGetApp()->zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
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

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	BOOL Themed = IsCtrlThemed();

	dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

	// Categories
	CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DefaultFont);

	for (UINT a=0; a<LFAttrCategoryCount; a++)
		if (m_Categories[a].Top!=-1)
		{
			CRect rect(1, m_Categories[a].Top-m_VScrollPos, rect.Width()-1, m_Categories[a].Bottom-m_VScrollPos);
			DrawCategory(dc, rect, LFGetApp()->m_AttrCategoryNames[a], NULL, Themed);
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
						LFGetApp()->zDrawThemeBackground(hThemeList, dc, LVP_LISTITEM, StateIDs[State], &rectProp, &rectProp);
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
			dc.DrawText(CString(pProp->Name)+_T(":"), rectLabel, DT_RIGHT | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);

			rectLabel.left = rectLabel.right+GUTTER;
			rectLabel.right = rect.Width();

			if ((pProp->Editable) && (pProp->pProperty->CanDelete()))
			{
				INT Offs = (rectLabel.Height()-m_IconSize)/2;
				DrawIconEx(dc, rectLabel.right-m_IconSize-Offs-2, rectLabel.top+Offs, ((INT)a==m_HotItem) && (m_HotPart==PARTRESET) ? m_PartPressed ? hIconResetPressed : hIconResetHot : hIconResetNormal, m_IconSize, m_IconSize, 0, NULL, DI_NORMAL);
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

				BOOL Hot = ((INT)a==m_HotItem) && (m_HotPart==PARTBUTTON);
				BOOL Pressed = Hot && m_PartPressed;

				if (hThemeButton)
				{
					rectButton.InflateRect(1, 1);

					UINT State = Pressed ? PBS_PRESSED : Hot ? PBS_HOT : PBS_NORMAL;
					LFGetApp()->zDrawThemeBackground(hThemeButton, dc, BP_PUSHBUTTON, State, &rectButton, NULL);
				}
				else
				{
					COLORREF c1 = Pressed ? 0x000000 : GetSysColor(COLOR_3DHIGHLIGHT);
					COLORREF c2 = Pressed ? GetSysColor(COLOR_3DSHADOW) : GetSysColor(COLOR_3DFACE);
					COLORREF c3 = Pressed ? GetSysColor(COLOR_3DFACE) : GetSysColor(COLOR_3DSHADOW);
					COLORREF c4 = Pressed ? GetSysColor(COLOR_3DHIGHLIGHT) : 0x000000;

					dc.FillSolidRect(rectButton, c2);

					CRect rectBorder(rectButton);
					dc.Draw3dRect(rectBorder, c1, c4);
					rectBorder.DeflateRect(1, 1);
					dc.Draw3dRect(rectBorder, c2, c3);
				}

				rectButton.bottom -= 4;
				if (Pressed)
					rectButton.OffsetRect(1, 1);

				dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
				dc.DrawText(_T("..."), rectButton, DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
			}

			dc.SetTextColor(clr2);
			pProp->pProperty->DrawValue(dc, rectLabel);
		}
	}

	//Header
	if (m_pHeader)
	{
		CRect rectHeader(0, -m_VScrollPos, rect.Width(), m_pHeader->GetPreferredHeight()-m_VScrollPos);
		m_pHeader->DrawHeader(dc, rectHeader, Themed);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}

void CInspectorGrid::OnSize(UINT nType, INT cx, INT cy)
{
	CPropertyHolder::OnSize(nType, cx, cy);
	AdjustLayout();

	if (p_Edit)
	{
		CRect rect;
		p_Edit->GetWindowRect(&rect);
		ScreenToClient(&rect);

		rect.left = m_LabelWidth+GUTTER;
		rect.right = cx-1;
		p_Edit->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}
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
		ScrollWindow(0, -nInc);

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = m_VScrollPos;
		SetScrollInfo(SB_VERT, &si);

		if (p_Edit)
		{
			CRect rect;
			p_Edit->GetWindowRect(&rect);
			ScreenToClient(&rect);

			rect.OffsetRect(0, -nInc);
			p_Edit->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	CPropertyHolder::OnVScroll(nSBCode, nPos, pScrollBar);
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
	m_HotPart = NOPART;
}

void CInspectorGrid::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if ((m_HotItem!=-1) && !p_Edit)
			if (!m_TooltipCtrl.IsWindowVisible())
			{
				Property* pProp = &m_Properties.m_Items[m_HotItem];
				ASSERT(pProp);

				INT Index = GetAttributeIconIndex(m_HotItem);
				HICON hIcon = (Index!=-1) ? m_AttributeIcons.ExtractIcon(Index) : NULL;

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

	INT nScrollLines;
	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nScrollLines, 0);
	if (nScrollLines<1)
		nScrollLines = 1;

	INT nInc = max(-m_VScrollPos, min(-zDelta*(INT)(m_RowHeight+1)*nScrollLines/WHEEL_DELTA, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_TooltipCtrl.Deactivate();

		m_VScrollPos += nInc;
		ScrollWindow(0, -nInc);
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
	INT Item = HitTest(point, &Part);
	if (Item!=-1)
	{
		SelectItem(Item);

		if (Part>=PARTBUTTON)
		{
			m_HotPart = Part;
			m_PartPressed = TRUE;
			SetCapture();
			InvalidateItem(m_SelectedItem);
		}
	}
}

void CInspectorGrid::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	INT Last = m_SelectedItem;

	switch(nChar)
	{
	case VK_HOME:
		for (UINT a=0; a<m_Properties.m_ItemCount; a++)
			if (m_Properties.m_Items[m_pSortArray[a]].Visible)
			{
				SelectItem(m_pSortArray[a]);
				break;
			}
		break;
	case VK_UP:
		for (UINT a=0; a<m_Properties.m_ItemCount; a++)
		{
			if (m_pSortArray[a]==m_SelectedItem)
			{
				SelectItem(Last);
				break;
			}
			if (m_Properties.m_Items[m_pSortArray[a]].Visible)
				Last = m_pSortArray[a];
		}
		break;
	case VK_DOWN:
		for (INT a=m_Properties.m_ItemCount-1; a>=0; a--)
		{
			if (m_pSortArray[a]==m_SelectedItem)
			{
				SelectItem(Last);
				break;
			}
			if (m_Properties.m_Items[m_pSortArray[a]].Visible)
				Last = m_pSortArray[a];
		}
		break;
	case VK_END:
		for (INT a=m_Properties.m_ItemCount-1; a>=0; a--)
			if (m_Properties.m_Items[m_pSortArray[a]].Visible)
			{
				SelectItem(m_pSortArray[a]);
				break;
			}
		break;
	case VK_EXECUTE:
	case VK_RETURN:
		if ((GetKeyState(VK_CONTROL)<0) && (m_SelectedItem!=-1))
			if (m_Properties.m_Items[m_SelectedItem].pProperty->HasButton())
			{
				m_Properties.m_Items[m_SelectedItem].pProperty->OnClickButton();
				break;
			}
	case VK_F2:
		if (m_SelectedItem!=-1)
			EditProperty(m_SelectedItem);
		break;
	case VK_BACK:
	case VK_DELETE:
	case VK_SPACE:
		if (m_SelectedItem!=-1)
			ResetProperty(m_SelectedItem);
		break;
	case VK_TAB:
		{
			CWnd* pWnd = GetParent()->GetNextDlgTabItem(GetFocus(), GetKeyState(VK_SHIFT)<0);
			if (pWnd)
				pWnd->SetFocus();
			break;
		}
	case VK_ESCAPE:
		GetParent()->SendMessage(WM_COMMAND, IDCANCEL);
		break;
	default:
		if (m_SelectedItem!=-1)
		{
			Property* pProp = &m_Properties.m_Items[m_SelectedItem];
			if ((pProp->Editable) && (pProp->pProperty->WantsChars()))
				if (m_Properties.m_Items[m_SelectedItem].pProperty->OnPushChar(nChar))
					break;
		}

		CPropertyHolder::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

void CInspectorGrid::OnLButtonUp(UINT nFlags, CPoint point)
{
	UINT Part;
	INT Item = HitTest(point, &Part);

	if (GetCapture()==this)
	{
		m_PartPressed = FALSE;
		ReleaseCapture();
		InvalidateItem(Item);

		if ((Item==m_SelectedItem) && (Item!=-1))
			if (Part==PARTRESET)
			{
				BOOL DoReset = (nFlags & MK_CONTROL);

				if (!DoReset)
				{
					CString Caption((LPCSTR)IDS_DELETEPROPERTY_CAPTION);
					CString Message((LPCSTR)IDS_DELETEPROPERTY_MSG);

					DoReset = (MessageBox(Message, Caption, MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING)==IDYES);
				}

				if (DoReset)
					ResetProperty(Item);
			}
			else
				if (Part==PARTBUTTON)
				{
					m_Properties.m_Items[Item].pProperty->OnClickButton();
				}
	}
	else
		if ((Item==m_SelectedItem) && (Item!=-1) && (Part==PARTVALUE))
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
	if ((Item!=-1) && (Part<=PARTBUTTON))
	{
		SelectItem(Item);
		EditProperty(Item);
	}
}

UINT CInspectorGrid::OnGetDlgCode()
{
	return DLGC_WANTARROWS | DLGC_WANTCHARS | DLGC_WANTALLKEYS;
}

BOOL CInspectorGrid::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*Message*/)
{
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	UINT Part;
	INT Item = HitTest(point, &Part);

	if ((Item!=-1) && (Part==PARTVALUE))
	{
		Property* pProp = &m_Properties.m_Items[Item];

		if (pProp->Editable)
		{
			SetCursor(pProp->pProperty->SetCursor(point.x-m_LabelWidth-GUTTER));
			return TRUE;
		}
	}

	SetCursor(LFGetApp()->LoadStandardCursor(IDC_ARROW));
	return TRUE;
}

void CInspectorGrid::OnDestroyEdit()
{
	DestroyEdit(TRUE);
}
