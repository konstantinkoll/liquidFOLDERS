
// LFSelectLocationIATADlg.cpp: Implementierung der Klasse LFSelectLocationIATA
//

#pragma once
#include "StdAfx.h"
#include "LFSelectLocationIATADlg.h"
#include "Resource.h"
#include "LFCore.h"


// LFSelectLocationIATADlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFSelectLocationIATADlg::LFSelectLocationIATADlg(UINT nIDTemplate, CWnd* pParentWnd, CHAR* Airport, BOOL AllowOverwriteName, BOOL AllowOverwriteGPS)
	: CDialog(nIDTemplate, pParentWnd)
{
	m_nIDTemplate = nIDTemplate;

	p_App = (LFApplication*)AfxGetApp();
	m_LastCountrySelected = p_App->GetGlobalInt(_T("LastCountrySelected"), 0);
	m_OverwriteName = AllowOverwriteName ? p_App->GetGlobalInt(_T("IATAOverwriteName"), TRUE) : FALSE;
	m_OverwriteGPS = AllowOverwriteGPS ? p_App->GetGlobalInt(_T("IATAOverwriteGPS"), TRUE) : FALSE;
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

void LFSelectLocationIATADlg::LoadCountry(UINT country, BOOL SelectFirst)
{
	CListCtrl* li = (CListCtrl*)GetDlgItem(IDC_AIRPORTS);
	li->SetRedraw(FALSE);
	li->SetItemCount(0);

	m_nAirports = 0;

	INT idx = LFIATAGetNextAirportByCountry(country, -1, &m_Airports[m_nAirports]);
	while ((idx!=-1) && (m_nAirports<MaxAirportsPerCountry))
		idx = LFIATAGetNextAirportByCountry(country, idx, &m_Airports[++m_nAirports]);

	li->SetItemCount(m_nAirports);
	li->SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
	li->SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);

	INT sel = 0;
	if ((!SelectFirst) && (p_Airport))
	{
		for (INT a=0; a<m_nAirports; a++)
			if (m_Airports[a]==p_Airport)
			{
				sel = a;
				break;
			}
	}
	li->SetItemState(sel, LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	li->SetItemState(sel, LVIS_SELECTED, LVIS_SELECTED);
	li->EnsureVisible(sel, FALSE);

	li->SetRedraw(TRUE);
	li->Invalidate();
}

void LFSelectLocationIATADlg::UpdatePreview()
{
	CListCtrl* li = (CListCtrl*)GetDlgItem(IDC_AIRPORTS);
	INT idx = li->GetNextItem(-1, LVIS_SELECTED);

	p_Airport = m_Airports[idx];
	m_Map.Update(p_Airport);

	LFGeoCoordinatesToString(p_Airport->Location, m_Buffer, 256, false);
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
	HICON hIcon = LoadIcon(LFCommDlgDLL.hResource, MAKEINTRESOURCE(m_nIDTemplate));
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
	CListCtrl* l = (CListCtrl*)GetDlgItem(IDC_AIRPORTS);
	l->SetExtendedStyle(l->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES);

	CString tmpStr;

	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;

	ENSURE(tmpStr.LoadString(IDS_AIRPORT_CODE));
	lvc.pszText = tmpStr.GetBuffer();
	lvc.cx = 70;
	l->InsertColumn(0, &lvc);

	ENSURE(tmpStr.LoadString(IDS_AIRPORT_LOCATION));
	lvc.pszText = tmpStr.GetBuffer();
	lvc.cx = 297;
	lvc.iSubItem = 1;
	l->InsertColumn(1, &lvc);

	// Init
	UINT country = p_Airport ? p_Airport->CountryID : m_LastCountrySelected;
	tmpStr = LFIATAGetCountry(country)->Name;
	c->SelectString(-1, tmpStr);
	LoadCountry(country, FALSE);

	if (m_nIDTemplate==IDD_SELECTIATA)
	{
		GetDlgItem(IDC_REPLACE_NAME)->EnableWindow(m_AllowOverwriteName);
		GetDlgItem(IDC_REPLACE_GPS)->EnableWindow(m_AllowOverwriteGPS);
	}

	return TRUE;
}

void LFSelectLocationIATADlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_MAP_PREVIEW, m_Map);
	if (m_nIDTemplate==IDD_SELECTIATA)
	{
		DDX_Check(pDX, IDC_REPLACE_NAME, m_OverwriteName);
		DDX_Check(pDX, IDC_REPLACE_GPS, m_OverwriteGPS);
	}

	if (pDX->m_bSaveAndValidate)
	{
		p_App->WriteGlobalInt(_T("LastCountrySelected"), m_LastCountrySelected);
		if (m_AllowOverwriteName)
			p_App->WriteGlobalInt(_T("IATAOverwriteName"), m_OverwriteName);
		if (m_AllowOverwriteGPS)
			p_App->WriteGlobalInt(_T("IATAOverwriteGPS"), m_OverwriteGPS);
	}
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
			INT idx = (INT)pLVCD->nmcd.dwItemSpec;

			if (strcmp(m_Airports[idx]->Code, m_Airports[idx]->MetroCode)==0)
				pLVCD->clrText = 0xFF0000;
		}
}

