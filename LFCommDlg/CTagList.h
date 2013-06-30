
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
	void DrawItem(INT nID, CDC* pDC);

	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSysColorChange();
	DECLARE_MESSAGE_MAP()

private:
	GraphicsPath m_Path;
	CBitmap* m_BgBitmaps[2];
};
