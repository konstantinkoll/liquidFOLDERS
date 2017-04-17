
// CIconCtrl.h: Schnittstelle der Klasse CIconCtrl
//

#pragma once


// CIconCtrl
//

class CIconCtrl : public CWnd
{
public:
	CIconCtrl();

	void SetIcon(HICON hIcon, INT IconSize, BOOL Center=TRUE);
	void SetCoreIcon(UINT nID, BOOL Center=TRUE);
	void SetTaskIcon(HINSTANCE hInst, UINT nID, BOOL Center=FALSE);

protected:
	INT m_IconSize;
	HICON m_hIcon;
	BOOL m_Center;

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
