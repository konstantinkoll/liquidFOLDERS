
// LFTooltip.h: Schnittstelle der Klasse LFTooltip
//

#pragma once


// LFTooltip
//

#define LFHOVERTIME     850

class LFTooltip : public CWnd
{
public:
	BOOL Create();

	void ShowTooltip(CPoint point, const CString& strCaption, const CString& strText, HICON hIcon=NULL, HBITMAP hBitmap=NULL);
	void HideTooltip();
};
