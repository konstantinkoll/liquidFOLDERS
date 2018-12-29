
// CFrontstageWnd: Schnittstelle der Klasse CFrontstageWnd
//

#pragma once


// CFrontstageWnd
//

#define CARDPADDING     7
#define HOVERTIME       850

#define TRACKMOUSE(Flags) \
	TRACKMOUSEEVENT tme; \
	tme.cbSize = sizeof(TRACKMOUSEEVENT); \
	tme.dwFlags = Flags; \
	tme.dwHoverTime = HOVERTIME; \
	tme.hwndTrack = m_hWnd; \
	TrackMouseEvent(&tme);

#define DECLARE_TOOLTIP() \
	public: \
	virtual BOOL PreTranslateMessage(MSG* pMsg); \
	protected: \
	virtual INT ItemAtPosition(CPoint point) const; \
	virtual CPoint PointAtPosition(CPoint point) const; \
	virtual LPCVOID PtrAtPosition(CPoint point) const; \
	virtual void InvalidateItem(INT Index); \
	virtual void InvalidatePoint(const CPoint& point); \
	virtual void InvalidatePtr(LPCVOID Ptr); \
	virtual void ShowTooltip(const CPoint& point); \
	void HideTooltip(); \
	void UpdateHoverItem(); \
	static BOOL IsPointValid(const CPoint& point); \
	afx_msg void OnMouseMove(UINT nFlags, CPoint point); \
	afx_msg void OnMouseHover(UINT nFlags, CPoint point); \
	afx_msg void OnMouseLeave(); \
	INT m_HoverItem; \
	CPoint m_HoverPoint; \
	LPCVOID m_HoverPtr; \
	private: \
	BOOL m_Hover;

#define CONSTRUCTOR_TOOLTIP() \
	m_Hover = FALSE; \
	m_HoverItem = m_HoverPoint.x = m_HoverPoint.y = -1; \
	m_HoverPtr = NULL;

#define TOOLTIP_NOWHEEL 

#define TOOLTIP_WHEEL \
	case WM_MOUSEWHEEL: \
	case WM_MOUSEHWHEEL:

#define IMPLEMENT_TOOLTIP_NOWHEEL(theClass, baseClass) IMPLEMENT_TOOLTIP(theClass, baseClass, TOOLTIP_NOWHEEL)
#define IMPLEMENT_TOOLTIP_WHEEL(theClass, baseClass) IMPLEMENT_TOOLTIP(theClass, baseClass, TOOLTIP_WHEEL)

#define IMPLEMENT_TOOLTIP(theClass, baseClass, MouseWheel) \
	BOOL theClass::PreTranslateMessage(MSG* pMsg) \
	{ \
	switch (pMsg->message) \
	{ \
	case WM_LBUTTONDOWN: \
	case WM_RBUTTONDOWN: \
	case WM_MBUTTONDOWN: \
	case WM_LBUTTONUP: \
	case WM_RBUTTONUP: \
	case WM_MBUTTONUP: \
	case WM_NCLBUTTONDOWN: \
	case WM_NCRBUTTONDOWN: \
	case WM_NCMBUTTONDOWN: \
	case WM_NCLBUTTONUP: \
	case WM_NCRBUTTONUP: \
	case WM_NCMBUTTONUP: \
	MouseWheel \
		HideTooltip(); \
		break; \
	} \
	return baseClass::PreTranslateMessage(pMsg); \
	} \
	void theClass::HideTooltip() \
	{ \
		LFGetApp()->HideTooltip(this); \
		m_Hover = FALSE; \
	} \
	inline BOOL theClass::IsPointValid(const CPoint& point) \
	{ \
	return (point.x!=-1) && (point.y!=-1); \
	} \
	void theClass::OnMouseMove(UINT nFlags, CPoint point) \
	{ \
	const INT HoverItem = ItemAtPosition(point); \
	CPoint HoverPoint = PointAtPosition(point); \
	const LPCVOID HoverPtr = PtrAtPosition(point); \
	if (!m_Hover) \
	{ \
		m_Hover = TRUE; \
		TRACKMOUSE(TME_HOVER | TME_LEAVE); \
	} \
	else \
	{ \
		if ((HoverItem!=m_HoverItem) || (HoverPoint!=m_HoverPoint) || (HoverPtr!=m_HoverPtr)) \
			HideTooltip(); \
	} \
	if (HoverItem!=m_HoverItem) \
	{ \
		InvalidateItem(m_HoverItem); \
		InvalidateItem(m_HoverItem=HoverItem); \
	} \
	if (HoverPoint!=m_HoverPoint) \
	{ \
		InvalidatePoint(m_HoverPoint); \
		InvalidatePoint(m_HoverPoint=HoverPoint); \
	} \
	if (HoverPtr!=m_HoverPtr) \
	{ \
		InvalidatePtr(m_HoverPtr); \
		InvalidatePtr(m_HoverPtr=HoverPtr); \
	} \
	baseClass::OnMouseMove(nFlags, point); \
	} \
	void theClass::OnMouseHover(UINT nFlags, CPoint point) \
	{ \
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0) \
	{ \
		if ((m_HoverItem!=-1) || IsPointValid(m_HoverPoint) || m_HoverPtr) \
			ShowTooltip(point); \
	} \
	else \
	{ \
		HideTooltip(); \
	} \
	TRACKMOUSE(TME_LEAVE); \
	} \
	void theClass::OnMouseLeave() \
	{ \
	HideTooltip(); \
	InvalidateItem(m_HoverItem); \
	InvalidatePoint(m_HoverPoint); \
	InvalidatePtr(m_HoverPtr); \
	CONSTRUCTOR_TOOLTIP() \
	baseClass::OnMouseLeave(); \
	}

