
// CInspectorGrid.cpp: Implementierung der Klasse CInspectorGrid
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFEditGenreDlg.h"
#include "LFEditHashtagsDlg.h"
#include "LFEditTimeDlg.h"


// CProperty
//

CString CProperty::m_MultipleValues;

CProperty::CProperty(LFVariantData* pVData, CWnd* pOwnerWnd)
{
	ASSERT(pVData);
	ASSERT(pOwnerWnd);

	if (m_MultipleValues.IsEmpty())
		ENSURE(m_MultipleValues.LoadString(IDS_MULTIPLEVALUES));

	p_OwnerWnd = pOwnerWnd;
	p_VData = pVData;
	p_VDataRange = NULL;
	m_Modified = m_Multiple = m_ShowRange = FALSE;
}

CProperty* CProperty::CreateProperty(LFVariantData* pVData, CWnd* pOwnerWnd)
{
	ASSERT(pVData);
	ASSERT(pVData->Type<LFTypeCount);
	ASSERT(pOwnerWnd);

	CProperty* pProperty;

	switch (pVData->Type)
	{
	case LFTypeIATACode:
		pProperty = new CPropertyIATA(pVData, pOwnerWnd);
		break;

	case LFTypeRating:
		pProperty = new CPropertyRating(pVData, pOwnerWnd);
		break;

	case LFTypeUINT:
	case LFTypeYear:
		pProperty = new CPropertyNumber(pVData, pOwnerWnd);
		break;

	case LFTypeSize:
		pProperty = new CPropertySize(pVData, pOwnerWnd);
		break;

	case LFTypeColor:
		pProperty = new CPropertyColor(pVData, pOwnerWnd);
		break;

	case LFTypeGeoCoordinates:
		pProperty = new CPropertyGeoCoordinates(pVData, pOwnerWnd);
		break;

	case LFTypeTime:
		pProperty = new CPropertyTime(pVData, pOwnerWnd);
		break;

	case LFTypeDuration:
		pProperty = new CPropertyDuration(pVData, pOwnerWnd);
		break;

	case LFTypeGenre:
		pProperty = new CPropertyGenre(pVData, pOwnerWnd);
		break;

	case LFTypeUnicodeArray:
		if (pVData->Attr==LFAttrHashtags) 
		{
			pProperty = new CPropertyHashtags(pVData, pOwnerWnd);
			break;
		}

	default:
		pProperty = new CProperty(pVData, pOwnerWnd);
	}

	ASSERT(pProperty);

	return pProperty;
}

void CProperty::ToString(LPWSTR pStr, SIZE_T nCount) const
{
	if (m_Multiple)
	{
		// Multiple values
		if (m_ShowRange)
		{
			// Range
			WCHAR tmpBuf[256];
			LFVariantDataToString(*p_VDataRange, tmpBuf, 256);

			LFVariantDataToString(*p_VData, pStr, nCount);
			wcscat_s(pStr, nCount, L" – ");
			wcscat_s(pStr, nCount, tmpBuf);
		}
		else
		{
			// Notification
			wcscpy_s(pStr, nCount, m_MultipleValues);
		}
	}
	else
	{
		// Attribute value
		LFVariantDataToString(*p_VData, pStr, nCount);
	}
}

INT CProperty::GetMinWidth() const
{
	return 8*LFGetApp()->m_DialogFont.GetFontHeight();
}

void CProperty::DrawValue(CDC& dc, LPCRECT lpcRect) const
{
	WCHAR tmpStr[256];
	ToString(tmpStr, 256);

	CFont* pOldFont = m_Multiple ? dc.SelectObject(&LFGetApp()->m_DialogItalicFont) : m_Modified ? dc.SelectObject(&LFGetApp()->m_DialogBoldFont) : NULL;
	dc.DrawText(tmpStr, -1, (LPRECT)lpcRect, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS | DT_SINGLELINE | DT_NOPREFIX);

	if (pOldFont)
		dc.SelectObject(pOldFont);
}

HCURSOR CProperty::SetCursor(INT /*x*/) const
{
	return LFGetApp()->LoadStandardCursor(IDC_IBEAM);
}

CMFCMaskedEdit* CProperty::CreateEditControl(const CRect& rect, HFONT hFont) const
{
	CMFCMaskedEdit* pWndEdit = new CMFCMaskedEdit();
	pWndEdit->Create(WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ES_AUTOHSCROLL, rect, p_OwnerWnd, 2);
	
	// Font
	pWndEdit->SendMessage(WM_SETFONT, (WPARAM)hFont);

	// Limit
	if (p_VData->Attr<LFAttributeCount)
		pWndEdit->SetLimitText((UINT)LFGetApp()->m_Attributes[p_VData->Attr].AttrProperties.cCharacters);

	// Default text
	if (!HasMultipleValues())
	{
		WCHAR tmpStr[256];
		ToString(tmpStr, 256);

		pWndEdit->SetWindowText(tmpStr);
	}

	return pWndEdit;
}

void CProperty::OnSetString(CString& Value) const
{
	// Prepare new string
	Value.Trim();

	if (p_VData->Attr==LFAttrLanguage)
		Value.MakeUpper();

	// Old string
	WCHAR tmpStr[256];
	ToString(tmpStr, 256);

	// Is new value different?
	if (((p_VData->Attr!=LFAttrFileName) || !Value.IsEmpty()) && (wcscmp(tmpStr, Value)!=0))
	{
		LFVariantDataFromString(*p_VData, Value);

		NotifyOwner();
	}
}

BOOL CProperty::OnClickValue(INT /*x*/) const
{
	return TRUE;
}

void CProperty::OnClickButton(const STOREID& /*StoreID*/)
{
}

BOOL CProperty::CanDelete() const
{
	return (p_VData->Attr!=LFAttrFileName) && (!LFIsNullVariantData(*p_VData) || m_Multiple);
}

BOOL CProperty::HasButton() const
{
	return FALSE;
}

BOOL CProperty::WantsChars() const
{
	return FALSE;
}
	
void CProperty::UpdateState(BOOL Multiple, const LFVariantData* pVDataRange)
{
	const UINT Type = p_VData->Type;

	m_ShowRange = ((m_Multiple=Multiple)==TRUE) &&
		((p_VDataRange=pVDataRange)!=NULL) && !LFIsNullVariantData(*pVDataRange) &&
		((Type==LFTypeRating) || (Type==LFTypeUINT) || (Type==LFTypeYear) || (Type==LFTypeSize) || (Type==LFTypeDouble) || (Type==LFTypeTime) || (Type==LFTypeBitrate) || (Type==LFTypeDuration) || (Type==LFTypeMegapixel));

	m_Modified = FALSE;
}

