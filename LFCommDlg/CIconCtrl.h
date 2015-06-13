
// CIconCtrl.h: Schnittstelle der Klasse CIconCtrl
//

#pragma once


// CIconCtrl
//

class CIconCtrl : public CWnd
{
public:
	CIconCtrl();
	~CIconCtrl();

	void SetIcon(HICON hIcon, INT cx, INT cy, BOOL Center=TRUE);
	void SetCoreIcon(UINT nID, BOOL Center=TRUE);
	void SetSmallIcon(HINSTANCE hInst, UINT nID, BOOL Center=FALSE);

protected:
	HICON m_Icon;
	INT m_IconSizeX;
	INT m_IconSizeY;
	BOOL m_Center;

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
