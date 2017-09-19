
// CFrontstagePane: Schnittstelle der Klasse CFrontstagePane
//

#pragma once
#include "CFrontstageWnd.h"


// CFrontstagePane
//

#define PANEGRIPPER     4

class CFrontstagePane : public CFrontstageWnd
{
public:
	CFrontstagePane();

	virtual INT GetMinWidth(INT Height) const;
	virtual void AdjustLayout(CRect rectLayout);

	BOOL Create(CWnd* pParentWnd, UINT nID, BOOL IsLeft, INT PreferredWidth, BOOL Shadow=FALSE);
	INT GetPreferredWidth() const;
	void SetMaxWidth(INT MaxWidth, INT Height);
	void GetLayoutRect(LPRECT lpRect) const;

protected:
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	DECLARE_MESSAGE_MAP()

	BOOL m_IsLeft;
	INT m_PreferredWidth;
	INT m_MaxWidth;
	BOOL m_Shadow;
};

inline INT CFrontstagePane::GetPreferredWidth() const
{
	return m_PreferredWidth;
}