BOOL CProperty::OnPushChar(UINT /*ch*/) const
{
	return FALSE;
}


// CPropertyHashtags
//

CPropertyHashtags::CPropertyHashtags(LFVariantData* pVData, CWnd* pOwnerWnd)
	: CProperty(pVData, pOwnerWnd)
{
	ASSERT(pVData);
	ASSERT(pVData->Type==LFTypeUnicodeArray);
}

BOOL CPropertyHashtags::HasButton() const
{
	return TRUE;
}

void CPropertyHashtags::OnClickButton(const STOREID& StoreID)
{
	LFEditHashtagsDlg dlg(m_Multiple ? _T("") : p_VData->UnicodeArray, StoreID, p_OwnerWnd);
	if (dlg.DoModal()==IDOK)
	{
		WCHAR Buffer[4096];
		wcscpy_s(Buffer, 4096, dlg.m_Hashtags);
		LFSanitizeUnicodeArray(Buffer, 4096);

		wcsncpy_s(p_VData->UnicodeArray, 256, Buffer, _TRUNCATE);
		p_VData->IsNull = FALSE;

		NotifyOwner();
	}
}


// CPropertyIATA
//

CPropertyIATA::CPropertyIATA(LFVariantData* pVData, CWnd* pOwnerWnd)
	: CProperty(pVData, pOwnerWnd)
{
	ASSERT(pVData);
	ASSERT(pVData->Type==LFTypeIATACode);

	p_VDataLocationName = p_VDataLocationGPS = NULL;
}