void LFSelectLocationIATADlg::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;

	INT idx = pItem->iItem;

	if (pItem->mask & LVIF_TEXT)
	{
		CHAR* src = (pItem->iSubItem==0) ? &m_Airports[idx]->Code[0] : &m_Airports[idx]->Name[0];
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

INT LFSelectLocationIATADlg::Compare(INT col, INT n1, INT n2)
{
	switch (col)
	{
	case 0:
		return strcmp(m_Airports[n1]->Code, m_Airports[n2]->Code);
	case 1:
		return strcmp(m_Airports[n1]->Name, m_Airports[n2]->Name);
	default:
		return 0;
	}
}

void LFSelectLocationIATADlg::Heap(INT col, INT wurzel, INT anz)
{
	while (wurzel<=anz/2-1)
	{
		INT idx = (wurzel+1)*2-1;
		if (idx+1<anz)
			if (Compare(col, idx, idx+1)<0)
				idx++;
		if (Compare(col, wurzel, idx)<0)
		{
			std::swap(m_Airports[wurzel], m_Airports[idx]);
			wurzel = idx;
		}
		else
		{
			break;
		}
	}
}

void LFSelectLocationIATADlg::OnSortItems(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_nAirports>1)
	{
		NMLISTVIEW *pLV = (NMLISTVIEW*)pNMHDR;
		INT col = pLV->iItem;

		for (INT a=m_nAirports/2-1; a>=0; a--)
			Heap(col, a, m_nAirports);
		for (INT a=m_nAirports-1; a>0; )
		{
			std::swap(m_Airports[0], m_Airports[a]);
			Heap(col, 0, a--);
		}

		GetDlgItem(IDC_AIRPORTS)->Invalidate();
		UpdatePreview();
	}

	*pResult = 0;
}

void LFSelectLocationIATADlg::OnSelectCountry()
{
	m_LastCountrySelected = ((CComboBox*)GetDlgItem(IDC_COUNTRY))->GetCurSel();
	LoadCountry(m_LastCountrySelected, TRUE);
}

void LFSelectLocationIATADlg::OnReportError(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	CString Subject = _T("IATA database error");

	INT idx = ((CListCtrl*)GetDlgItem(IDC_AIRPORTS))->GetNextItem(-1, LVIS_SELECTED);
	if (idx!=-1)
	{
		CString Code(m_Airports[idx]->Code);
		CString Name(m_Airports[idx]->Name);
		CString Country(&LFIATAGetCountry(m_Airports[idx]->CountryID)->Name[0]);
		Subject += _T(": ")+Code+_T(" (")+Name+_T(", ")+Country+_T(")");
	}

	p_App->SendMail(Subject);

	*pResult = 0;
}
