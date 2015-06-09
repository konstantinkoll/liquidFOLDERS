
// LFDialog.h: Schnittstelle der Klasse LFDialog
//


#pragma once
#include "CCategory.h"
#include "CGroupBox.h"
#include "LFApplication.h"


// LFDialog
//

class LFDialog : public CDialog
{
public:
	LFDialog(UINT nIDTemplate, CWnd* pParentWnd=NULL, BOOL UAC=FALSE);

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void AdjustLayout();

	void GetLayoutRect(LPRECT lpRect) const;

protected:
	LFApplication* p_App;
	CCategory m_wndCategory[4];
	UINT m_nIDTemplate;
	BOOL m_UAC;

	virtual void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);

	CWnd* GetBottomWnd() const;
	void SetBottomLeftControl(CWnd* pChildWnd);
	void SetBottomLeftControl(UINT nID);
	void AddBottomRightControl(CWnd* pChildWnd);
	void AddBottomRightControl(UINT nID);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnSysColorChange();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	DECLARE_MESSAGE_MAP()

private:
	CGroupBox m_wndGroupBox[3];
	HICON hIconShield;
	INT m_ShieldSize;
	INT m_UACHeight;
	CBitmap m_BackBuffer;
	INT m_BackBufferL;
	INT m_BackBufferH;
	HBRUSH hBackgroundBrush;
	CWnd* p_BottomLeftControl;
	CList<CWnd*> m_BottomRightControls;
	CPoint m_LastSize;
};