CMFCMaskedEdit* CPropertyIATA::CreateEditControl(const CRect& rect, HFONT hFont) const
{
	CMFCMaskedEdit* pWndEdit = CProperty::CreateEditControl(rect, hFont);

	pWndEdit->SetValidChars(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

	return pWndEdit;
}

BOOL CPropertyIATA::HasButton() const
{
	return TRUE;
}

void CPropertyIATA::OnSetString(CString& Value) const
{
	// Prepare new string
	Value.Trim();

	if ((Value.GetLength()!=0) && (Value.GetLength()!=3))
		return;

	// Get old airport
	LPCAIRPORT pAirportOld = NULL;
	if (!m_Multiple)
		LFIATAGetAirportByCode(p_VData->IATACode, pAirportOld);

	// Set new airport
	LFVariantDataFromString(*p_VData, Value);

	// Try to find the airport, and replace location name and coordinates with data from airport database
	SHORT Attr2 = -1;
	SHORT Attr3 = -1;

	LPCAIRPORT pAirportNew = NULL;
	if (LFIATAGetAirportByCode(p_VData->IATACode, pAirportNew) && !m_Multiple)
	{
		// Location name
		if (p_VDataLocationName)
		{
			BOOL OverwriteLocationName = LFIsNullVariantData(*p_VDataLocationName);

			if (pAirportOld)
			{
				WCHAR tmpStr[256];
				MultiByteToWideChar(CP_ACP, 0, pAirportOld->Name, -1, tmpStr, 256);

				OverwriteLocationName |= (wcscmp(p_VDataLocationName->UnicodeString, tmpStr)==0);
			}

			if (OverwriteLocationName)
				Attr2 = OnSetLocationName(pAirportNew);
		}

		// GPS location
		if (p_VDataLocationGPS)
		{
			BOOL OverwriteLocationGPS = LFIsNullVariantData(*p_VDataLocationGPS);

			if (pAirportOld)
				OverwriteLocationGPS |= (p_VDataLocationGPS->GeoCoordinates.Latitude==pAirportOld->Location.Latitude) && (p_VDataLocationGPS->GeoCoordinates.Longitude==pAirportOld->Location.Longitude);

			if (OverwriteLocationGPS)
				Attr3 = OnSetLocationGPS(pAirportNew);
		}
	}

	NotifyOwner(Attr2, Attr3);
}

void CPropertyIATA::OnClickButton(const STOREID& /*StoreID*/)
{
	LFSelectPropertyIATADlg dlg(NULL, &p_VData->AnsiString[0], p_VDataLocationName!=NULL, p_VDataLocationGPS!=NULL);
	if ((dlg.DoModal()==IDOK) && dlg.p_Airport)
	{
		// IATA code
		strcpy_s(p_VData->IATACode, 256, dlg.p_Airport->Code);
		p_VData->IsNull = FALSE;

		// Other attributes
		const SHORT Attr2 = dlg.m_OverwriteName ? OnSetLocationName(dlg.p_Airport) : -1;
		const SHORT Attr3 = dlg.m_OverwriteGPS ? OnSetLocationGPS(dlg.p_Airport) : -1;

		NotifyOwner(Attr2, Attr3);
	}
}

void CPropertyIATA::SetAdditionalVData(LFVariantData* pVDataLocationName, LFVariantData* pVDataLocationGPS)
{
	if ((p_VDataLocationName=pVDataLocationName)!=NULL)
		ASSERT(LFGetApp()->m_Attributes[p_VDataLocationName->Attr].AttrProperties.Type==LFTypeUnicodeString);

	if ((p_VDataLocationGPS=pVDataLocationGPS)!=NULL)
		ASSERT(LFGetApp()->m_Attributes[p_VDataLocationGPS->Attr].AttrProperties.Type==LFTypeGeoCoordinates);
}

SHORT CPropertyIATA::OnSetLocationName(LPCAIRPORT lpcAirport) const
{
	ASSERT(lpcAirport);

	if (p_VDataLocationName)
	{
		MultiByteToWideChar(CP_ACP, 0, lpcAirport->Name, -1, p_VDataLocationName->UnicodeString, 256);
		p_VDataLocationName->IsNull = FALSE;

		return (SHORT)p_VDataLocationName->Attr;
	}

	return -1;
}

SHORT CPropertyIATA::OnSetLocationGPS(LPCAIRPORT lpcAirport) const
{
	ASSERT(lpcAirport);

	if (p_VDataLocationGPS)
	{
		p_VDataLocationGPS->GeoCoordinates = lpcAirport->Location;
		p_VDataLocationGPS->IsNull = FALSE;

		return (SHORT)p_VDataLocationGPS->Attr;
	}

	return -1;
}

// CPropertyRating
//

CPropertyRating::CPropertyRating(LFVariantData* pVData, CWnd* pOwnerWnd)
	: CProperty(pVData, pOwnerWnd)
{
	ASSERT(pVData);
	ASSERT(pVData->Type==LFTypeRating);
}

INT CPropertyRating::GetMinWidth() const
{
	return RATINGBITMAPWIDTH+6;
}

void CPropertyRating::DrawValue(CDC& dc, LPCRECT lpcRect) const
{
	// Rating bitmap
	const UCHAR Rating = m_Multiple ? m_ShowRange ? p_VDataRange->Rating : 0 : p_VData->Rating;
	ASSERT(Rating<=LFMaxRating);

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	HBITMAP hOldBitmap = (HBITMAP)dcMem.SelectObject(p_VData->Attr==LFAttrPriority ? LFGetApp()->hPriorityBitmaps[Rating] : LFGetApp()->hRatingBitmaps[Rating]);

	dc.AlphaBlend(lpcRect->left+6, (lpcRect->top+lpcRect->bottom-RATINGBITMAPHEIGHT)/2, RATINGBITMAPWIDTH, RATINGBITMAPHEIGHT, &dcMem, 0, 0, RATINGBITMAPWIDTH, RATINGBITMAPHEIGHT, BF);

	SelectObject(dcMem, hOldBitmap);
}

HCURSOR CPropertyRating::SetCursor(INT x) const
{
	return LFGetApp()->LoadStandardCursor(x<6 ? IDC_HAND : ((x<RATINGBITMAPWIDTH+6) && ((x-6)%18<16)) ? IDC_HAND : IDC_ARROW);
}

BOOL CPropertyRating::CanDelete() const
{
	return FALSE;
}

BOOL CPropertyRating::WantsChars() const
{
	return TRUE;
}

BOOL CPropertyRating::OnClickValue(INT x) const
{
	if ((x>=0) && (x<RATINGBITMAPWIDTH+6))
		if ((x<6) || ((x-6)%18<16))
			OnSetRating((UCHAR)((x<6) ? 0 : 2*((x-6)/18)+((x-6)%18>8)+1));

	return FALSE;
}

BOOL CPropertyRating::OnPushChar(UINT nChar) const
{
	INT Rating = m_Multiple ? 0 : p_VData->Rating;

	switch (nChar)
	{
	case 0x27:
	case 0x6B:
	case 0xBB:
		if (++Rating>LFMaxRating)
			Rating = LFMaxRating;

		break;

	case 0x25:
	case 0x6D:
	case 0xBD:
		if (--Rating<0)
			Rating = 0;

		break;

	case L'0':
	case L'1':
	case L'2':
	case L'3':
	case L'4':
	case L'5':
		Rating = (nChar-L'0')*2;
		break;

	default:
		return FALSE;
	}

	OnSetRating((UCHAR)Rating);

	return TRUE;
}

void CPropertyRating::OnSetRating(UCHAR Rating) const
{
	assert(Rating<=LFMaxRating);

	if (p_VData->Rating!=Rating)
	{
		p_VData->Rating = Rating;
		p_VData->IsNull = FALSE;

		NotifyOwner();
	}
}


// CPropertyNumber
//

CPropertyNumber::CPropertyNumber(LFVariantData* pVData, CWnd* pOwnerWnd)
	: CProperty(pVData, pOwnerWnd)
{
	ASSERT(pVData);
	ASSERT((pVData->Type==LFTypeUINT) || (pVData->Type==LFTypeDuration) || (pVData->Type==LFTypeYear));
}

CMFCMaskedEdit* CPropertyNumber::CreateEditControl(const CRect& rect, HFONT hFont) const
{
	CMFCMaskedEdit* pWndEdit = CProperty::CreateEditControl(rect, hFont);

	pWndEdit->SetValidChars(_T("0123456789"));

	return pWndEdit;
}


// CPropertySize
//

CPropertySize::CPropertySize(LFVariantData* pVData, CWnd* pOwnerWnd)
	: CProperty(pVData, pOwnerWnd)
{
	ASSERT(pVData);
	ASSERT(pVData->Type==LFTypeSize);
}

CMFCMaskedEdit* CPropertySize::CreateEditControl(const CRect& rect, HFONT hFont) const
{
	CMFCMaskedEdit* pWndEdit = CProperty::CreateEditControl(rect, hFont);

	pWndEdit->SetValidChars(_T("0123456789 KkMmGgBb"));

	return pWndEdit;
}


// CPropertyColor
//

#define GUTTER     2

CIcons CPropertyColor::m_ColorDots;

CPropertyColor::CPropertyColor(LFVariantData* pVData, CWnd* pOwnerWnd)
	: CProperty(pVData, pOwnerWnd)
{
	ASSERT(pVData);
	ASSERT(pVData->Type==LFTypeColor);

	LFGetApp()->LoadColorDots(m_ColorDots, LFGetApp()->m_DialogFont);
}

INT CPropertyColor::GetMinWidth() const
{
	return 7*m_ColorDots.GetIconSize()+6*GUTTER;
}

void CPropertyColor::DrawValue(CDC& dc, LPCRECT lpcRect) const
{
	const INT Size = m_ColorDots.GetIconSize();

	INT x = lpcRect->left;
	INT y = (lpcRect->top+lpcRect->bottom-Size)/2;

	for (UINT a=1; a<LFItemColorCount; a++)
	{
		const BOOL Enabled = (p_VData->ColorSet & (1 << a));

		m_ColorDots.Draw(dc, x, y, Enabled ? a-1 : 2, FALSE, !Enabled);
		x += Size+GUTTER;
	}
}

HCURSOR CPropertyColor::SetCursor(INT x) const
{
	const INT Size = m_ColorDots.GetIconSize();

	return LFGetApp()->LoadStandardCursor(((x<7*Size+6*GUTTER) && (x%(Size+GUTTER)<Size)) ? IDC_HAND : IDC_ARROW);
}

BOOL CPropertyColor::CanDelete() const
{
	return (p_VData->ColorSet>1);
}

BOOL CPropertyColor::WantsChars() const
{
	return TRUE;
}

BOOL CPropertyColor::OnClickValue(INT x) const
{
	const INT Size = m_ColorDots.GetIconSize();

	if ((x>=0) && (x<7*Size+6*GUTTER))
		if (x%(Size+GUTTER)<Size)
			OnSetColor((BYTE)(1+x/(Size+GUTTER)));

	return FALSE;
}

BOOL CPropertyColor::OnPushChar(UINT nChar) const
{
	if ((nChar>=L'0') && (nChar<=L'7'))
		OnSetColor((BYTE)(nChar-L'0'));

	return TRUE;
}

void CPropertyColor::OnSetColor(BYTE Color) const
{
	assert(Color<LFItemColorCount);

	if (p_VData->Color!=Color)
	{
		p_VData->Color = Color;
		p_VData->ColorSet = (1 << Color);
		p_VData->IsNull = FALSE;

		NotifyOwner();
	}
}


// CPropertyGeoCoordinates
//

CPropertyGeoCoordinates::CPropertyGeoCoordinates(LFVariantData* pVData, CWnd* pOwnerWnd)
	: CProperty(pVData, pOwnerWnd)
{
	ASSERT(pVData);
	ASSERT(pVData->Type==LFTypeGeoCoordinates);
}

BOOL CPropertyGeoCoordinates::HasButton() const
{
	return TRUE;
}

HCURSOR CPropertyGeoCoordinates::SetCursor(INT /*x*/) const
{
	return LFGetApp()->LoadStandardCursor(IDC_ARROW);
}

BOOL CPropertyGeoCoordinates::OnClickValue(INT /*x*/) const
{
	return FALSE;
}

void CPropertyGeoCoordinates::OnClickButton(const STOREID& /*StoreID*/)
{
	LFSelectLocationGPSDlg dlg(p_VData->GeoCoordinates, p_OwnerWnd);
	if (dlg.DoModal()==IDOK)
	{
		p_VData->GeoCoordinates = dlg.m_Location;
		p_VData->IsNull = FALSE;

		NotifyOwner();
	}
}


// CPropertyTime
//

CPropertyTime::CPropertyTime(LFVariantData* pVData, CWnd* pOwnerWnd)
	: CProperty(pVData, pOwnerWnd)
{
	ASSERT(pVData);
	ASSERT(pVData->Type==LFTypeTime);
}

BOOL CPropertyTime::HasButton() const
{
	return TRUE;
}

HCURSOR CPropertyTime::SetCursor(INT /*x*/) const
{
	return LFGetApp()->LoadStandardCursor(IDC_ARROW);
}

BOOL CPropertyTime::OnClickValue(INT /*x*/) const
{
	return FALSE;
}

void CPropertyTime::OnClickButton(const STOREID& /*StoreID*/)
{
	if (LFEditTimeDlg(p_VData, p_OwnerWnd).DoModal()==IDOK)
		NotifyOwner();
}


// CPropertyDuration
//

CPropertyDuration::CPropertyDuration(LFVariantData* pData, CWnd* pOwnerWnd)
	: CPropertyNumber(pData, pOwnerWnd)
{
}

CMFCMaskedEdit* CPropertyDuration::CreateEditControl(const CRect& rect, HFONT hFont) const
{
	CMFCMaskedEdit* pWndEdit = CPropertyNumber::CreateEditControl(rect, hFont);

	pWndEdit->EnableMask(_T("DD DD DD"), _T("__:__:__"), _T('0'), _T("0123456789"));
	pWndEdit->EnableGetMaskedCharsOnly(FALSE);
	pWndEdit->SetWindowText(_T("00:00:00"));

	return pWndEdit;
}


// CPropertyGenre
//

CPropertyGenre::CPropertyGenre(LFVariantData* pVData, CWnd* pOwnerWnd)
	: CProperty(pVData, pOwnerWnd)
{
	ASSERT(pVData);
	ASSERT(pVData->Type==LFTypeGenre);
}

BOOL CPropertyGenre::HasButton() const
{
	return TRUE;
}

BOOL CPropertyGenre::OnClickValue(INT /*x*/) const
{
	return FALSE;
}

void CPropertyGenre::OnClickButton(const STOREID& StoreID)
{
	LFEditGenreDlg dlg(m_Multiple ? 0 : p_VData->Genre, StoreID, p_OwnerWnd);
	if (dlg.DoModal()==IDOK)
	{
		p_VData->UINT32 = dlg.m_Genre;
		p_VData->IsNull = FALSE;

		NotifyOwner();
	}
}


// CInspectorGrid
//

#define BORDER         2
#define MARGIN         2

#define NOPART         0
#define PARTLABEL      1
#define PARTVALUE      2
#define PARTBUTTON     3
#define PARTRESET      4

HICON CInspectorGrid::hIconResetNormal = NULL;
HICON CInspectorGrid::hIconResetSelected = NULL;
HICON CInspectorGrid::hIconResetHot = NULL;
HICON CInspectorGrid::hIconResetPressed = NULL;

CInspectorGrid::CInspectorGrid()
	: CFrontstageScroller(FRONTSTAGE_COMPLEXBACKGROUND)
{
	DEFAULTSTOREID(m_StoreID);
	m_SortAlphabetic = m_PartPressed = FALSE;
	m_pSortArray = NULL;
	m_pHeader = NULL;
	m_pWndEdit = NULL;
	m_IconSize = m_Context = m_ContextMenuID = 0;
	m_SelectedItem = m_EditItem = -1;
	m_HoverPart = NOPART;
}

BOOL CInspectorGrid::Create(CWnd* pOwnerWndWnd, UINT nID, UINT ContextMenuID, CInspectorHeader* pHeader)
{
	m_ContextMenuID = ContextMenuID;
	m_pHeader = pHeader;

	const CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	return CFrontstageScroller::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP, CRect(0, 0, 0, 0), pOwnerWndWnd, nID);
}

