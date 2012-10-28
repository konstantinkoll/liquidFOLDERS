
// CTooltipList: Schnittstelle der Klasse CTooltipList
//

#pragma once
#include "CExplorerList.h"
#include "LFTooltip.h"


// CTooltipList
//

#define REQUEST_TOOLTIP_DATA     1

struct tagTOOLTIPDATA
{
	NMHDR hdr;
	BOOL Show;
	HICON hIcon;
	INT cx;
	INT cy;
	WCHAR Text[1024];
};

class AFX_EXT_CLASS CTooltipList : public CExplorerList
{
public:
	CTooltipList();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	LFTooltip m_TooltipCtrl;
	BOOL m_Hover;
	INT m_HoverItem;
	INT m_TooltipItem;

	virtual void Init();

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};
