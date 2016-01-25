
// GlobeOptionsDlg.cpp: Implementierung der Klasse GlobeOptionsDlg
//

#include "stdafx.h"
#include "GlobeOptionsDlg.h"


// GlobeOptionsDlg
//

GlobeOptionsDlg::GlobeOptionsDlg(LFViewParameters* pViewParameters, CWnd* pParentWnd)
	: LFDialog(IDD_GLOBEOPTIONS, pParentWnd)
{
	ASSERT(pViewParameters);

	p_ViewParameters = pViewParameters;
}

void GlobeOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MODELQUALITY, m_wndModelQuality);
	DDX_Control(pDX, IDC_TEXTUREQUALITY, m_wndTextureQuality);

	DDX_Check(pDX, IDC_TEXTURECOMPRESS, theApp.m_TextureCompress);
	DDX_Check(pDX, IDC_SPOTS, p_ViewParameters->GlobeShowSpots);
	DDX_Check(pDX, IDC_AIRPORTNAMES, p_ViewParameters->GlobeShowAirportNames);
	DDX_Check(pDX, IDC_GPSCOORDINATES, p_ViewParameters->GlobeShowGPS);
	DDX_Check(pDX, IDC_DESCRIPTION, p_ViewParameters->GlobeShowDescription);

	if (pDX->m_bSaveAndValidate)
	{
		theApp.m_ModelQuality = (GLModelQuality)m_wndModelQuality.GetCurSel();
		theApp.m_TextureQuality = (GLTextureQuality)m_wndTextureQuality.GetCurSel();
	}
}

void GlobeOptionsDlg::AddQuality(CComboBox& wndCombobox, UINT nResID)
{
	CString tmpStr((LPCSTR)nResID);
	wndCombobox.AddString(tmpStr);
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

	if (p_ViewParameters->SortBy!=LFAttrLocationIATA)
	{
		GetDlgItem(IDC_AIRPORTNAMES)->EnableWindow(FALSE);
		GetDlgItem(IDC_GPSCOORDINATES)->EnableWindow(FALSE);
	}

	return TRUE;
}
