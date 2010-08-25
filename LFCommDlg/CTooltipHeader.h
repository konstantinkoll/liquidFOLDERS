
// LFTooltipHeader: Schnittstelle der Klasse LFTooltipHeader
//

#pragma once
#include "afxpropertygridtooltipctrl.h"
#include "LFTooltip.h"


// LFTooltipHeader
//

class AFX_EXT_CLASS CTooltipHeader : public CHeaderCtrl
{
public:
	CTooltipHeader();
	~CTooltipHeader();

	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

protected:
	LFTooltip Tooltip;

	void TrackToolTip(CPoint point);

private:
	wchar_t TooltipText[256];
	int Tracking;
	BOOL MouseInWnd;
};
