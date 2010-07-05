
// CTagList: Schnittstelle der Klasse CTagList
//

#pragma once
#include "liquidFOLDERS.h"


// CTagList
//

class AFX_EXT_CLASS CTagList : public CListCtrl
{
public:
	CTagList();
	~CTagList();

protected:
	CFont m_FontLarge;
	CFont m_FontSmall;

	void CreateRoundRectangle(CRect rect, int rad, GraphicsPath& path);
	void DrawItem(int nID, CDC* pDC);

	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