BOOL CInspectorGrid::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (m_pWndEdit)
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
		if (m_pWndEdit)
			return TRUE;

		break;
	}

	return CFrontstageScroller::PreTranslateMessage(pMsg);
}

void CInspectorGrid::SetPropertyName(Property& Prop, LPCWSTR pszName)
{
	ASSERT(pszName);

	// Set property name
	wcscpy_s(Prop.Name, LFAttributeNameSize, pszName);

	// Update label width
	Prop.LabelWidth = LFGetApp()->m_DialogFont.GetTextExtent(CString(pszName)+_T(":")).cx;
}

void CInspectorGrid::AddProperty(LFVariantData* pVData, LPCWSTR pszName, UINT CategoryID, BOOL Editable, BOOL Visible)
{
	ASSERT(pVData);
	ASSERT(pszName);

	// Property structure
	Property Prop;
	Prop.pProperty = CProperty::CreateProperty(pVData, this);
	Prop.CategoryID = CategoryID;
	Prop.Editable = Editable;
	Prop.Visible = Visible;
	Prop.Top = Prop.Bottom = -1;

	// Set property name
	SetPropertyName(Prop, pszName);

	// Add property to grid
	m_Properties.AddItem(Prop);
}

void CInspectorGrid::AddAttributeProperties(LFVariantData* pVDataArray, SIZE_T ItemSize)
{
	ASSERT(pVDataArray);

	// For byte granular addressing
	LPBYTE pVData = (LPBYTE)pVDataArray;

	// Iterate attributes
	for (UINT a=0; a<LFAttributeCount; a++)
		AddProperty((LFVariantData*)(pVData+a*ItemSize),
			LFGetApp()->GetAttributeName(a, m_Context),
			LFGetApp()->m_Attributes[a].AttrProperties.Category,
			LFGetApp()->IsAttributeEditable(a),
			LFGetApp()->IsAttributeEditable(a) && (a!=LFAttrFileName) && (a!=LFAttrPriority) && (a!=LFAttrDueTime));

	// Add pointers to location name and GPS coordinates to IATA code property
	((CPropertyIATA*)m_Properties[LFAttrLocationIATA].pProperty)->SetAdditionalVData((LFVariantData*)(pVData+LFAttrLocationName*ItemSize), (LFVariantData*)(pVData+LFAttrLocationGPS*ItemSize));
}

