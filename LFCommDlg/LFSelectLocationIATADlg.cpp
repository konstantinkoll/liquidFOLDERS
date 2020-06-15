
// LFSelectLocationIATADlg.cpp: Implementierung der Klasse FMSelectLocationIATA
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CAirportList
//

#define MAXAIRPORTCOUNT     2500

CString CAirportList::m_SubitemName;
UINT CAirportList::m_SortAttribute = 0;
BOOL CAirportList::m_SortDescending = FALSE;

CAirportList::CAirportList()
	: CFrontstageItemView(FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLEFOCUSITEM, sizeof(AirportItemData))
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = LFGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CAirportList";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CAirportList", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	// Subitem name
	if (m_SubitemName.IsEmpty())
		ENSURE(m_SubitemName.LoadString(IDS_AIRPORT_CODE));

	// Item
	CFrontstageScroller::SetItemHeight(LFGetApp()->m_DefaultFont.GetFontHeight()+2*ITEMCELLPADDINGY);
}


// Header

void CAirportList::UpdateHeaderColumn(ATTRIBUTE Attr, HDITEM& HeaderItem) const
{
	HeaderItem.mask = HDI_WIDTH | HDI_FORMAT;
	HeaderItem.fmt = HDF_STRING | HDF_LEFT;

	if (m_SortAttribute==Attr)
		HeaderItem.fmt |= m_SortDescending ? HDF_SORTDOWN : HDF_SORTUP;

	if ((HeaderItem.cxy=m_ColumnWidth[Attr])<ITEMVIEWMINWIDTH)
		HeaderItem.cxy = ITEMVIEWMINWIDTH;
}

void CAirportList::HeaderColumnClicked(ATTRIBUTE Attr)
{
	m_SortDescending = (m_SortAttribute==Attr) ? !m_SortDescending : FALSE;
	m_SortAttribute = Attr;

	UpdateHeader();
	SortItems();

	AdjustLayout();
}


// Layouts

void CAirportList::AdjustLayout()
{
	// Header
	m_ColumnWidth[0] = LFGetApp()->m_DefaultFont.GetTextExtent(m_SubitemName).cx+ITEMCELLSPACER;
	m_ColumnWidth[1] = 0;

	SetFixedColumnWidths(m_ColumnOrder, m_ColumnWidth);

	UpdateHeader();

	// Item layout
	AdjustLayoutList();
}


// Item data

void CAirportList::AddAirport(LPCAIRPORT lpcAirport)
{
	ASSERT(lpcAirport);

	// Item data
	AirportItemData Data;

	Data.lpcAirport = lpcAirport;

	AddItem(&Data);
}

BOOL CAirportList::SetAirports(UINT CountryID, LPCAIRPORT lpcAirportSelected)
{
	HideTooltip();

	// Header
	if (!HasHeader())
	{
		AddHeaderColumn(FALSE, m_SubitemName);
		AddHeaderColumn(FALSE, IDS_AIRPORT_LOCATION);
	}

	// Items
	SetItemCount(MAXAIRPORTCOUNT, FALSE);

	LPCAIRPORT lpcAirport;
	INT Index = LFIATAGetNextAirportByCountry(CountryID, -1, lpcAirport);
	while (Index!=-1)
	{
		AddAirport(lpcAirport);

		Index = LFIATAGetNextAirportByCountry(CountryID, Index, lpcAirport);
	}

	LastItem();
	SortItems();

	AdjustLayout();

	// Set focus item
	if (lpcAirportSelected)
		for (INT a=0; a<m_ItemCount; a++)
			if (lpcAirportSelected==GetAirport(a))
			{
				SetFocusItem(a);

				return TRUE;
			}

	SetFocusItem(0);

	return FALSE;
}

LPCAIRPORT CAirportList::GetSelectedAirport() const
{
	const INT Index = GetSelectedItem();

	return (Index<0) ? NULL : GetAirport(Index);
}


// Item sort

INT CAirportList::CompareItems(AirportItemData* pData1, AirportItemData* pData2, const SortParameters& Parameters)
{
	INT Result = 0;

	switch (Parameters.Attr)
	{
	case 0:
		Result = strcmp(pData1->lpcAirport->Code, pData2->lpcAirport->Code);
		break;

	case 1:
		Result = strcmp(pData1->lpcAirport->Name, pData2->lpcAirport->Name);
		break;
	}

	return Parameters.Descending ? -Result : Result;
}


// Item handling

