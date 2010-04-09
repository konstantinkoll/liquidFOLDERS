
// CCaptionBar.cpp: Implementierung der Klasse CCaptionBar
//

#include "stdafx.h"
#include "CCaptionBar.h"


// CCaptionBar

CCaptionBar::CCaptionBar()
	: CMFCCaptionBar()
{
}

CCaptionBar::~CCaptionBar()
{
}

BOOL CCaptionBar::LoadState(LPCTSTR /*lpszProfileName*/, int /*nIndex*/, UINT /*uiID*/)
{
	return TRUE;
}

BOOL CCaptionBar::SaveState(LPCTSTR /*lpszProfileName*/, int /*nIndex*/, UINT /*uiID*/)
{
	return TRUE;
}
