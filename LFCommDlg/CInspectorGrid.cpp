
// CInspectorGrid.cpp: Implementierung der Klasse CInspectorGrid
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFPickGenreDlg.h"
#include "LFPickHashtagsDlg.h"
#include "LFPickStringDlg.h"
#include "LFPickTimeDlg.h"


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

CMFCMaskedEdit* CProperty::CreateEditControl(const CRect& rect) const
{
	CMFCMaskedEdit* pWndEdit = new CMFCMaskedEdit();
	pWndEdit->Create(WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ES_AUTOHSCROLL, rect, p_OwnerWnd, 2);

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

void CProperty::OnClickButton(ITEMCONTEXT Context)
{
	ASSERT(p_VData->Type==LFTypeUnicodeString);

	LFPickStringDlg dlg(p_VData->Attr, Context, m_Multiple ? _T("") : p_VData->UnicodeString, p_OwnerWnd);
	if (dlg.DoModal()==IDOK)
	{
		wcsncpy_s(p_VData->UnicodeString, LFGetApp()->m_Attributes[p_VData->Attr].AttrProperties.cCharacters, dlg.m_UnicodeString, _TRUNCATE);
		p_VData->IsNull = FALSE;

		NotifyOwner();
	}

}

BOOL CProperty::CanDelete() const
{
	return (p_VData->Attr!=LFAttrFileName) && (!LFIsNullVariantData(*p_VData) || m_Multiple);
}

BOOL CProperty::WantsChars() const
{
	return FALSE;
}
	
void CProperty::UpdateState(BOOL Multiple, const LFVariantData* pVDataRange)
{
	const UINT Type = p_VData->Type;

	m_ShowRange = ((m_Multiple=Multiple)==TRUE) && (LFCompareVariantData(*p_VData, *pVDataRange)!=0) &&
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

void CPropertyHashtags::OnClickButton(ITEMCONTEXT Context)
{
	LFPickHashtagsDlg dlg(p_VData->Attr, Context, m_Multiple ? _T("") : p_VData->UnicodeArray, p_OwnerWnd);
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

CMFCMaskedEdit* CPropertyIATA::CreateEditControl(const CRect& rect) const
{
	CMFCMaskedEdit* pWndEdit = CProperty::CreateEditControl(rect);

	pWndEdit->SetValidChars(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

	return pWndEdit;
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

void CPropertyIATA::OnClickButton(ITEMCONTEXT /*Context*/)
{
	LFSelectPropertyIATADlg dlg(p_OwnerWnd, &p_VData->AnsiString[0], p_VDataLocationName!=NULL, p_VDataLocationGPS!=NULL);
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

CMFCMaskedEdit* CPropertyNumber::CreateEditControl(const CRect& rect) const
{
	CMFCMaskedEdit* pWndEdit = CProperty::CreateEditControl(rect);

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

CMFCMaskedEdit* CPropertySize::CreateEditControl(const CRect& rect) const
{
	CMFCMaskedEdit* pWndEdit = CProperty::CreateEditControl(rect);

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
	const INT y = (lpcRect->top+lpcRect->bottom-Size)/2;

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
	{
		OnSetColor((BYTE)(nChar-L'0'));

		return TRUE;
	}

	return FALSE;
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

HCURSOR CPropertyGeoCoordinates::SetCursor(INT /*x*/) const
{
	return LFGetApp()->LoadStandardCursor(IDC_ARROW);
}

BOOL CPropertyGeoCoordinates::OnClickValue(INT /*x*/) const
{
	return FALSE;
}

void CPropertyGeoCoordinates::OnClickButton(ITEMCONTEXT /*Context*/)
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

HCURSOR CPropertyTime::SetCursor(INT /*x*/) const
{
	return LFGetApp()->LoadStandardCursor(IDC_ARROW);
}

BOOL CPropertyTime::OnClickValue(INT /*x*/) const
{
	return FALSE;
}

void CPropertyTime::OnClickButton(ITEMCONTEXT Context)
{
	if (LFPickTimeDlg(p_VData, Context, p_OwnerWnd).DoModal()==IDOK)
		NotifyOwner();
}


// CPropertyDuration
//

CPropertyDuration::CPropertyDuration(LFVariantData* pData, CWnd* pOwnerWnd)
	: CPropertyNumber(pData, pOwnerWnd)
{
}

CMFCMaskedEdit* CPropertyDuration::CreateEditControl(const CRect& rect) const
{
	CMFCMaskedEdit* pWndEdit = CPropertyNumber::CreateEditControl(rect);

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

BOOL CPropertyGenre::OnClickValue(INT /*x*/) const
{
	return FALSE;
}

void CPropertyGenre::OnClickButton(ITEMCONTEXT Context)
{
	LFPickGenreDlg dlg(p_VData->Attr, Context, m_Multiple ? 0 : p_VData->Genre, p_OwnerWnd);
	if (dlg.DoModal()==IDOK)
	{
		p_VData->UINT32 = dlg.m_Genre;
		p_VData->IsNull = FALSE;

		NotifyOwner();
	}
}


// CAttributePickDlg
//

CAttributePickDlg::CAttributePickDlg(ATTRIBUTE Attr, ITEMCONTEXT Context, UINT nIDTemplate, CWnd* pParentWnd)
	: LFDialog(nIDTemplate, pParentWnd)
{
	ASSERT(Attr<LFAttributeCount);
	ASSERT(Context<LFContextCount);

	m_Attr = Attr;
	m_Context = Context;
}

BOOL CAttributePickDlg::InitDialog()
{
	// Caption
	CString Caption;
	GetWindowText(Caption);

	if (Caption.IsEmpty())
		SetWindowText(LFGetApp()->GetAttributeName(m_Attr, m_Context));

	return TRUE;
}

LFSearchResult* CAttributePickDlg::RunQuery() const
{
	// Filter (slaves have to be included for tooltips)
	LFFilter* pFilter = LFAllocFilter();
	pFilter->Query.Context = LFGetApp()->IsAttributeTaxonomyPickGlobally(m_Attr) ? LFContextAllFiles : m_Context;

	// Query
	CWaitCursor WaitCursor;

	LFSearchResult* pRawFiles = LFQuery(pFilter);
	LFSearchResult* pCookedFiles = LFGroupSearchResult(pRawFiles, m_Attr, FALSE, TRUE, pFilter);

	LFFreeFilter(pFilter);
	LFFreeSearchResult(pRawFiles);

	return pCookedFiles;
}


// CInspectorGrid
//

#define PADDINGX     (ITEMCELLPADDINGX/2)
#define PADDINGY     ITEMCELLPADDINGY
#define MARGIN       2

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
	: CFrontstageItemView(FRONTSTAGE_COMPLEXBACKGROUND | FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLEFOCUSITEM | FRONTSTAGE_ENABLELABELEDIT | FRONTSTAGE_HIDESELECTIONONEDIT, sizeof(PropertyItemData))
{
	m_SortAlphabetic = m_PartPressed = FALSE;
	m_pHeader = NULL;
	m_IconSize = m_ContextMenuID = 0;
	m_Context = 0;
	m_HoverPart = NOPART;

	// Item cateogries
	for (UINT a=0; a<LFAttrCategoryCount; a++)
		AddItemCategory(LFGetApp()->m_AttrCategoryNames[a]);
}

BOOL CInspectorGrid::Create(CWnd* pOwnerWnd, UINT nID, UINT ContextMenuID, CInspectorHeader* pHeader)
{
	m_ContextMenuID = ContextMenuID;
	m_pHeader = pHeader;

	return CFrontstageItemView::Create(pOwnerWnd, nID);
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

	return CFrontstageItemView::PreTranslateMessage(pMsg);
}


// Menus

BOOL CInspectorGrid::GetContextMenu(CMenu& Menu, INT /*Index*/)
{
	if (m_ContextMenuID)
		Menu.LoadMenu(m_ContextMenuID);

	return FALSE;
}


// Layouts

INT CInspectorGrid::GetItemHeight() const
{
	return max(LFGetApp()->m_DialogFont.GetFontHeight(), 14)+2*PADDINGY;
}

void CInspectorGrid::AdjustLayout()
{
	if (m_FocusItem==-1)
		m_FocusItem = 0;

	// Minimum width of control
	INT LabelWidth = m_LabelWidth = m_MinWidth = 0;

	for (INT a=0; a<m_ItemCount; a++)
	{
		PropertyItemData* pData = GetPropertyItemData(a);

		// Label width
		if (pData->LabelWidth>LabelWidth)
			LabelWidth = pData->LabelWidth;

		// Visible label width
		if (pData->Hdr.Valid && pData->LabelWidth>m_LabelWidth)
			m_LabelWidth = pData->LabelWidth;

		// Data width
		const INT DataWidth = pData->pProperty->GetMinWidth();

		if (DataWidth>m_MinWidth)
			m_MinWidth = DataWidth;
	}

	const INT Margin = m_pHeader ? INSPECTORGRIDMARGIN : BACKSTAGEBORDER;
	m_MinWidth += 2*Margin+max(128, LabelWidth+m_IconSize+2*((m_ItemHeight-m_IconSize)/2)+4*PADDINGX)+GetSystemMetrics(SM_CXVSCROLL);

	// Set layout
	CFrontstageItemView::AdjustLayoutGrid(CSize(0, GetItemHeight()), TRUE, Margin, m_pHeader ? m_pHeader->GetPreferredHeight()+ITEMVIEWMARGINLARGE : 0, FALSE);
}


// Item categories

INT CInspectorGrid::GetItemCategory(INT Index) const
{
	return m_SortAlphabetic ? CFrontstageItemView::GetItemCategory(Index) : GetPropertyItemData(Index)->CategoryID;
}


// Item data

PropertyItemData* CInspectorGrid::GetPropertyItemDataForAttribute(ATTRIBUTE Attr) const
{
	for (INT a=0; a<m_ItemCount; a++)
	{
		PropertyItemData* pData = GetPropertyItemData(a);

		if (pData->Attr==Attr)
			return pData;
	}

	// This should never happen
	ASSERT(FALSE);

	return NULL;
}

void CInspectorGrid::SetPropertyName(PropertyItemData& Data, LPCWSTR pszName)
{
	ASSERT(pszName);

	// Set property name
	wcscpy_s(Data.Name, LFAttributeNameSize, pszName);

	// Update label width
	Data.LabelWidth = LFGetApp()->m_DialogFont.GetTextExtent(CString(pszName)+_T(":")).cx;
}

void CInspectorGrid::SetContext(ITEMCONTEXT Context)
{
	// Set new context
	m_Context = Context;

	// Update property names
	for (INT a=0; a<m_ItemCount; a++)
	{
		PropertyItemData* pData = GetPropertyItemData(a);

		if (pData->Attr<LFAttributeCount)
			SetPropertyName(*pData, LFGetApp()->GetAttributeName(pData->Attr, m_Context));
	}

	SortItems();
}

void CInspectorGrid::AddProperty(LFVariantData* pVData, LPCWSTR pszName, UINT CategoryID, BOOL Visible, BOOL Editable)
{
	ASSERT(pVData);
	ASSERT(pszName);

	PropertyItemData Data;

	Data.Hdr.Valid = Visible;
	Data.pProperty = CProperty::CreateProperty(pVData, this);
	Data.Attr = pVData->Attr;
	Data.CategoryID = CategoryID;
	Data.Editable = Editable;
	SetPropertyName(Data, pszName);

	AddItem(&Data);
}

void CInspectorGrid::AddAttributeProperties(LFVariantData* pVDataArray, SIZE_T ItemSize, UINT ItemCount)
{
	ASSERT(pVDataArray);
	ASSERT(ItemSize>=sizeof(LFVariantData));
	ASSERT(ItemCount>=LFAttributeCount);

	SetItemCount(ItemCount, FALSE);

	// For byte granular addressing
	LPBYTE pVData = (LPBYTE)pVDataArray;

	// Iterate attributes
	for (UINT a=0; a<LFAttributeCount; a++)
		AddProperty((LFVariantData*)(pVData+a*ItemSize),
			LFGetApp()->GetAttributeName(a, m_Context),
			LFGetApp()->m_Attributes[a].AttrProperties.Category,
			LFGetApp()->IsAttributeEditable(a) && (a!=LFAttrFileName) && (a!=LFAttrPriority) && (a!=LFAttrDueTime),
			LFGetApp()->IsAttributeEditable(a));

	// Add pointers to location name and GPS coordinates to IATA code property
	((CPropertyIATA*)GetPropertyItemData(LFAttrLocationIATA)->pProperty)->SetAdditionalVData((LFVariantData*)(pVData+LFAttrLocationName*ItemSize), (LFVariantData*)(pVData+LFAttrLocationGPS*ItemSize));
}

void CInspectorGrid::UpdatePropertyState(ATTRIBUTE Attr, BOOL Multiple, BOOL Editable, BOOL Visible, const LFVariantData* pVDataRange)
{
	DestroyEdit();

	// Find attribute item
	PropertyItemData* pData = GetPropertyItemDataForAttribute(Attr);

	pData->Hdr.Valid = Visible;
	pData->Editable = Editable;
	pData->pProperty->UpdateState(Multiple, pVDataRange);
}


// Item sort

INT CInspectorGrid::CompareItems(PropertyItemData* pData1, PropertyItemData* pData2, const SortParameters& Parameters)
{
	return Parameters.Parameter1 ? wcscmp(pData1->Name, pData2->Name) : (pData1->CategoryID!=pData2->CategoryID) ? (INT)pData1->CategoryID-(INT)pData2->CategoryID : (INT)pData1->Attr-(INT)pData2->Attr;
}

void CInspectorGrid::SortItems()
{
	DestroyEdit();

	CFrontstageItemView::SortItems((PFNCOMPARE)CompareItems, 0, FALSE, m_SortAlphabetic);
	AdjustLayout();
}

void CInspectorGrid::SetAlphabeticMode(BOOL SortAlphabetic)
{
	// Set alphabetic mode
	m_SortAlphabetic = SortAlphabetic;

	SortItems();
	EnsureVisible(m_FocusItem);
}


// Item handling

void CInspectorGrid::ShowTooltip(const CPoint& point)
{
	ASSERT(m_HoverItem>=0);

	if (!m_pWndEdit)
	{
		const PropertyItemData* pData = GetPropertyItemData(m_HoverItem);

		WCHAR tmpStr[256];
		pData->pProperty->ToString(tmpStr, 256);

		LFGetApp()->ShowTooltip(this, point, pData->Name, tmpStr, (m_HoverItem<LFAttributeCount) ? LFGetApp()->m_CoreImageListExtraLarge.ExtractIcon(LFGetApp()->GetAttributeIcon(m_HoverItem, m_Context)-1) : NULL);
	}
}

UINT CInspectorGrid::PartAtPosition(const CPoint& point, INT Index) const
{
	ASSERT(Index<m_ItemCount);

	if (Index==-1)
		return NOPART;

	const PropertyItemData* pData = GetPropertyItemData(Index);

	// Label
	CRect rectPart(pData->Hdr.Rect.left+PADDINGX, pData->Hdr.Rect.top-m_VScrollPos, pData->Hdr.Rect.left+m_LabelWidth+PADDINGX, pData->Hdr.Rect.bottom-m_VScrollPos);

	if (rectPart.PtInRect(point))
		return PARTLABEL;

	rectPart.left = rectPart.right+2*PADDINGX;
	rectPart.right = pData->Hdr.Rect.right;

	// Edit button
	if (pData->Editable && pData->pProperty->HasButton() && (!IsEditing() || (Index!=m_EditItem)))
	{
		CRect rectButton(rectPart);
		rectPart.right = (rectButton.left=rectButton.right-rectButton.Height()-m_IconSize/2)-PADDINGX;

		rectButton.DeflateRect(1, 1);

		if (rectButton.PtInRect(point))
			return PARTBUTTON;
	}

	// Reset button
	if (pData->Editable && pData->pProperty->CanDelete())
	{
		const INT Offs = (m_ItemHeight-m_IconSize)/2;
		
		if (CRect(rectPart.right-m_IconSize-Offs, rectPart.top+Offs, rectPart.right-Offs, rectPart.top+Offs+m_IconSize).PtInRect(point))
			return PARTRESET;

		rectPart.right -= m_IconSize+2*Offs+PADDINGX;
	}

	// Value
	if (rectPart.right==pData->Hdr.Rect.right)
		rectPart.right -= PADDINGX;

	return rectPart.PtInRect(point) ? PARTVALUE : NOPART;
}


// Selected item commands

void CInspectorGrid::ResetProperty(INT Index, BOOL Force)
{
	ASSERT(Index>=0);
	ASSERT(Index<(INT)m_ItemCount);

	const PropertyItemData* pData = GetPropertyItemData(GetSelectedItem());

	if (pData->Editable && (Index!=LFAttrFileName))
		if (Force || (LFMessageBox(this, CString((LPCSTR)IDS_DELETEPROPERTY_MSG), CString((LPCSTR)IDS_DELETEPROPERTY_CAPTION), MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING)==IDYES))
		{
			LFClearVariantData(*pData->pProperty->GetVData());

			NotifyOwner(pData->Attr);
		}
}

void CInspectorGrid::ModifyProperty(SHORT Attr)
{
	ASSERT(Attr<m_ItemCount);

	if (Attr>=0)
	{
		GetPropertyItemDataForAttribute(Attr)->pProperty->SetModified();

		Invalidate();
	}
}

void CInspectorGrid::FireSelectedItem()
{
	ASSERT(IsFocusItemEnabled());
	ASSERT(GetSelectedItem()>=0);

	const PropertyItemData* pData = GetPropertyItemData(GetSelectedItem());

	if (pData->Editable && pData->pProperty->HasButton())
		pData->pProperty->OnClickButton(m_Context);
}

void CInspectorGrid::DeleteSelectedItem()
{
	ASSERT(IsFocusItemEnabled());
	ASSERT(GetSelectedItem()>=0);

	ResetProperty(GetSelectedItem());
}


// Drawing

void CInspectorGrid::DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed)
{
	ASSERT(rectItem);

	const PropertyItemData* pData = GetPropertyItemData(Index);

	CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DialogFont);

	// Label
	const COLORREF clr = !pData->Editable ? SetLightTextColor(dc, Index, Themed) : dc.GetTextColor();

	CRect rectPart(rectItem->left+PADDINGX, rectItem->top, rectItem->left+m_LabelWidth+PADDINGX, rectItem->bottom);
	dc.DrawText(CString(pData->Name)+_T(":"), rectPart, DT_RIGHT | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);

	rectPart.left = rectPart.right+2*PADDINGX;
	rectPart.right = rectItem->right;

	// Edit button
	if (pData->Editable && pData->pProperty->HasButton() && (!IsEditing() || (Index!=m_EditItem)))
	{
		CRect rectButton(rectPart);
		rectPart.right = (rectButton.left=rectButton.right-rectButton.Height()-m_IconSize/2)-PADDINGX;

		if (!Themed)
			rectButton.DeflateRect(1, 1);

		const BOOL Hover = (Index==m_HoverItem) && (m_HoverPart==PARTBUTTON);
		const BOOL Pressed = Hover && m_PartPressed;

		DrawWhiteButtonBackground(dc, g, rectButton, Themed, FALSE, Pressed, Hover, FALSE, TRUE);

		if (Pressed)
			rectButton.OffsetRect(1, 1);

		dc.DrawText(_T("..."), rectButton, DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
	}

	// Reset button
	if (pData->Editable && pData->pProperty->CanDelete())
	{
		const INT Offs = (m_ItemHeight-m_IconSize)/2;
		DrawIconEx(dc, rectPart.right-m_IconSize-Offs, rectPart.top+Offs, (Index==m_HoverItem) && (m_HoverPart==PARTRESET) ? m_PartPressed ? hIconResetPressed : hIconResetHot : IsItemSelected(Index) ? hIconResetSelected : hIconResetNormal, m_IconSize, m_IconSize, 0, NULL, DI_NORMAL);

		rectPart.right -= m_IconSize+2*Offs+PADDINGX;
	}

	// Value
	if (rectPart.right==rectItem->right)
		rectPart.right -= PADDINGX;

	dc.SetTextColor(clr);
	pData->pProperty->DrawValue(dc, rectPart);

	dc.SelectObject(pOldFont);
}

void CInspectorGrid::DrawStage(CDC& dc, Graphics& g, const CRect& rect, const CRect& rectUpdate, BOOL Themed)
{
	//Header
	if (m_pHeader)
	{
		CRect rectHeader(0, -m_VScrollPos, rect.Width(), m_pHeader->GetPreferredHeight()-m_VScrollPos);
		m_pHeader->DrawHeader(dc, g, rectHeader, Themed);
	}

	CFrontstageItemView::DrawStage(dc, g, rect, rectUpdate, Themed);

	// Shadow
	if (m_pHeader && Themed)
		CTaskbar::DrawTaskbarShadow(g, rect);
}


// Label edit

BOOL CInspectorGrid::AllowItemEditLabel(INT Index) const
{
	ASSERT(Index>=0);
	ASSERT(Index<m_ItemCount);

	return GetPropertyItemData(Index)->Editable;
}

LFFont* CInspectorGrid::GetLabelFont() const
{
	return &LFGetApp()->m_DialogBoldFont;
}

RECT CInspectorGrid::GetLabelRect() const
{
	ASSERT(m_EditItem>=0);
	ASSERT(m_EditItem<m_ItemCount);

	RECT rect = GetItemRect(m_EditItem);

	rect.top += 2;
	rect.bottom -=2;
	rect.left += m_LabelWidth+3*PADDINGX;

	return rect;
}

CEdit* CInspectorGrid::CreateLabelEditControl()
{
	ASSERT(m_EditItem>=0);
	ASSERT(m_EditItem<m_ItemCount);

	return GetPropertyItemData(m_EditItem)->pProperty->CreateEditControl(GetLabelRect());
}

void CInspectorGrid::EndLabelEdit(INT Index, CString& Value)
{
	ASSERT(Index>=0);
	ASSERT(Index<m_ItemCount);

	GetPropertyItemData(Index)->pProperty->OnSetString(Value);
}


BEGIN_MESSAGE_MAP(CInspectorGrid, CFrontstageItemView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()
	ON_WM_SETCURSOR()

	ON_MESSAGE(WM_PROPERTYCHANGED, OnPropertyChanged)
END_MESSAGE_MAP()

INT CInspectorGrid::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstageItemView::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Icons
	const INT ItemHeight = GetItemHeight();
	m_IconSize = (ItemHeight>=27) ? 25 : (ItemHeight>=22) ? 20 : (ItemHeight>=18) ? 16 : 14;

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

	for (INT a=0; a<m_ItemCount; a++)
		delete GetPropertyItemData(a)->pProperty;

	CFrontstageItemView::OnDestroy();
}


void CInspectorGrid::OnMouseMove(UINT nFlags, CPoint point)
{
	if (GetCapture()==this)
	{
		const UINT Part = PartAtPosition(point, m_FocusItem);
		const BOOL PartPressed = (Part==m_HoverPart);

		if (m_PartPressed!=PartPressed)
		{
			m_PartPressed = PartPressed;
			InvalidateItem(m_FocusItem);
		}
	}
	else
	{
		CFrontstageItemView::OnMouseMove(nFlags, point);

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

	CFrontstageItemView::OnMouseLeave();
}

void CInspectorGrid::OnLButtonDown(UINT nFlags, CPoint point)
{
	CFrontstageItemView::OnLButtonDown(nFlags, point);

	const UINT Part = PartAtPosition(point, m_FocusItem);
	if (Part>=PARTBUTTON)
	{
		m_HoverPart = Part;
		m_PartPressed = TRUE;

		SetCapture();

		InvalidateItem(m_FocusItem);
	}
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

		if ((Index!=-1) && (Index==m_FocusItem))
			switch (Part)
			{
			case PARTBUTTON:
				// Edit button
				GetPropertyItemData(Index)->pProperty->OnClickButton(m_Context);
				break;

			case PARTRESET:
				// Reset button
				ResetProperty(Index, (nFlags & MK_CONTROL) || !m_pHeader);
				break;
			}
	}
	else
	{
		if ((Index!=-1) && (Index==m_FocusItem) && (Part==PARTVALUE) && GetPropertyItemData(Index)->pProperty->OnClickValue(point.x-m_LabelWidth-3*PADDINGX))
			EditLabel(Index);
	}
}

void CInspectorGrid::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	m_PartPressed = FALSE;
	ReleaseCapture();

	CFrontstageItemView::OnLButtonDblClk(nFlags, point);
}

UINT CInspectorGrid::OnGetDlgCode()
{
	return DLGC_WANTARROWS | DLGC_WANTCHARS;
}

void CInspectorGrid::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Push char to property object
	const INT Index = GetSelectedItem();
	if (Index!=-1)
	{
		const PropertyItemData* pData = GetPropertyItemData(Index);

		if (pData->Editable && pData->pProperty->WantsChars() && pData->pProperty->OnPushChar(nChar))
			return;
	}

	// Char not wanted, call default handler
	CFrontstageItemView::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CInspectorGrid::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*Message*/)
{
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	const INT Index = ItemAtPosition(point);
	if (PartAtPosition(point, Index)==PARTVALUE)
	{
		const PropertyItemData* pData = GetPropertyItemData(Index);

		if (pData->Editable)
		{
			SetCursor(pData->pProperty->SetCursor(point.x-pData->Hdr.Rect.left-m_LabelWidth-3*PADDINGX));

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
