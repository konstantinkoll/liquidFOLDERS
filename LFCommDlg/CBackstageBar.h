
// CBackstageBar.h: Schnittstelle der Klasse CBackstageBar
//

#pragma once
#include "LFApplication.h"
#include "LFDynArray.h"


// CBackstageBar
//

#define BACKSTAGEICON_MINIMIZE       0
#define BACKSTAGEICON_MAXIMIZE       1
#define BACKSTAGEICON_RESTORE        2
#define BACKSTAGEICON_CLOSE          3
#define BACKSTAGEICON_ARROWLEFT      4
#define BACKSTAGEICON_ARROWRIGHT     5

struct BarItem
{
	UINT Command;
	INT IconID;
	WCHAR Name[256];
	INT PreferredWidth;
	BOOL Red;
	BOOL Enabled;
	INT Width;
	INT Left;
	INT Right;
};

class CBackstageBar : public CWnd
{
public:
	CBackstageBar(BOOL Small=FALSE);

	BOOL Create(CWnd* pParentWnd, UINT nID, INT m_Spacer=0, BOOL ReverseOrder=FALSE);
	static UINT GetPreferredHeight();
	UINT GetPreferredWidth() const;

protected:
	virtual void DrawItem(CDC& dc, CRect& rectItem, UINT Index, UINT State, BOOL Themed);
	virtual void OnClickButton(INT Index) const=0;

	void Reset();
	void AddItem(UINT Command, INT IconID, INT PreferredWidth=0, BOOL Red=FALSE, WCHAR* pName=NULL, BOOL Enabled=TRUE);
	void AdjustLayout();
	INT HitTest(const CPoint& point) const;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	DECLARE_MESSAGE_MAP()

	LFDynArray<BarItem, 2, 2> m_BarItems;
	BOOL m_Small;
	INT m_Hover;
	INT m_Pressed;
	INT m_IconSize;
	INT m_ButtonSize;
	INT m_Spacer;
	BOOL m_ReverseOrder;

private:
	static HBITMAP LoadMaskedIcon(UINT nID, INT Size, COLORREF clr=0xFFFFFF);

	LFFont* p_Font;
};