void CInspectorGrid::SortProperties()
{
	DestroyEdit();

	// If neccessary, delete sorting of property items (to be recreated when layout is adjusted)
	if (m_pSortArray)
	{
		delete m_pSortArray;
		m_pSortArray = NULL;
	}

	AdjustLayout();
	EnsureVisible(m_SelectedItem);
}

void CInspectorGrid::SetAlphabeticMode(BOOL SortAlphabetic)
{
	// Set alphabetic mode
	m_SortAlphabetic = SortAlphabetic;

	SortProperties();
}

void CInspectorGrid::SetContext(UINT Context)
{
	// Set new context
	m_Context = Context;

	// Update property names
	for (UINT a=0; a<m_Properties.m_ItemCount; a++)
	{
		const UINT Attr = m_Properties[a].pProperty->GetVData()->Attr;

		if (Attr<LFAttributeCount)
			SetPropertyName(m_Properties[a], LFGetApp()->GetAttributeName(Attr, m_Context));
	}

	SortProperties();
}

void CInspectorGrid::UpdatePropertyState(UINT Index, BOOL Multiple, BOOL Editable, BOOL Visible, const LFVariantData* pVDataRange)
{
	ASSERT(Index<m_Properties.m_ItemCount);

	DestroyEdit();

	m_Properties[Index].Editable = Editable;
	m_Properties[Index].Visible = Visible;
	m_Properties[Index].pProperty->UpdateState(Multiple, pVDataRange);
}

RECT CInspectorGrid::GetItemRect(INT Index) const
{
	RECT rect = { 0, 0, 0, 0 };

	if ((Index>=0) && (Index<(INT)m_Properties.m_ItemCount))
	{
		GetClientRect(&rect);

		rect.top = m_Properties[Index].Top;
		rect.bottom = m_Properties[Index].Bottom;
		rect.right -= MARGIN;

		OffsetRect(&rect, 0, -m_VScrollPos);
	}

	return rect;
}

UINT CInspectorGrid::PartAtPosition(const CPoint& point, INT Index) const
{
	if ((Index>=0) && (Index<(INT)m_Properties.m_ItemCount))
	{
		const RECT rect = GetItemRect(Index);

		// Label
		if (point.x<m_LabelWidth+BORDER)
			return PARTLABEL;

		const Property* pProp = &m_Properties[Index];
		CRect rectPart(m_LabelWidth+BORDER, pProp->Top-m_VScrollPos, rect.right+1, pProp->Bottom-m_VScrollPos);

		// Reset button
		if (pProp->Editable && pProp->pProperty->CanDelete() && (Index!=m_EditItem))
		{
			const INT Offs = (m_ItemHeight-m_IconSize)/2;
			CRect rectReset(rectPart.right-m_IconSize-Offs-1, rectPart.top+Offs, rectPart.right-Offs-1, rectPart.top+Offs+m_IconSize);

			if (rectReset.PtInRect(point))
				return PARTRESET;

			rectPart.right -= m_IconSize+Offs+BORDER+2;
		}
		else
		{
			rectPart.right -= BORDER;
		}

		// Button
		if (pProp->Editable && pProp->pProperty->HasButton() && (Index!=m_EditItem))
		{
			CRect rectButton(rectPart);
			rectButton.left = rectButton.right-rectButton.Height()-m_IconSize/2;

			if (rectButton.PtInRect(point))
				return PARTBUTTON;
		}

		// Value
		return PARTVALUE;
	}

	return NOPART;
}

INT CInspectorGrid::ItemAtPosition(CPoint point) const
{
	for (UINT a=0; a<m_Properties.m_ItemCount; a++)
	{
		const RECT rect = GetItemRect(a);

		if (PtInRect(&rect, point))
			return a;
	}

	return -1;
}

void CInspectorGrid::InvalidateItem(INT Index)
{
	const RECT rect = GetItemRect(Index);

	InvalidateRect(&rect);
}

void CInspectorGrid::EnsureVisible(INT Index)
{
	if (Index==-1)
		return;

	CRect rect;
	GetClientRect(rect);

	if (!rect.Height())
		return;

	const RECT rectItem = GetItemRect(Index);

	// Vertical
	INT nInc = 0;

	if (rectItem.bottom>rect.Height())
		nInc = rectItem.bottom-rect.Height();

	if (rectItem.top<nInc)
		nInc = rectItem.top;

	if ((nInc=max(-m_VScrollPos, min(nInc, m_VScrollMax-m_VScrollPos)))!=0)
	{
		m_VScrollPos += nInc;
		ScrollWindow(0, -nInc);
		SetScrollPos(SB_VERT, m_VScrollPos);
	}
}

void CInspectorGrid::SelectItem(INT Index)
{
	if ((Index==m_SelectedItem) || (Index==-1))
		return;

	if (!m_Properties[Index].Visible)
		return;

	InvalidateItem(m_SelectedItem);
	m_SelectedItem = Index;
	EnsureVisible(Index);
	InvalidateItem(Index);

	ReleaseCapture();
}

void CInspectorGrid::ShowTooltip(const CPoint& point)
{
	ASSERT(m_HoverItem>=0);

	if (!m_pWndEdit)
	{
		const Property* pProp = &m_Properties[m_HoverItem];

		WCHAR tmpStr[256];
		pProp->pProperty->ToString(tmpStr, 256);

		LFGetApp()->ShowTooltip(this, point, pProp->Name, tmpStr, (m_HoverItem<LFAttributeCount) ? LFGetApp()->m_CoreImageListExtraLarge.ExtractIcon(LFGetApp()->GetAttributeIcon(m_HoverItem, m_Context)-1) : NULL);
	}
}

