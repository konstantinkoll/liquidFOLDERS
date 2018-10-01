
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
	virtual BOOL AllowHeaderColumnDrag(UINT Attr) const;
	virtual BOOL AllowHeaderColumnTrack(UINT Attr) const;
	virtual void UpdateHeaderColumnOrder(UINT Attr, INT Position);
	virtual void UpdateHeaderColumnWidth(UINT Attr, INT Width);
	virtual void UpdateHeaderColumn(UINT Attr, HDITEM& HeaderItem) const;
	virtual void HeaderColumnClicked(UINT Attr);
	virtual void AdjustScrollbars();
	virtual void AdjustLayout();
	virtual void GetNothingMessage(CString& strMessage, COLORREF& clrMessage, BOOL Themed) const;
	virtual BOOL DrawNothing() const;
	virtual void DrawNothing(CDC& dc, CRect rect, BOOL Themed) const;
	virtual void DrawStage(CDC& dc, Graphics& g, const CRect& rect, const CRect& rectUpdate, BOOL Themed);
	virtual void ScrollWindow(INT dx, INT dy, LPCRECT lpRect=NULL, LPCRECT lpClipRect=NULL);

	void ResetScrollArea();
	void SetItemHeight(INT ItemHeight);
	BOOL HasHeader() const;
	BOOL IsHeaderVisible() const;
	BOOL AddHeaderColumn(LPCWSTR Caption=L"", BOOL Right=FALSE);
	BOOL AddHeaderColumn(UINT nID, BOOL Right=FALSE);
	void UpdateHeaderColumnOrder(UINT Attr, INT Position, INT* pColumnOrder, INT* pColumnWidths);
	void UpdateHeader(INT* pColumnOrder, INT* pColumnWidths, BOOL bShowHeader=TRUE, INT PreviewAttribute=-1);

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

	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginTrack(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemClick(NMHDR* pNMHDR, LRESULT* pResult);
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

	INT m_HeaderItemClicked;

private:
	void HideHeader() const;
	void ShowHeader() const;

	CTooltipHeader* m_pWndHeader;
	BOOL m_IgnoreHeaderItemChange;
};

inline BOOL CFrontstageScroller::HasHeader() const
{
	return m_pWndHeader!=NULL;
}

inline BOOL CFrontstageScroller::IsHeaderVisible() const
{
	return HasHeader() && !(m_pWndHeader->GetStyle() & HDS_HIDDEN);
}

inline BOOL CFrontstageScroller::AddHeaderColumn(UINT nID, BOOL Right)
{
	return AddHeaderColumn(CString((LPCSTR)nID), Right);
}

inline void CFrontstageScroller::HideHeader() const
{
	ASSERT(HasHeader());

	m_pWndHeader->ModifyStyle(0, HDS_HIDDEN);
}

inline void CFrontstageScroller::ShowHeader() const
{
	ASSERT(HasHeader());

	m_pWndHeader->ModifyStyle(HDS_HIDDEN, 0);
}
