
// CCaptionBar.cpp: Implementierung der Klasse CCaptionBar
//

#include "stdafx.h"
#include "CCaptionBar.h"


// CCaptionBar
//

CCaptionBar::CCaptionBar()
	: CMFCCaptionBar()
{
}

CCaptionBar::~CCaptionBar()
{
	if (m_hIcon)
		DestroyIcon(m_hIcon);
}

BOOL CCaptionBar::LoadState(LPCTSTR /*lpszProfileName*/, INT /*nIndex*/, UINT /*uiID*/)
{
	return TRUE;
}

BOOL CCaptionBar::SaveState(LPCTSTR /*lpszProfileName*/, INT /*nIndex*/, UINT /*uiID*/)
{
	return TRUE;
}

void CCaptionBar::SetIcon(LPCWSTR Icon, BarElementAlignment iconAlignment)
{
	if (m_hIcon)
	{
		DestroyIcon(m_hIcon);
		m_hIcon = NULL;
	}

	HICON hIcon = NULL;
	HINSTANCE hModIcons = LoadLibrary(_T("LFCOMMDLG.DLL"));
	if (hModIcons)
	{
		hIcon = (HICON)LoadImage(hModIcons, Icon, IMAGE_ICON, 24, 24, LR_DEFAULTCOLOR);
		FreeLibrary(hModIcons);
	}

	CMFCCaptionBar::SetIcon(hIcon, iconAlignment);
}
