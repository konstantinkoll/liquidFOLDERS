
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

	INT Scanned = swscanf_s(str.GetBuffer(), L"%i�%i\'%i\"%c", &Deg, &Min, &Sec, &Ch, 1);

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

LFSelectLocationGPSDlg::LFSelectLocationGPSDlg(const LFGeoCoordinates Location, CWnd* pParentWnd)
	: CDialog(IDD_SELECTGPS, pParentWnd)
{
	m_Location = Location;
}

void LFSelectLocationGPSDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_MAP_SELECTION, m_Map);

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


BEGIN_MESSAGE_MAP(LFSelectLocationGPSDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_NOTIFY(MAP_UPDATE_LOCATION, IDC_MAP_SELECTION, OnUpdateEdit)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_EN_KILLFOCUS(IDC_LATITUDE, OnLatitudeChanged)
	ON_EN_KILLFOCUS(IDC_LONGITUDE, OnLongitudeChanged)
END_MESSAGE_MAP()

BOOL LFSelectLocationGPSDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol f�r dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LFGetApp()->LoadDialogIcon(IDD_SELECTGPS);
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	m_Map.SetGeoCoordinates(m_Location);
	SetTimer(1, 500, NULL);

	return TRUE;
}

void LFSelectLocationGPSDlg::OnDestroy()
{
	KillTimer(1);

	CDialog::OnDestroy();
}

void LFSelectLocationGPSDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
		m_Map.OnBlink();

	CDialog::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

void LFSelectLocationGPSDlg::OnUpdateEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	tagGPSDATA* pTag = (tagGPSDATA*)pNMHDR;
	m_Location = *pTag->pCoord;

	WCHAR tmpStr[256];
	LFGeoCoordinateToString(m_Location.Latitude, tmpStr, 256, TRUE, FALSE);
	GetDlgItem(IDC_LATITUDE)->SetWindowText(tmpStr);
	LFGeoCoordinateToString(m_Location.Longitude, tmpStr, 256, FALSE, FALSE);
	GetDlgItem(IDC_LONGITUDE)->SetWindowText(tmpStr);

	*pResult = 0;
}

void LFSelectLocationGPSDlg::OnReset()
{
	m_Location.Latitude = m_Location.Longitude = 0.0;
	EndDialog(IDOK);
}

void LFSelectLocationGPSDlg::OnLatitudeChanged()
{
	CString strLat;
	GetDlgItem(IDC_LATITUDE)->GetWindowText(strLat);
	m_Location.Latitude = StringToCoord(strLat);

	m_Map.SetGeoCoordinates(m_Location);
}

void LFSelectLocationGPSDlg::OnLongitudeChanged()
{
	CString strLon;
	GetDlgItem(IDC_LONGITUDE)->GetWindowText(strLon);
	m_Location.Longitude = StringToCoord(strLon);

	m_Map.SetGeoCoordinates(m_Location);
}
