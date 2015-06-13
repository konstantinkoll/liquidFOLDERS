
// GlobeOptionsDlg.cpp: Implementierung der Klasse GlobeOptionsDlg
//

#include "stdafx.h"
#include "GlobeOptionsDlg.h"


// GlobeOptionsDlg
//

GlobeOptionsDlg::GlobeOptionsDlg(LFViewParameters* View, UINT Context, CWnd* pParentWnd)
	: LFDialog(IDD_GLOBEOPTIONS, pParentWnd)
{
	ASSERT(View);

	p_View = View;
	m_Context = Context;
}

void GlobeOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_TEXTURESIZE, m_wndTextureSize);
	DDX_Control(pDX, IDC_VIEWPORT, m_wndViewport);

	DDX_Check(pDX, IDC_HQMODEL, theApp.m_GlobeHQModel);
	DDX_Check(pDX, IDC_LIGHTING, theApp.m_GlobeLighting);
	DDX_Check(pDX, IDC_ATMOSPHERE, theApp.m_GlobeAtmosphere);
	DDX_Check(pDX, IDC_SPOTS, p_View->GlobeShowSpots);
	DDX_Check(pDX, IDC_AIRPORTNAMES, p_View->GlobeShowAirportNames);
	DDX_Check(pDX, IDC_GPSCOORDINATES, p_View->GlobeShowGPS);
	DDX_Check(pDX, IDC_DESCRIPTION, p_View->GlobeShowDescription);
	DDX_Check(pDX, IDC_VIEWPORT, theApp.m_GlobeShowViewport);
	DDX_Check(pDX, IDC_CROSSHAIRS, theApp.m_GlobeShowCrosshairs);

	if (pDX->m_bSaveAndValidate)
		theApp.m_nTextureSize = m_wndTextureSize.GetCurSel();
}


BEGIN_MESSAGE_MAP(GlobeOptionsDlg, LFDialog)
	ON_BN_CLICKED(IDC_VIEWPORT, OnViewport)
END_MESSAGE_MAP()

BOOL GlobeOptionsDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	// Texturgröße
	CString tmpStr((LPCSTR)IDS_AUTOMATIC);
	m_wndTextureSize.AddString(tmpStr);
	m_wndTextureSize.AddString(_T("1024×1024"));
	m_wndTextureSize.AddString(_T("2048×2048"));
	m_wndTextureSize.AddString(_T("4096×4096"));
	m_wndTextureSize.AddString(_T("8192×4096"));
	m_wndTextureSize.SetCurSel(theApp.m_nTextureSize);

	// Inaktive Elemente
	if (p_View->SortBy!=LFAttrLocationIATA)
	{
		GetDlgItem(IDC_AIRPORTNAMES)->EnableWindow(FALSE);
		GetDlgItem(IDC_GPSCOORDINATES)->EnableWindow(FALSE);
	}

	// Fadenkreuz
	OnViewport();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void GlobeOptionsDlg::OnViewport()
{
	GetDlgItem(IDC_CROSSHAIRS)->EnableWindow(m_wndViewport.GetCheck());
}
