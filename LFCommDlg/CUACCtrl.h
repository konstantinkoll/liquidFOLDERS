
// CUACCtrl.h: Schnittstelle der Klasse CUACCtrl
//

#pragma once
#include "liquidFOLDERS.h"
#include "CGdiPlusBitmap.h"


// CUACCtrl
//

class CUACCtrl : public CWnd
{
public:
	CUACCtrl();
	virtual ~CUACCtrl();

	void Create(CRect &rect, CWnd* pParentWnd, UINT nID);

protected:
	HICON m_hIcon;
	CFont m_Font;
	UINT m_Border;
	UINT m_IconSz;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