#define BEGIN_TOOLTIP_MAP(theClass, baseClass) \
	BEGIN_MESSAGE_MAP(theClass, baseClass) \
	ON_WM_MOUSEMOVE() \
	ON_WM_MOUSEHOVER() \
	ON_WM_MOUSELEAVE()

#define END_TOOLTIP_MAP() END_MESSAGE_MAP()

class CFrontstageWnd : public CWnd
{
public:
	CFrontstageWnd();

	void DrawWindowEdge(Graphics& g, BOOL Themed) const;
	void DrawWindowEdge(CDC& dc, BOOL Themed) const;

protected:
	virtual BOOL GetContextMenu(CMenu& Menu, INT Index);
	virtual BOOL GetContextMenu(CMenu& Menu, LPCVOID Ptr);
	virtual BOOL GetContextMenu(CMenu& Menu, const CPoint& point);

	BOOL HasBorder() const;
	void DrawCardBackground(CDC& dc, Graphics& g, LPCRECT lpRect, BOOL Themed) const;
	void DrawCardForeground(CDC& dc, Graphics& g, LPCRECT lpRect, BOOL Themed, BOOL Hot=FALSE, BOOL Focused=FALSE, BOOL Selected=FALSE, COLORREF TextColor=(COLORREF)-1, BOOL ShowFocusRect=TRUE) const;
	void TrackPopupMenu(CMenu& Menu, const CPoint& pos, CWnd* pWndOwner, BOOL SetDefaultItem=TRUE, BOOL AlignRight=FALSE) const;
	void TrackPopupMenu(CMenu& Menu, const CPoint& pos, BOOL SetDefaultItem=TRUE, BOOL AlignRight=FALSE) const;

	afx_msg void OnDestroy();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg LRESULT OnNcPaint(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnInitMenuPopup(CMenu* pMenuPopup, UINT nIndex, BOOL bSysMenu);
	DECLARE_MESSAGE_MAP()

	DECLARE_TOOLTIP()
};

inline BOOL CFrontstageWnd::HasBorder() const
{
	return ((GetStyle() & (WS_CHILD | WS_BORDER))==(WS_CHILD | WS_BORDER)) || (GetExStyle() & WS_EX_CLIENTEDGE);
}

inline void CFrontstageWnd::TrackPopupMenu(CMenu& Menu, const CPoint& pos, BOOL SetDefaultItem, BOOL AlignRight) const
{
	TrackPopupMenu(Menu, pos, GetOwner(), SetDefaultItem, AlignRight);
}
