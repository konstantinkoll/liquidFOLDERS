
// CSidebar.h: Schnittstelle der Klasse CSidebar
//

#pragma once
#include "LFDynArray.h"


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
	UINT Number;
	COLORREF Color;
};

class CSidebar : public CWnd
{
public:
	CSidebar();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void AdjustLayout();
	virtual CString AppendTooltip(UINT CmdID);

	BOOL Create(CWnd* pParentWnd, UINT nID, UINT LargeIconsID, UINT SmallIconsID, BOOL ShowNumbers);
	void AddCommand(UINT CmdID, INT IconID, WCHAR* Caption, WCHAR* Hint, COLORREF Color=(COLORREF)-1);
	void AddCaption(WCHAR* Caption=NULL);
	void AddCaption(UINT ResID);
	void ResetNumbers();
	void SetNumber(UINT CmdID, UINT Number);
	INT GetPreferredWidth();
	void SetSelection(UINT CmdID=0);

protected:
	void AddItem(BOOL Selectable, UINT CmdID, INT IconID, WCHAR* Caption, WCHAR* Hint, COLORREF Color=(COLORREF)-1);
	INT ItemAtPosition(CPoint point);
	void InvalidateItem(INT Index);
	void SelectItem(INT Index);
	void PrepareBitmaps();

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

	LFDynArray<SidebarItem> m_Items;
	INT m_Width;
	INT m_SelectedItem;
	INT m_HotItem;
	INT m_NumberWidth;
	BOOL m_Hover;
	BOOL m_Keyboard;
	BOOL m_ShowNumbers;
	CIcons m_SmallIcons;
	CIcons m_LargeIcons;
	CIcons* p_Icons;
	INT m_IconSize;
};
