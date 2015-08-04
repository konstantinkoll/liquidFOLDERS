
// LFSelectLocationIATADlg.cpp: Implementierung der Klasse LFSelectLocationIATA
//

#include "stdafx.h"
#include "LFCommDlg.h"


__forceinline void Swap(LFAirport*& Eins, LFAirport*& Zwei)
{
	LFAirport* Temp = Eins;
	Eins = Zwei;
	Zwei = Temp;
}


// LFSelectLocationIATADlg
//

LFSelectLocationIATADlg::LFSelectLocationIATADlg(UINT nIDTemplate, CWnd* pParentWnd, CHAR* Airport, BOOL AllowOverwriteName, BOOL AllowOverwriteGPS)
	: CDialog(nIDTemplate, pParentWnd)
{
	m_nIDTemplate = nIDTemplate;

	m_LastCountrySelected = LFGetApp()->GetInt(_T("IATALastCountrySelected"), 0);
	m_LastSortColumn = LFGetApp()->GetInt(_T("IATALastSortColumn"), 0);
	m_LastSortDirection = LFGetApp()->GetInt(_T("IATALastSortDirection"), FALSE);
	m_OverwriteName = AllowOverwriteName ? LFGetApp()->GetInt(_T("IATAOverwriteName"), TRUE) : FALSE;
	m_OverwriteGPS = AllowOverwriteGPS ? LFGetApp()->GetInt(_T("IATAOverwriteGPS"), TRUE) : FALSE;
	m_AllowOverwriteName = AllowOverwriteName;
	m_AllowOverwriteGPS = AllowOverwriteGPS;

	if (Airport)
	{
		if (!LFIATAGetAirportByCode(Airport, &p_Airport))
			p_Airport = NULL;
	}
	else
	{
		p_Airport = NULL;
	}
}

void LFSelectLocationIATADlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_MAP_PREVIEW, m_wndMap);
	DDX_Control(pDX, IDC_AIRPORTS, m_wndList);
	if (m_nIDTemplate==IDD_SELECTIATA)
	{
		DDX_Check(pDX, IDC_REPLACE_NAME, m_OverwriteName);
		DDX_Check(pDX, IDC_REPLACE_GPS, m_OverwriteGPS);
	}

	if (pDX->m_bSaveAndValidate)
	{
		LFGetApp()->WriteInt(_T("IATALastCountrySelected"), m_LastCountrySelected);
		LFGetApp()->WriteInt(_T("IATALastSortColumn"), m_LastSortColumn);
		LFGetApp()->WriteInt(_T("IATALastSortDirection"), m_LastSortDirection);
		if (m_AllowOverwriteName)
			LFGetApp()->WriteInt(_T("IATAOverwriteName"), m_OverwriteName);
		if (m_AllowOverwriteGPS)
			LFGetApp()->WriteInt(_T("IATAOverwriteGPS"), m_OverwriteGPS);
	}
}

INT LFSelectLocationIATADlg::Compare(INT n1, INT n2)
{
	INT Result = 0;

	switch (m_LastSortColumn)
	{
	case 0:
		Result = strcmp(m_Airports[n1]->Code, m_Airports[n2]->Code);
		break;

	case 1:
		Result = strcmp(m_Airports[n1]->Name, m_Airports[n2]->Name);
		break;
	}

	if (m_LastSortDirection)
		Result = -Result;

	return Result;
}

void LFSelectLocationIATADlg::Heap(INT Wurzel, INT Anzahl)
{
	while (Wurzel<=Anzahl/2-1)
	{
		INT Index = (Wurzel+1)*2-1;
		if (Index+1<Anzahl)
			if (Compare(Index, Index+1)<0)
				Index++;
		if (Compare(Wurzel, Index)<0)
		{
			Swap(m_Airports[Wurzel], m_Airports[Index]);
			Wurzel = Index;
		}
		else
		{
			break;
		}
	}
}

void LFSelectLocationIATADlg::Sort()
{
	if (m_nAirports>1)
	{
		for (INT a=m_nAirports/2-1; a>=0; a--)
			Heap(a, m_nAirports);
		for (INT a=m_nAirports-1; a>0; a--)
		{
			Swap(m_Airports[0], m_Airports[a]);
			Heap(0, a);
		}
	}

	CHeaderCtrl* pHeaderCtrl = m_wndList.GetHeaderCtrl();

	HDITEM Item;
	ZeroMemory(&Item, sizeof(Item));
	Item.mask = HDI_FORMAT;

	for (INT a=0; a<2; a++)
	{
		pHeaderCtrl->GetItem(a, &Item);

		Item.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
		if (a==(INT)m_LastSortColumn)
			Item.fmt |= m_LastSortDirection ? HDF_SORTDOWN : HDF_SORTUP;

		pHeaderCtrl->SetItem(a, &Item);
	}

	INT sel = 0;
	if (p_Airport)
	{
		for (INT a=0; a<m_nAirports; a++)
			if (m_Airports[a]==p_Airport)
			{
				sel = a;
				break;
			}
	}
	m_wndList.SetItemState(sel, LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	m_wndList.SetItemState(sel, LVIS_SELECTED, LVIS_SELECTED);
	m_wndList.EnsureVisible(sel, FALSE);
}

void LFSelectLocationIATADlg::LoadCountry(UINT country)
{
	m_wndList.SetRedraw(FALSE);
	m_wndList.SetItemCount(0);

	m_nAirports = 0;

	INT Index = LFIATAGetNextAirportByCountry(country, -1, &m_Airports[m_nAirports]);
	while ((Index!=-1) && (m_nAirports<MaxAirportsPerCountry))
		Index = LFIATAGetNextAirportByCountry(country, Index, &m_Airports[++m_nAirports]);

	m_wndList.SetItemCount(m_nAirports);
	Sort();

	m_wndList.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
	m_wndList.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);

	m_wndList.SetRedraw(TRUE);
	m_wndList.Invalidate();

	UpdatePreview();
}

