
// CIconCtrl.h: Schnittstelle der Klasse CIconCtrl
//

#pragma once


// CIconCtrl
//

class AFX_EXT_CLASS CIconCtrl : public CWnd
{
public:
	CIconCtrl();
	~CIconCtrl();

	void SetIcon(HICON _icon, int _cx, int _cy);
	void SetCoreIcon(UINT nID);

protected:
	HICON m_Icon;
	int m_IconSizeX;
	int m_IconSizeY;

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
