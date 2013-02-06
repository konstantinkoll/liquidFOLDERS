
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

	void SetIcon(HICON hIcon, INT cx, INT cy);
	void SetCoreIcon(UINT nID);
	void SetSmallIcon(HINSTANCE hInst, UINT nID);

protected:
	HICON m_Icon;
	INT m_IconSizeX;
	INT m_IconSizeY;

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
