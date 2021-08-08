
// AboutDlg.cpp: Implementierung der Klasse AboutDlg
//

#include "stdafx.h"
#include "AboutDlg.h"
#include "liquidFOLDERS.h"


// AboutDlg
//

AboutDlg::AboutDlg(CWnd* pParentWnd)
	: LFAboutDialog(0x0005, pParentWnd)
{
}

void AboutDlg::DoDataExchange(CDataExchange* pDX)
{
	LFAboutDialog::DoDataExchange(pDX);

	// Start with
	DDX_Control(pDX, IDC_STARTWITH, m_wndStartWith);

	// 3D settings
	DDX_Control(pDX, IDC_MODELQUALITY, m_wndModelQuality);
	DDX_Control(pDX, IDC_TEXTUREQUALITY, m_wndTextureQuality);
	DDX_Check(pDX, IDC_TEXTURECOMPRESS, theApp.m_TextureCompress);

	if (pDX->m_bSaveAndValidate)
	{
		theApp.m_StartWith = m_wndStartWith.GetCurSel();

		theApp.m_ModelQuality = (GLModelQuality)m_wndModelQuality.GetCurSel();
		theApp.m_TextureQuality = (GLTextureQuality)m_wndTextureQuality.GetCurSel();
	}

	// Icons
	DDX_Check(pDX, IDM_ICONS_SHOWCAPACITY, theApp.m_GlobalViewSettings.IconsShowCapacity);

	// Calendar
	DDX_Check(pDX, IDM_CALENDAR_SHOWDAYS, theApp.m_GlobalViewSettings.CalendarShowDays);

	// Globe
	DDX_Check(pDX, IDM_GLOBE_SHOWLOCATIONS, theApp.m_GlobalViewSettings.GlobeShowLocations);
	DDX_Check(pDX, IDM_GLOBE_SHOWAIRPORTNAMES, theApp.m_GlobalViewSettings.GlobeShowAirportNames);
	DDX_Check(pDX, IDM_GLOBE_SHOWCOORDINATES, theApp.m_GlobalViewSettings.GlobeShowCoordinates);
	DDX_Check(pDX, IDM_GLOBE_SHOWDESCRIPTIONS, theApp.m_GlobalViewSettings.GlobeShowDescriptions);

	// Tag cloud
	DDX_Radio(pDX, IDM_TAGCLOUD_SORTVALUE, theApp.m_GlobalViewSettings.TagcloudSort);
	DDX_Check(pDX, IDM_TAGCLOUD_SHOWRARE, theApp.m_GlobalViewSettings.TagcloudShowRare);
	DDX_Check(pDX, IDM_TAGCLOUD_USESIZE, theApp.m_GlobalViewSettings.TagcloudUseSize);
	DDX_Check(pDX, IDM_TAGCLOUD_USECOLOR, theApp.m_GlobalViewSettings.TagcloudUseColor);
	DDX_Check(pDX, IDM_TAGCLOUD_USEOPACITY, theApp.m_GlobalViewSettings.TagcloudUseOpacity);
}

BOOL AboutDlg::InitSidebar(LPSIZE pszTabArea)
{
	if (!LFTabbedDialog::InitSidebar(pszTabArea))
		return FALSE;

	AddTab(IDD_ABOUT_GENERAL, pszTabArea);
	AddTab(IDD_ABOUT_VIEWS, pszTabArea);

	return TRUE;
}

void AboutDlg::AddContextName(CComboBox& wndCombobox, UINT ContextID)
{
	ASSERT(ContextID<LFContextCount);

	wndCombobox.AddString(CString(theApp.m_Contexts[ContextID].Name));
}

void AboutDlg::AddQualityString(CComboBox& wndCombobox, UINT nResID)
{
	wndCombobox.AddString(CString((LPCSTR)nResID));
}

BOOL AboutDlg::InitDialog()
{
	// Start with
	AddContextName(m_wndStartWith, LFContextStores);
	AddContextName(m_wndStartWith, LFContextAllFiles);
	AddContextName(m_wndStartWith, LFContextFavorites);
	AddContextName(m_wndStartWith, LFContextTasks);
	AddContextName(m_wndStartWith, LFContextNew);

	m_wndStartWith.SetCurSel(theApp.m_StartWith);

	// Initialize OpenGL
	theRenderer.Initialize();

	// 3D model
	AddQualityString(m_wndModelQuality, IDS_QUALITY_LOW);

	if (theRenderer.m_MaxModelQuality>=MODELMEDIUM)
		AddQualityString(m_wndModelQuality, IDS_QUALITY_MEDIUM);

	if (theRenderer.m_MaxModelQuality>=MODELHIGH)
		AddQualityString(m_wndModelQuality, IDS_QUALITY_HIGH);

	if (theRenderer.m_MaxModelQuality>=MODELULTRA)
		AddQualityString(m_wndModelQuality, IDS_QUALITY_ULTRA);

	m_wndModelQuality.SetCurSel(min((INT)theApp.m_ModelQuality, (INT)theRenderer.m_MaxModelQuality));

	// Texture
	AddQualityString(m_wndTextureQuality, IDS_QUALITY_LOW);

	if (theRenderer.m_MaxTextureQuality>=TEXTUREMEDIUM)
		AddQualityString(m_wndTextureQuality, IDS_QUALITY_MEDIUM);

	if (theRenderer.m_MaxTextureQuality>=TEXTUREULTRA)
		AddQualityString(m_wndTextureQuality, IDS_QUALITY_ULTRA);

	m_wndTextureQuality.SetCurSel(min((INT)theApp.m_TextureQuality, (INT)theRenderer.m_MaxTextureQuality));

	// Disabled controls
	GetDlgItem(IDC_TEXTURECOMPRESS)->EnableWindow(theRenderer.m_SupportsTextureCompression);

	return LFAboutDialog::InitDialog();
}
