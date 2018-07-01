
// CFrontstageScroller: Schnittstelle der Klasse CFrontstageScroller
//

#pragma once
#include "CFrontstageWnd.h"
#include "CTooltipHeader.h"


// CFrontstageScroller
//

#define FRONTSTAGE_WHITEBACKGROUND       0x00000000
#define FRONTSTAGE_COMPLEXBACKGROUND     0x00000001
#define FRONTSTAGE_CARDBACKGROUND        0x00000003

#define DEFAULTSCROLLSTEP     150

class CFrontstageScroller : public CFrontstageWnd
{
public:
	CFrontstageScroller(UINT Flags=FRONTSTAGE_CARDBACKGROUND);

protected:
	virtual INT GetHeaderIndent() const;
	virtual void GetHeaderContextMenu(CMenu& Menu, INT HeaderItem);
	virtual void AdjustScrollbars();
	virtual void AdjustLayout();
	virtual void GetNothingMessage(CString& strMessage, COLORREF& clrMessage, BOOL Themed) const;
	virtual BOOL DrawNothing() const;
	virtual void DrawNothing(CDC& dc, CRect rect, BOOL Themed) const;
	virtual void DrawStage(CDC& dc, Graphics& g, const CRect& rect, const CRect& rectUpdate, BOOL Themed);
	virtual void ScrollWindow(INT dx, INT dy, LPCRECT lpRect=NULL, LPCRECT lpClipRect=NULL);

	void ResetScrollArea();
	void SetItemHeight(INT ItemHeight, INT Gutter=-1);

	afx_msg void OnDestroy();
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt);
	afx_msg void OnMouseHWheel(UINT nFlags, SHORT zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	DECLARE_MESSAGE_MAP()

	UINT m_Flags;

	UINT m_HeaderHeight;
	INT m_ScrollWidth;
	INT m_ScrollHeight;
	INT m_HScrollPos;
	INT m_VScrollPos;
	INT m_HScrollMax;
	INT m_VScrollMax;
	INT m_ItemHeight;
	CSize m_szScrollStep;

	CTooltipHeader* m_pWndHeader;

private:
};
