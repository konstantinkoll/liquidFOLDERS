
// CSidebar.h: Schnittstelle der Klasse CSidebar
//

#pragma once
#include "LFApplication.h"
#include "DynArray.h"


// CSidebar
//

struct SidebarItem
{
	RECT Rect;
	BOOL Selectable;
	UINT CmdID;
	INT IconID;
	WCHAR Caption[256];
	WCHAR Hint[256];
	INT Height;
};

class AFX_EXT_CLASS CSidebar : public CWnd
{
friend class CSidebarCommand;

public:
	CSidebar();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void AdjustLayout();

	BOOL Create(CWnd* pParentWnd, UINT nID, UINT LargeIconsID, UINT SmallIconsID);
	void AddCommand(UINT CmdID, INT IconID, WCHAR* Caption, WCHAR* Hint);
	void AddCaption(WCHAR* Caption=NULL);
	void AddCaption(UINT ResID);
	INT GetPreferredWidth();
	void Reset(UINT CmdID=0);

protected:
	LFApplication* p_App;
	LFTooltip m_TooltipCtrl;
	DynArray<SidebarItem> m_Items;
	INT m_Width;
	INT m_SelectedItem;
	INT m_HotItem;
	BOOL m_Hover;
	BOOL m_Keyboard;
	CMFCToolBarImages m_SmallIcons;
	CMFCToolBarImages m_LargeIcons;

	void AddItem(BOOL Selectable, UINT CmdID, INT IconID, WCHAR* Caption, WCHAR* Hint);
	INT ItemAtPosition(CPoint point);
	void InvalidateItem(INT idx);
	void SelectItem(INT idx);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	DECLARE_MESSAGE_MAP()
};