BOOL CInspectorGrid::GetContextMenu(CMenu& Menu, INT /*Index*/)
{
	if (m_ContextMenuID)
		Menu.LoadMenu(m_ContextMenuID);

	return FALSE;
}

INT CInspectorGrid::Compare(INT Eins, INT Zwei) const
{
	const Property* pEins = &m_Properties[m_pSortArray[Eins]];
	const Property* pZwei = &m_Properties[m_pSortArray[Zwei]];

	return m_SortAlphabetic ? wcscmp(pEins->Name, pZwei->Name) : (pEins->CategoryID!=pZwei->CategoryID) ? (INT)pEins->CategoryID-(INT)pZwei->CategoryID : m_pSortArray[Eins]-m_pSortArray[Zwei];
}

void CInspectorGrid::Heap(INT Element, INT Count)
{
	while (Element<=Count/2-1)
	{
		INT Index = (Element+1)*2-1;
		if (Index+1<Count)
			if (Compare(Index, Index+1)<0)
				Index++;

		if (Compare(Element, Index)<0)
		{
			const INT Temp = m_pSortArray[Element];
			m_pSortArray[Element] = m_pSortArray[Index];
			m_pSortArray[Index] = Temp;

			Element = Index;
		}
		else
		{
			break;
		}
	}
}

void CInspectorGrid::CreateSortArray()
{
	if (m_pSortArray)
		return;

	ASSERT(m_Properties.m_ItemCount);
	m_pSortArray = new INT[m_Properties.m_ItemCount];

	// Canonical sort
	for (UINT a=0; a<m_Properties.m_ItemCount; a++)
		m_pSortArray[a] = a;

	// Heapsort with compare function
	if (m_Properties.m_ItemCount>1)
	{
		for (INT a=m_Properties.m_ItemCount/2-1; a>=0; a--)
			Heap(a, m_Properties.m_ItemCount);

		for (INT a=m_Properties.m_ItemCount-1; a>0; a--)
		{
			const INT Temp = m_pSortArray[0];
			m_pSortArray[0] = m_pSortArray[a];
			m_pSortArray[a] = Temp;

			Heap(0, a);
		}
	}
}

void CInspectorGrid::AdjustLayout()
{
	if (!m_Properties.m_ItemCount)
		return;

	// Sort array
	CreateSortArray();
	ASSERT(m_pSortArray);

	// Reset categories
	for (UINT a=0; a<LFAttrCategoryCount; a++)
		m_Categories[a].Top = m_Categories[a].Bottom = -1;

	// Place categories and  properties
	INT LabelWidth = m_LabelWidth = m_MinWidth = 0;
	m_ScrollHeight = m_pHeader ? m_pHeader->GetPreferredHeight()+1 : MARGIN+1;
	UINT CategoryID = (UINT)-1;

	for (UINT a=0; a<m_Properties.m_ItemCount; a++)
	{
		Property* pProp = &m_Properties[m_pSortArray[a]];

		// Category
		if (!m_SortAlphabetic && pProp->Visible && (pProp->CategoryID!=CategoryID))
		{
			CategoryID = pProp->CategoryID;

			const INT Spacer = (m_ScrollHeight==1) ? -MARGIN : 8;

			m_Categories[CategoryID].Top = m_ScrollHeight+Spacer;
			m_Categories[CategoryID].Bottom = m_ScrollHeight+LFGetApp()->m_LargeFont.GetFontHeight()+2*LFCATEGORYPADDING+Spacer;

			m_ScrollHeight += LFGetApp()->m_LargeFont.GetFontHeight()+2*MARGIN+Spacer+1;
		}

		// Label width
		if (pProp->LabelWidth>LabelWidth)
			LabelWidth = pProp->LabelWidth;

		if (pProp->Visible)
		{
			pProp->Top = m_ScrollHeight;
			pProp->Bottom = m_ScrollHeight+18;

			if (pProp->LabelWidth>m_LabelWidth)
				m_LabelWidth = pProp->LabelWidth;

			m_ScrollHeight += m_szScrollStep.cy;
		}
		else
		{
			pProp->Top = pProp->Bottom = -1;
		}

		// Data width
		const INT DataWidth = pProp->pProperty->GetMinWidth();

		if (DataWidth>m_MinWidth)
			m_MinWidth = DataWidth;
	}

	// Bottom margin
	m_ScrollHeight += MARGIN+2;

	// Minimum width of control
	LabelWidth += 2*BORDER;
	m_LabelWidth += 2*BORDER;
	m_MinWidth += LabelWidth+m_IconSize+(m_ItemHeight-m_IconSize)/2+BORDER+4;

	if (m_SelectedItem==-1)
		m_SelectedItem = 0;

	// Select new property when current property is invisible
	if (!m_Properties[m_SelectedItem].Visible)
	{
		m_SelectedItem = -1;

		for (UINT a=0; a<m_Properties.m_ItemCount; a++)
			if (m_Properties[m_pSortArray[a]].Visible)
			{
				m_SelectedItem = m_pSortArray[a];
				break;
			}
	}

	CFrontstageScroller::AdjustLayout();
}

