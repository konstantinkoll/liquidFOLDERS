
// LFSelectPropertyIATADlg.cpp: Implementierung der Klasse LFSelectPropertyIATA
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFSelectPropertyIATADlg
//

LFSelectPropertyIATADlg::LFSelectPropertyIATADlg(CWnd* pParentWnd, const LPCSTR lpcAirport, BOOL AllowOverwriteName, BOOL AllowOverwriteGPS)
	: LFSelectLocationIATADlg(pParentWnd, lpcAirport, IDD_SELECTPROPERTYIATA)
{
	m_AllowOverwriteName = AllowOverwriteName;
	m_AllowOverwriteGPS = AllowOverwriteGPS;

	m_OverwriteName = AllowOverwriteName ? LFGetApp()->GetInt(_T("IATAOverwriteName"), TRUE) : FALSE;
	m_OverwriteGPS = AllowOverwriteGPS ? LFGetApp()->GetInt(_T("IATAOverwriteGPS"), TRUE) : FALSE;
}

void LFSelectPropertyIATADlg::DoDataExchange(CDataExchange* pDX)
{
	LFSelectLocationIATADlg::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_REPLACE_NAME, m_OverwriteName);
	DDX_Check(pDX, IDC_REPLACE_GPS, m_OverwriteGPS);

	if (pDX->m_bSaveAndValidate)
	{
		if (m_AllowOverwriteName)
			LFGetApp()->WriteInt(_T("IATAOverwriteName"), m_OverwriteName);

		if (m_AllowOverwriteGPS)
			LFGetApp()->WriteInt(_T("IATAOverwriteGPS"), m_OverwriteGPS);
	}
}

BOOL LFSelectPropertyIATADlg::InitDialog()
{
	LFSelectLocationIATADlg::InitDialog();

	// Optionen
	GetDlgItem(IDC_REPLACE_NAME)->EnableWindow(m_AllowOverwriteName);
	GetDlgItem(IDC_REPLACE_GPS)->EnableWindow(m_AllowOverwriteGPS);

	return p_Airport!=NULL;
}
