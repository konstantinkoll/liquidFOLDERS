
// GlobeOptionsDlg.cpp: Implementierung der Klasse GlobeOptionsDlg
//

#include "stdafx.h"
#include "GlobeOptionsDlg.h"


// GlobeOptionsDlg
//

GlobeOptionsDlg::GlobeOptionsDlg(CWnd* pParent, LFViewParameters* View, UINT Context)
	: CDialog(IDD_GLOBEOPTIONS, pParent)
{
	ASSERT(View);
	p_View = View;
	m_Context = Context;
}

void GlobeOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_TEXTURESIZE, m_wndTextureSize);
	DDX_Control(pDX, IDC_VIEWPORT, m_wndViewport);

	DDX_Check(pDX, IDC_HQMODEL, theApp.m_GlobeHQModel);
	DDX_Check(pDX, IDC_LIGHTING, theApp.m_GlobeLighting);
	DDX_Check(pDX, IDC_ATMOSPHERE, theApp.m_GlobeAtmosphere);
	DDX_Check(pDX, IDC_SPOTS, p_View->GlobeShowSpots);
	DDX_Check(pDX, IDC_AIRPORTNAMES, p_View->GlobeShowAirportNames);
	DDX_Check(pDX, IDC_GPSCOORDINATES, p_View->GlobeShowGPS);
	DDX_Check(pDX, IDC_DESCRIPTION, p_View->GlobeShowDescription);
	DDX_Check(pDX, IDC_VIEWPORT, p_View->GlobeShowViewport);
	DDX_Check(pDX, IDC_CROSSHAIR, p_View->GlobeShowCrosshair);

	if (pDX->m_bSaveAndValidate)
		theApp.m_nTextureSize = m_wndTextureSize.GetCurSel();
}


BEGIN_MESSAGE_MAP(GlobeOptionsDlg, CDialog)
	ON_BN_CLICKED(IDC_VIEWPORT, OnViewport)
END_MESSAGE_MAP()

BOOL GlobeOptionsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_GLOBEOPTIONS);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Texturgröße
	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_AUTOMATIC));
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
	GetDlgItem(IDC_CROSSHAIR)->EnableWindow(m_wndViewport.GetCheck());
}
