
// CHeaderArea.h: Schnittstelle der Klasse CHeaderArea
//

#pragma once
#include "CFrontstageWnd.h"
#include "CHeaderButton.h"
#include "LFDynArray.h"


// CHeaderArea
//

class CHeaderArea : public CFrontstageWnd
{
public:
	CHeaderArea();

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	BOOL Create(CWnd* pParentWnd, UINT nID, BOOL Shadow=FALSE);
	void SetHeader(LPCWSTR Caption=L"", LPCWSTR Hint=L"", HBITMAP hBitmap=NULL, const CPoint& m_BitmapOffset=CPoint(0, 0), BOOL Repaint=TRUE);
	UINT GetPreferredHeight() const;
	CHeaderButton* AddButton(UINT nID=0);

protected:
	void AdjustLayout();

	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSysColorChange();
	afx_msg LRESULT OnThemeChanged();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnIdleUpdateCmdUI();
	afx_msg void OnAdjustLayout();
	DECLARE_MESSAGE_MAP()

	BOOL m_Shadow;
	LFDynArray<CHeaderButton*, 2, 2> m_Buttons;
	CString m_Caption;
	CString m_Hint;
	HBITMAP hIconBitmap;
	INT m_BitmapWidth;
	INT m_BitmapHeight;
	CPoint m_BitmapOffset;
	INT m_RightEdge;

private:
	UINT GetBitmapMinHeight() const;
	UINT GetTextMinHeight() const;
	UINT GetButtonHeight() const;
	UINT GetButtonMinHeight() const;

	INT m_BackBufferL;
	INT m_BackBufferH;
	HBRUSH hBackgroundBrush;
};
