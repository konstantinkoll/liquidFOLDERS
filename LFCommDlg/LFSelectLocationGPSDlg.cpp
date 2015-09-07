
// LFSelectLocationGPSDlg.cpp: Implementierung der Klasse LFSelectLocationGPSDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


DOUBLE StringToCoord(CString str)
{
	INT Deg;
	INT Min;
	INT Sec;
	WCHAR Ch;
	DOUBLE Result = 0.0;

	INT Scanned = swscanf_s(str.GetBuffer(), L"%i°%i\'%i\"%c", &Deg, &Min, &Sec, &Ch, 1);

	if (Scanned>=1)
		Result += Deg;
	if (Scanned>=2)
		Result += abs(Min)/60.0;
	if (Scanned>=3)
		Result += abs(Sec)/3600.0;
	if (Scanned>=4)
		if ((Ch==L'N') || (Ch==L'W'))
			Result = -Result;

	if ((Result<-180.0) || (Result>180.0))
		Result = 0.0;

	return Result;
}


// LFSelectLocationGPSDlg
//

LFSelectLocationGPSDlg::LFSelectLocationGPSDlg(const LFGeoCoordinates& Location, CWnd* pParentWnd)
	: LFDialog(IDD_SELECTGPS, pParentWnd)
{
	m_Location = Location;
}

void LFSelectLocationGPSDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MAP, m_wndMap);

	if (pDX->m_bSaveAndValidate)
	{
		CString strLat;
		GetDlgItem(IDC_LATITUDE)->GetWindowText(strLat);
		m_Location.Latitude = StringToCoord(strLat);

		CString strLon;
		GetDlgItem(IDC_LONGITUDE)->GetWindowText(strLon);
		m_Location.Longitude = StringToCoord(strLon);
	}
}


BEGIN_MESSAGE_MAP(LFSelectLocationGPSDlg, LFDialog)
	ON_NOTIFY(MAP_UPDATE_LOCATION, IDC_MAP, OnUpdateEdit)
	ON_EN_KILLFOCUS(IDC_LATITUDE, OnLatitudeChanged)
	ON_EN_KILLFOCUS(IDC_LONGITUDE, OnLongitudeChanged)

	ON_COMMAND(IDM_SELECTGPS_IATA, OnIATA)
	ON_COMMAND(IDM_SELECTGPS_RESET, OnReset)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_SELECTGPS_IATA, IDM_SELECTGPS_RESET, OnUpdateCommands)
END_MESSAGE_MAP()

BOOL LFSelectLocationGPSDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	m_wndMap.SetLocation(m_Location);
	m_wndMap.SetMenu(IDM_SELECTGPS);

	return FALSE;
}

void LFSelectLocationGPSDlg::OnUpdateEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_GPSDATA* pTag = (NM_GPSDATA*)pNMHDR;

	m_Location = *pTag->pLocation;

	WCHAR tmpStr[256];
	LFGeoCoordinateToString(m_Location.Latitude, tmpStr, 256, TRUE, FALSE);
	GetDlgItem(IDC_LATITUDE)->SetWindowText(tmpStr);

	LFGeoCoordinateToString(m_Location.Longitude, tmpStr, 256, FALSE, FALSE);
	GetDlgItem(IDC_LONGITUDE)->SetWindowText(tmpStr);

	*pResult = 0;
}

void LFSelectLocationGPSDlg::OnLatitudeChanged()
{
	CString strLat;
	GetDlgItem(IDC_LATITUDE)->GetWindowText(strLat);

	m_Location.Latitude = StringToCoord(strLat);

	m_wndMap.SetLocation(m_Location);
}

void LFSelectLocationGPSDlg::OnLongitudeChanged()
{
	CString strLon;
	GetDlgItem(IDC_LONGITUDE)->GetWindowText(strLon);

	m_Location.Longitude = StringToCoord(strLon);

	m_wndMap.SetLocation(m_Location);
}


void LFSelectLocationGPSDlg::OnIATA()
{
	LFSelectLocationIATADlg dlg(FALSE, this);
	if (dlg.DoModal()==IDOK)
	{
		m_Location = dlg.p_Airport->Location;

		m_wndMap.SetLocation(m_Location);
	}
}

void LFSelectLocationGPSDlg::OnReset()
{
	m_Location.Latitude = m_Location.Longitude = 0.0;

	m_wndMap.SetLocation(m_Location);
}

void LFSelectLocationGPSDlg::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;

	if (pCmdUI->m_nID==IDM_SELECTGPS_RESET)
		b &= (m_Location.Latitude!=0) || (m_Location.Longitude!=0);

	pCmdUI->Enable(b);
}
