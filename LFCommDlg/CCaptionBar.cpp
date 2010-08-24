
// CCaptionBar.cpp: Implementierung der Klasse CCaptionBar
//

#include "stdafx.h"
#include "CCaptionBar.h"


// CCaptionBar
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

CCaptionBar::CCaptionBar()
	: CMFCCaptionBar()
{
}

CCaptionBar::~CCaptionBar()
{
	if (m_hIcon)
		DestroyIcon(m_hIcon);
}

BOOL CCaptionBar::LoadState(LPCTSTR /*lpszProfileName*/, int /*nIndex*/, UINT /*uiID*/)
{
	return TRUE;
}

BOOL CCaptionBar::SaveState(LPCTSTR /*lpszProfileName*/, int /*nIndex*/, UINT /*uiID*/)
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

	CMFCCaptionBar::SetIcon((HICON)LoadImage(LFCommDlgDLL.hResource, Icon, IMAGE_ICON, 24, 24, LR_DEFAULTCOLOR), iconAlignment);
}