void CInspectorGrid::DrawStage(CDC& dc, Graphics& g, const CRect& rect, const CRect& /*rectUpdate*/, BOOL Themed)
{
	//Header
	if (m_pHeader)
	{
		CRect rectHeader(0, -m_VScrollPos, rect.Width(), m_pHeader->GetPreferredHeight()-m_VScrollPos);
		m_pHeader->DrawHeader(dc, g, rectHeader, Themed);
	}

	// Categories
	for (UINT a=0; a<LFAttrCategoryCount; a++)
		if (m_Categories[a].Top!=-1)
			DrawCategory(dc, CRect(0, m_Categories[a].Top-m_VScrollPos, rect.Width()-1, m_Categories[a].Bottom-m_VScrollPos), LFGetApp()->m_AttrCategoryNames[a], NULL, Themed);

	// Items
	CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DialogFont);

	for (UINT a=0; a<m_Properties.m_ItemCount; a++)
	{
		Property* pProp = &m_Properties[a];
		if (pProp->Visible)
		{
			const RECT rectProp = GetItemRect(a);
			BOOL Selected = ((INT)a==m_SelectedItem) && (GetFocus()==this);

			if ((INT)a!=m_EditItem)
			{
				DrawListItemBackground(dc, &rectProp, Themed, GetFocus()==this, (INT)a==m_HoverItem, Selected, Selected);
				DrawListItemForeground(dc, &rectProp, Themed, GetFocus()==this, (INT)a==m_HoverItem, Selected, Selected);
			}
			else
			{
				dc.SetTextColor(Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));
			}

			COLORREF clr = dc.SetTextColor(Selected ? dc.GetTextColor() : pProp->Editable ? Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT) : GetSysColor(COLOR_3DSHADOW));

			// Label
			CRect rectValue(BORDER, pProp->Top-m_VScrollPos, m_LabelWidth-BORDER, pProp->Bottom-m_VScrollPos);
			dc.DrawText(CString(pProp->Name)+_T(":"), rectValue, DT_RIGHT | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);

			rectValue.left = rectValue.right+2*BORDER;
			rectValue.right = rect.Width();

			// Delete button
			if (pProp->Editable && pProp->pProperty->CanDelete())
			{
				const INT Offs = (m_ItemHeight-m_IconSize)/2;
				DrawIconEx(dc, rectValue.right-m_IconSize-Offs-2, rectValue.top+Offs, ((INT)a==m_HoverItem) && (m_HoverPart==PARTRESET) ? m_PartPressed ? hIconResetPressed : hIconResetHot : Selected ? hIconResetSelected : hIconResetNormal, m_IconSize, m_IconSize, 0, NULL, DI_NORMAL);

				rectValue.right -= m_IconSize+Offs+BORDER+2;
			}
			else
			{
				rectValue.right -= BORDER-1;
			}

			// Button
			if (pProp->Editable && pProp->pProperty->HasButton() && ((INT)a!=m_EditItem))
			{
				CRect rectButton(rectValue);
				rectButton.right -= BORDER-1;
				rectButton.left = rectButton.right-rectButton.Height()-m_IconSize/2;
				rectValue.right -= rectButton.Width()+2*BORDER;

				if (!Themed)
					rectButton.DeflateRect(1, 1);

				const BOOL Hover = ((INT)a==m_HoverItem) && (m_HoverPart==PARTBUTTON);
				const BOOL Pressed = Hover && m_PartPressed;

				DrawWhiteButtonBackground(dc, g, rectButton, Themed, FALSE, Pressed, Hover, FALSE, TRUE);

				if (Pressed)
					rectButton.OffsetRect(1, 1);

				dc.DrawText(_T("..."), rectButton, DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
			}

			// Value
			dc.SetTextColor(clr);
			pProp->pProperty->DrawValue(dc, rectValue);
		}
	}

	dc.SelectObject(pOldFont);

	// Shadow
	if (m_pHeader && Themed)
		CTaskbar::DrawTaskbarShadow(g, rect);
}

void CInspectorGrid::ResetProperty(INT Index)
{
	ASSERT(Index>=0);
	ASSERT(Index<(INT)m_Properties.m_ItemCount);

	if (m_Properties[Index].Editable && (Index!=LFAttrFileName))
	{
		LFClearVariantData(*m_Properties[Index].pProperty->GetVData());

		NotifyOwner(Index);
	}
}

void CInspectorGrid::EditProperty(INT Index)
{
	ASSERT(Index>=0);
	ASSERT(Index<(INT)m_Properties.m_ItemCount);

	m_EditItem = -1;

	Property* pProp = &m_Properties[Index];
	if (pProp->Editable)
	{
		// Create edit control
		if (pProp->pProperty->OnClickValue(-1))
		{
			EnsureVisible(m_EditItem=Index);
			InvalidateItem(Index);

			RECT rect = GetItemRect(Index);
			rect.top += 2;
			rect.bottom -=2;
			rect.left += m_LabelWidth+BORDER-1;

			ASSERT(!m_pWndEdit);
			m_pWndEdit = pProp->pProperty->CreateEditControl(rect, LFGetApp()->m_DialogBoldFont);
			m_pWndEdit->SetFocus();

			return;
		}

		if (pProp->pProperty->HasButton())
			pProp->pProperty->OnClickButton(m_StoreID);
	}
}

void CInspectorGrid::ModifyProperty(INT Index)
{
	if (Index!=-1)
	{
		ASSERT(m_Properties[Index].pProperty->GetVData()->Attr==(UINT)Index);

		m_Properties[Index].pProperty->SetModified();
		InvalidateItem(Index);
	}
}

void CInspectorGrid::DestroyEdit(BOOL Accept)
{
	if (m_pWndEdit)
	{
		const INT EditItem = m_EditItem;

		// Set m_pWndEdit to NULL to avoid recursive calls when the edit window loses focus
		CEdit* pVictim = m_pWndEdit;
		m_pWndEdit = NULL;

		// Get text
		CString Value;
		pVictim->GetWindowText(Value);

		// Destroy window; this will trigger another DestroyEdit() call!
		pVictim->DestroyWindow();
		delete pVictim;

		if (Accept && (EditItem!=-1))
			m_Properties[EditItem].pProperty->OnSetString(Value);

		// Redraw whole item
		InvalidateItem(m_EditItem);
	}

	m_EditItem = -1;
}


BEGIN_MESSAGE_MAP(CInspectorGrid, CFrontstageScroller)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_KEYDOWN()
	ON_WM_SETCURSOR()
	ON_WM_GETDLGCODE()

	ON_MESSAGE(WM_PROPERTYCHANGED, OnPropertyChanged)

	ON_EN_KILLFOCUS(2, OnDestroyEdit)
END_MESSAGE_MAP()

INT CInspectorGrid::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstageScroller::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Item height
	CFrontstageScroller::SetItemHeight(max(LFGetApp()->m_DialogFont.GetFontHeight(), 16)+2);

	// Icons
	m_IconSize = (m_ItemHeight>=27) ? 25 : (m_ItemHeight>=22) ? 20 : (m_ItemHeight>=18) ? 16 : 14;

	if (!hIconResetNormal)
		hIconResetNormal = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_RESET_NORMAL), IMAGE_ICON, m_IconSize, m_IconSize, LR_SHARED);

	if (!hIconResetSelected)
		hIconResetSelected = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_RESET_SELECTED), IMAGE_ICON, m_IconSize, m_IconSize, LR_SHARED);

	if (!hIconResetHot)
		hIconResetHot = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_RESET_HOT), IMAGE_ICON, m_IconSize, m_IconSize, LR_SHARED);

	if (!hIconResetPressed)
		hIconResetPressed = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_RESET_PRESSED), IMAGE_ICON, m_IconSize, m_IconSize, LR_SHARED);

	return 0;
}

