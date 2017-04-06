
// GlobeOptionsDlg.cpp: Implementierung der Klasse GlobeOptionsDlg
//

#include "stdafx.h"
#include "GlobeOptionsDlg.h"


// GlobeOptionsDlg
//

GlobeOptionsDlg::GlobeOptionsDlg(UINT Context, CWnd* pParentWnd)
	: LFDialog(IDD_GLOBEOPTIONS, pParentWnd)
{
	p_ContextViewSettings = &theApp.m_ContextViewSettings[m_Context=Context];
}

void GlobeOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MODELQUALITY, m_wndModelQuality);
	DDX_Control(pDX, IDC_TEXTUREQUALITY, m_wndTextureQuality);

	DDX_Check(pDX, IDC_TEXTURECOMPRESS, theApp.m_TextureCompress);
	DDX_Check(pDX, IDC_SPOTS, theApp.m_GlobalViewSettings.GlobeShowSpots);
	DDX_Check(pDX, IDC_AIRPORTNAMES, theApp.m_GlobalViewSettings.GlobeShowAirportNames);
	DDX_Check(pDX, IDC_GPSCOORDINATES, theApp.m_GlobalViewSettings.GlobeShowGPS);
	DDX_Check(pDX, IDC_DESCRIPTION, theApp.m_GlobalViewSettings.GlobeShowDescription);

	if (pDX->m_bSaveAndValidate)
	{
		theApp.m_ModelQuality = (GLModelQuality)m_wndModelQuality.GetCurSel();
		theApp.m_TextureQuality = (GLTextureQuality)m_wndTextureQuality.GetCurSel();
	}
}

void GlobeOptionsDlg::AddQuality(CComboBox& wndCombobox, UINT nResID)
{
	wndCombobox.AddString(CString((LPCSTR)nResID));
}

BOOL GlobeOptionsDlg::InitDialog()
{
	// Initialize OpenGL
	theRenderer.Initialize();

	// 3D model
	AddQuality(m_wndModelQuality, IDS_QUALITY_LOW);

	if (theRenderer.m_MaxModelQuality>=MODELMEDIUM)
		AddQuality(m_wndModelQuality, IDS_QUALITY_MEDIUM);

	if (theRenderer.m_MaxModelQuality>=MODELHIGH)
		AddQuality(m_wndModelQuality, IDS_QUALITY_HIGH);

	if (theRenderer.m_MaxModelQuality>=MODELULTRA)
		AddQuality(m_wndModelQuality, IDS_QUALITY_ULTRA);

	m_wndModelQuality.SetCurSel(min((INT)theApp.m_ModelQuality, (INT)theRenderer.m_MaxModelQuality));

	// Texture
	AddQuality(m_wndTextureQuality, IDS_QUALITY_LOW);

	if (theRenderer.m_MaxTextureQuality>=TEXTUREMEDIUM)
		AddQuality(m_wndTextureQuality, IDS_QUALITY_MEDIUM);

	if (theRenderer.m_MaxTextureQuality>=TEXTUREULTRA)
		AddQuality(m_wndTextureQuality, IDS_QUALITY_ULTRA);

	m_wndTextureQuality.SetCurSel(min((INT)theApp.m_TextureQuality, (INT)theRenderer.m_MaxTextureQuality));

	// Disabled controls
	GetDlgItem(IDC_TEXTURECOMPRESS)->EnableWindow(theRenderer.m_SupportsTextureCompression);

	if (p_ContextViewSettings->SortBy!=LFAttrLocationIATA)
	{
		GetDlgItem(IDC_AIRPORTNAMES)->EnableWindow(FALSE);
		GetDlgItem(IDC_GPSCOORDINATES)->EnableWindow(FALSE);
	}

	return TRUE;
}