void CAirportList::ShowTooltip(const CPoint& point)
{
	ASSERT(m_HoverItem>=0);

	LPCAIRPORT lpcAirport = GetAirport(m_HoverItem);

	CString Hint;
	LFTooltip::AppendAttribute(Hint, IDS_AIRPORT_NAME, lpcAirport->Name);
	LFTooltip::AppendAttribute(Hint, IDS_AIRPORT_COUNTRY, LFIATAGetCountry(lpcAirport->CountryID)->Name);

	WCHAR tmpStr[256];
	LFGeoCoordinatesToString(lpcAirport->Location, tmpStr, 256, FALSE);
	LFTooltip::AppendAttribute(Hint, IDS_AIRPORT_LOCATION, tmpStr);

	LFGetApp()->ShowTooltip(this, point, CString(lpcAirport->Name), Hint, NULL, LFIATACreateAirportMap(lpcAirport, 192, 192));
}

COLORREF CAirportList::GetItemTextColor(INT Index, BOOL /*Themed*/) const
{
	LPCAIRPORT lpcAirport = GetAirport(Index);

	return strcmp(lpcAirport->Code, lpcAirport->MetroCode) ? (COLORREF)-1 : 0x208040;
}


// Drawing

void CAirportList::DrawItemCell(CDC& dc, CRect& rectCell, INT Index, ATTRIBUTE Attr, BOOL /*Themed*/)
{
	LPCAIRPORT lpcAirport = GetAirport(Index);

	// Column
	CString strCell;

	switch (Attr)
	{
	case 0:
		strCell = lpcAirport->Code;
		break;

	case 1:
		strCell = lpcAirport->Name;
		break;
	}

	dc.DrawText(strCell, rectCell, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
}

void CAirportList::DrawItem(CDC& dc, Graphics& /*g*/, LPCRECT rectItem, INT Index, BOOL Themed)
{
	ASSERT(rectItem);

	DrawListItem(dc, rectItem, Index, Themed, m_ColumnOrder, m_ColumnWidth);
}


// LFSelectLocationIATADlg
//

LFSelectLocationIATADlg::LFSelectLocationIATADlg(CWnd* pParentWnd, LPCSTR lpcAirport, UINT nIDTemplate)
	: LFDialog(nIDTemplate, pParentWnd)
{
	m_LastCountrySelected = LFGetApp()->GetInt(_T("IATALastCountrySelected"), 0);
	m_LastSortColumn = LFGetApp()->GetInt(_T("IATALastSortColumn"), 0);
	m_LastSortDirection = LFGetApp()->GetInt(_T("IATALastSortDirection"), FALSE);

	p_Airport = NULL;
	LFIATAGetAirportByCode(lpcAirport, p_Airport);
}

void LFSelectLocationIATADlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_AIRPORTS, m_wndAirportList);

	if (pDX->m_bSaveAndValidate)
	{
		p_Airport = GetSelectedAirport();

		LFGetApp()->WriteInt(_T("IATALastCountrySelected"), m_LastCountrySelected);
		LFGetApp()->WriteInt(_T("IATALastSortColumn"), m_LastSortColumn);
		LFGetApp()->WriteInt(_T("IATALastSortDirection"), m_LastSortDirection);
	}
}

BOOL LFSelectLocationIATADlg::InitDialog()
{
	// Countries and territories
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COUNTRY);

	const UINT CountyCount = LFIATAGetCountryCount();
	for (UINT a=0; a<CountyCount; a++)
		pComboBox->AddString(CString(LFIATAGetCountry(a)->Name));

	const UINT CountryID = p_Airport ? p_Airport->CountryID : m_LastCountrySelected;
	pComboBox->SelectString(-1, CString(LFIATAGetCountry(CountryID)->Name));

	// Airports
	if (m_wndAirportList.SetAirports(CountryID, p_Airport))
	{
		m_wndAirportList.SetFocus();

		return FALSE;
	}

	return TRUE;
}


BEGIN_MESSAGE_MAP(LFSelectLocationIATADlg, LFDialog)
	ON_CONTROL(CBN_CLOSEUP, IDC_COUNTRY, OnCloseUp)
	ON_CONTROL(CBN_SELCHANGE, IDC_COUNTRY, OnSelectCountry)
END_MESSAGE_MAP()

void LFSelectLocationIATADlg::OnCloseUp()
{
	m_wndAirportList.SetFocus();
}

void LFSelectLocationIATADlg::OnSelectCountry()
{
	m_wndAirportList.SetAirports(m_LastCountrySelected=((CComboBox*)GetDlgItem(IDC_COUNTRY))->GetCurSel(), p_Airport);
}