void CInspectorGrid::OnDestroy()
{
	DestroyEdit();

	for (UINT a=0; a<m_Properties.m_ItemCount; a++)
		delete m_Properties[a].pProperty;

	delete m_pSortArray;

	CFrontstageScroller::OnDestroy();
}

void CInspectorGrid::OnSize(UINT nType, INT cx, INT cy)
{
	CFrontstageScroller::OnSize(nType, cx, cy);

	// Adjust size of edit control
	if (m_pWndEdit)
	{
		CRect rect;
		m_pWndEdit->GetWindowRect(rect);
		ScreenToClient(rect);

		rect.left = m_LabelWidth+BORDER;
		rect.right = cx-1;
		m_pWndEdit->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

void CInspectorGrid::OnMouseMove(UINT nFlags, CPoint point)
{
	const BOOL Dragging = (GetCapture()==this);

	if (Dragging)
	{
		const UINT Part = PartAtPosition(point, m_SelectedItem);

		BOOL PartPressed = (Part==m_HoverPart);
		if (m_PartPressed!=PartPressed)
		{
			m_PartPressed = PartPressed;
			InvalidateItem(m_SelectedItem);
		}
	}
	else
	{
		CFrontstageWnd::OnMouseMove(nFlags, point);

		const UINT Part = PartAtPosition(point, m_HoverItem);
		if (m_HoverPart!=Part)
		{
			m_HoverPart = Part;
			InvalidateItem(m_HoverItem);
		}
	}
}

void CInspectorGrid::OnMouseLeave()
{
	m_HoverPart = NOPART;

	CFrontstageWnd::OnMouseLeave();
}

void CInspectorGrid::OnLButtonDown(UINT nFlags, CPoint point)
{
	const INT Index = ItemAtPosition(point);
	if (Index!=-1)
	{
		SelectItem(Index);

		const UINT Part = PartAtPosition(point, Index);
		if (Part>=PARTBUTTON)
		{
			m_HoverPart = Part;
			m_PartPressed = TRUE;

			SetCapture();

			InvalidateItem(m_SelectedItem);
		}
	}

	CFrontstageScroller::OnLButtonDown(nFlags, point);
}

void CInspectorGrid::OnLButtonUp(UINT nFlags, CPoint point)
{
	const INT Index = ItemAtPosition(point);
	const UINT Part = PartAtPosition(point, Index);

	if (GetCapture()==this)
	{
		m_PartPressed = FALSE;
		ReleaseCapture();
		InvalidateItem(Index);

		if ((Index==m_SelectedItem) && (Index!=-1))
			if (Part==PARTRESET)
			{
				BOOL DoReset = (nFlags & MK_CONTROL) || !m_pHeader;

				if (!DoReset)
					DoReset = (LFMessageBox(this, CString((LPCSTR)IDS_DELETEPROPERTY_MSG), CString((LPCSTR)IDS_DELETEPROPERTY_CAPTION), MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING)==IDYES);

				if (DoReset)
					ResetProperty(Index);
			}
			else
				if (Part==PARTBUTTON)
				{
					m_Properties[Index].pProperty->OnClickButton(m_StoreID);
				}
	}
	else
		if ((Index==m_SelectedItem) && (Index!=-1) && (Part==PARTVALUE))
		{
			const Property* pProp = &m_Properties[Index];

			if (pProp->Editable && pProp->pProperty->OnClickValue(point.x-m_LabelWidth-BORDER))
				EditProperty(Index);
		}
}

void CInspectorGrid::OnLButtonDblClk(UINT /*nFlags*/, CPoint point)
{
	m_PartPressed = FALSE;
	ReleaseCapture();

	const INT Index = ItemAtPosition(point);
	if ((Index!=-1) && (PartAtPosition(point, Index)<=PARTBUTTON))
	{
		SelectItem(Index);
		EditProperty(Index);
	}
}

void CInspectorGrid::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	INT Last = m_SelectedItem;

	switch (nChar)
	{
	case VK_HOME:
		for (UINT a=0; a<m_Properties.m_ItemCount; a++)
			if (m_Properties[m_pSortArray[a]].Visible)
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
			if (m_Properties[m_pSortArray[a]].Visible)
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
			if (m_Properties[m_pSortArray[a]].Visible)
				Last = m_pSortArray[a];
		}

		break;

	case VK_END:
		for (INT a=m_Properties.m_ItemCount-1; a>=0; a--)
			if (m_Properties[m_pSortArray[a]].Visible)
			{
				SelectItem(m_pSortArray[a]);
				break;
			}

		break;

	case VK_EXECUTE:
	case VK_RETURN:
		if ((GetKeyState(VK_CONTROL)<0) && (m_SelectedItem!=-1))
			if (m_Properties[m_SelectedItem].pProperty->HasButton())
			{
				m_Properties[m_SelectedItem].pProperty->OnClickButton(m_StoreID);
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
			const Property* pProp = &m_Properties[m_SelectedItem];

			if (pProp->Editable && pProp->pProperty->WantsChars())
				if (m_Properties[m_SelectedItem].pProperty->OnPushChar(nChar))
					break;
		}

		CFrontstageScroller::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

BOOL CInspectorGrid::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*Message*/)
{
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	const INT Index = ItemAtPosition(point);
	if ((Index!=-1) && (PartAtPosition(point, Index)==PARTVALUE))
	{
		const Property* pProp = &m_Properties[Index];

		if (pProp->Editable)
		{
			SetCursor(pProp->pProperty->SetCursor(point.x-m_LabelWidth-BORDER));

			return TRUE;
		}
	}

	SetCursor(LFGetApp()->LoadStandardCursor(IDC_ARROW));

	return TRUE;
}


LRESULT CInspectorGrid::OnPropertyChanged(WPARAM wParam, LPARAM lParam)
{
	ModifyProperty((SHORT)LOWORD(wParam));
	ModifyProperty((SHORT)LOWORD(lParam));
	ModifyProperty((SHORT)HIWORD(lParam));

	UpdateWindow();

	return GetOwner()->PostMessage(WM_PROPERTYCHANGED, wParam, lParam);
}

UINT CInspectorGrid::OnGetDlgCode()
{
	return DLGC_WANTARROWS | DLGC_WANTCHARS | DLGC_WANTALLKEYS;
}


void CInspectorGrid::OnDestroyEdit()
{
	DestroyEdit(TRUE);
}