void LFSelectLocationIATADlg::UpdatePreview()
{
	INT Index = m_wndList.GetNextItem(-1, LVIS_SELECTED);

	p_Airport = m_Airports[Index];
	m_wndMap.Update(p_Airport);

	LFGeoCoordinatesToString(p_Airport->Location, m_Buffer, 256, FALSE);
	GetDlgItem(IDC_GPSLOCATION)->SetWindowText(m_Buffer);
}


BEGIN_MESSAGE_MAP(LFSelectLocationIATADlg, CDialog)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_AIRPORTS, OnCustomDraw)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_AIRPORTS, OnGetDispInfo)
	ON_NOTIFY(NM_DBLCLK, IDC_AIRPORTS, OnDoubleClick)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_AIRPORTS, OnItemChanged)
	ON_NOTIFY(HDN_ITEMCLICK, 0, OnSortItems)
	ON_CONTROL(CBN_SELCHANGE, IDC_COUNTRY, OnSelectCountry)
	ON_NOTIFY(NM_CLICK, IDC_REPORTERROR, OnReportError)
END_MESSAGE_MAP()

BOOL LFSelectLocationIATADlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LFGetApp()->LoadDialogIcon(m_nIDTemplate);
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	// Combobox füllen
	CComboBox* c = (CComboBox*)GetDlgItem(IDC_COUNTRY);
	UINT cCount = LFIATAGetCountryCount();
	for (UINT a=0; a<cCount; a++)
	{
		CString tmpStr(&LFIATAGetCountry(a)->Name[0]);
		c->AddString(tmpStr);
	}

	// Liste konfigurieren
	m_wndList.SetExtendedStyle(m_wndList.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES);

	CString tmpStr;

	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;

	ENSURE(tmpStr.LoadString(IDS_AIRPORT_CODE));
	lvc.pszText = tmpStr.GetBuffer();
	m_wndList.InsertColumn(0, &lvc);

	ENSURE(tmpStr.LoadString(IDS_AIRPORT_LOCATION));
	lvc.pszText = tmpStr.GetBuffer();
	lvc.iSubItem = 1;
	m_wndList.InsertColumn(1, &lvc);

	// Init
	UINT country = p_Airport ? p_Airport->CountryID : m_LastCountrySelected;
	tmpStr = LFIATAGetCountry(country)->Name;
	c->SelectString(-1, tmpStr);
	LoadCountry(country);

	if (m_nIDTemplate==IDD_SELECTIATA)
	{
		GetDlgItem(IDC_REPLACE_NAME)->EnableWindow(m_AllowOverwriteName);
		GetDlgItem(IDC_REPLACE_GPS)->EnableWindow(m_AllowOverwriteGPS);
	}

	return TRUE;
}

void LFSelectLocationIATADlg::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = (NMLVCUSTOMDRAW*)pNMHDR;
	*pResult = CDRF_DODEFAULT;

	if (CDDS_PREPAINT==pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else
		if (CDDS_ITEMPREPAINT==pLVCD->nmcd.dwDrawStage)
		{
			INT Index = (INT)pLVCD->nmcd.dwItemSpec;

			if (strcmp(m_Airports[Index]->Code, m_Airports[Index]->MetroCode)==0)
				pLVCD->clrText = 0xFF0000;
		}
}

void LFSelectLocationIATADlg::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;

	INT Index = pItem->iItem;

	if (pItem->mask & LVIF_TEXT)
	{
		CHAR* src = (pItem->iSubItem==0) ? &m_Airports[Index]->Code[0] : &m_Airports[Index]->Name[0];
		MultiByteToWideChar(CP_ACP, 0, src, -1, m_Buffer, 256);
		pItem->pszText = (LPWSTR)m_Buffer;
	}
}

void LFSelectLocationIATADlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	PostMessage(WM_COMMAND, (WPARAM)IDOK);
}

void LFSelectLocationIATADlg::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVIS_SELECTED))
		UpdatePreview();
}

void LFSelectLocationIATADlg::OnSortItems(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW *pLV = (NMLISTVIEW*)pNMHDR;
	INT col = pLV->iItem;

	if (col!=(INT)m_LastSortColumn)
	{
		m_LastSortColumn = col;
		m_LastSortDirection = FALSE;
	}
	else
	{
		m_LastSortDirection = !m_LastSortDirection;
	}

	m_wndList.SetRedraw(FALSE);

	Sort();

	m_wndList.SetRedraw(TRUE);
	m_wndList.Invalidate();

	*pResult = 0;
}

void LFSelectLocationIATADlg::OnSelectCountry()
{
	m_LastCountrySelected = ((CComboBox*)GetDlgItem(IDC_COUNTRY))->GetCurSel();
	LoadCountry(m_LastCountrySelected);
}

void LFSelectLocationIATADlg::OnReportError(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	CString Subject = _T("IATA database error");

	INT Index = m_wndList.GetNextItem(-1, LVIS_SELECTED);
	if (Index!=-1)
	{
		CString Code(m_Airports[Index]->Code);
		CString Name(m_Airports[Index]->Name);
		CString Country(&LFIATAGetCountry(m_Airports[Index]->CountryID)->Name[0]);
		Subject += _T(": ")+Code+_T(" (")+Name+_T(", ")+Country+_T(")");
	}

	LFGetApp()->SendMail(Subject);

	*pResult = 0;
}
