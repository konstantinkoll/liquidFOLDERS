
// CTagList: Schnittstelle der Klasse CTagList
//

#pragma once


// CTagList
//

class CTagList : public CListCtrl
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
