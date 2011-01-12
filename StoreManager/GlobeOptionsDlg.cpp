
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
	DDX_Check(pDX, IDC_HQMODEL, theApp.m_GlobeHQModel);
	DDX_Check(pDX, IDC_LIGHTING, theApp.m_GlobeLighting);
	DDX_Check(pDX, IDC_ATMOSPHERE, theApp.m_GlobeAtmosphere);
	DDX_Check(pDX, IDC_SPOTS, p_View->GlobeShowSpots);
	DDX_Check(pDX, IDC_AIRPORTNAMES, p_View->GlobeShowAirportNames);
	DDX_Check(pDX, IDC_GPSCOORDINATES, p_View->GlobeShowGPS);
	DDX_Check(pDX, IDC_DESCRIPTION, p_View->GlobeShowDescription);
	DDX_Check(pDX, IDC_VIEWPORT, p_View->GlobeShowViewport);
	DDX_Check(pDX, IDC_CROSSHAIR, p_View->GlobeShowCrosshair);
}

BEGIN_MESSAGE_MAP(GlobeOptionsDlg, CDialog)
	//ON_BN_CLICKED(IDC_AUTODIRS, OnSetAttrGroupBox)
END_MESSAGE_MAP()

BOOL GlobeOptionsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_GLOBEOPTIONS);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
