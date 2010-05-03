#include "StdAfx.h"
#include "LFSelectLocationGPSDlg.h"
#include "Resource.h"

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFSelectLocationGPSDlg::LFSelectLocationGPSDlg(CWnd* pParentWnd, LFGeoCoordinates* pCoord)
	: CDialog(IDD_SELECTGPS, pParentWnd)
{
	m_pCoord = pCoord;
}

LFSelectLocationGPSDlg::~LFSelectLocationGPSDlg()
{
}

BEGIN_MESSAGE_MAP(LFSelectLocationGPSDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_NOTIFY(MAP_UPDATE_LOCATION, IDC_MAP_SELECTION, OnUpdateEdit)
END_MESSAGE_MAP()


BOOL LFSelectLocationGPSDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LoadIcon(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDD_SELECTGPS));
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	m_Map.ModifyStyle(0, WS_BORDER);
	m_Map.SetGeoCoordinates(*m_pCoord);
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
	*m_pCoord = *pTag->pCoord;

	wchar_t tmpStr[256];
	LFGeoCoordinateToString(m_pCoord->Latitude, tmpStr, 256, TRUE);
	GetDlgItem(IDC_LATITUDE)->SetWindowText(tmpStr);
	LFGeoCoordinateToString(m_pCoord->Longitude, tmpStr, 256, FALSE);
	GetDlgItem(IDC_LONGITUDE)->SetWindowText(tmpStr);

	*pResult = 0;
}

void LFSelectLocationGPSDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MAP_SELECTION, m_Map);
}
