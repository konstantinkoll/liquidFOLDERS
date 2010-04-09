
#pragma once
#include "afxcontrolbarutil.h"
#include "afxbutton.h"
#include "afxtoolbarimages.h"
#include "afxpane.h"


class AFX_EXT_CLASS LFCaptionBar : public CPane
{
public:
	LFCaptionBar();
	virtual ~LFCaptionBar();

	BOOL Create(DWORD dwStyle, CWnd* pParentWnd, UINT uID, int nHeight=-1);

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	afx_msg LRESULT OnUpdateToolTips(WPARAM, LPARAM);
	afx_msg BOOL OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

public:
	enum BarElement
	{
		ELEM_BUTTON,
		ELEM_TEXT,
		ELEM_ICON
	};

	enum BarElementAlignment
	{
		ALIGN_INVALID,
		ALIGN_LEFT,
		ALIGN_RIGHT,
		ALIGN_CENTER
	};

	COLORREF m_clrBarText;
	COLORREF m_clrBarBackground;
	COLORREF m_clrBarBorder;

	virtual BOOL DoesAllowDynInsertBefore() const {return FALSE;}

protected:
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual void OnDrawBackground(CDC* pDC, CRect rect);
	virtual void OnDrawBorder(CDC* pDC, CRect rect);
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual void AdjustLayout();
	virtual void RecalcLayout();
	virtual BOOL OnShowControlBarMenu(CPoint /*point*/) { return FALSE; }

	void UpdateTooltips();

	CToolTipCtrl* m_pToolTip;

	HICON m_hIcon;
	CMFCToolBarImages m_Bitmap;
	BOOL m_bStretchImage;
	BarElementAlignment m_iconAlignment;
	CRect m_rectImage;
	CString m_strImageToolTip;
	CString m_strImageDescription;
	HFONT m_hFont;
	CString m_strText;
	CStringArray m_arTextParts;
	BarElementAlignment m_textAlignment;
	CRect m_rectText;
	CRect m_rectDrawText;
	BOOL m_bTextIsTruncated;
	CString m_strBtnText;
	CString m_strButtonToolTip;
	CString m_strButtonDescription;
	UINT m_uiBtnID;
	BarElementAlignment m_btnAlignnment;
	CRect m_rectButton;
	BOOL m_bIsBtnPressed;
	BOOL m_bIsBtnHighlighted;
	BOOL m_bIsBtnForcePressed;
	BOOL m_bTracked;
	BOOL m_bBtnEnabled;
	BOOL m_bBtnHasDropDownArrow;
	BOOL m_bFlatBorder;

	int m_nBorderSize;
	int m_nMargin;
	int m_nHorzElementOffset;
	int m_nDefaultHeight;
	int m_nCurrentHeight;

	// Close Button attributes
	BOOL m_bIsCloseBtnPressed;
	BOOL m_bIsCloseBtnHighlighted;
	BOOL m_bCloseTracked;
	CRect m_rectClose;

	CRect m_rectInfo;
};
