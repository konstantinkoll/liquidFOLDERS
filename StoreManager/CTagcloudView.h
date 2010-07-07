
// CTagcloudView.h: Schnittstelle der Klasse CTagcloudView
//

#pragma once
#include "liquidFOLDERS.h"
#include "StoreManager.h"
#include "CFileView.h"


struct Tag
{
	int cnt;
	BOOL selected;
	UINT fontsize;
	COLORREF color;
	UINT alpha;
	RECT rect;
};


// CTagcloudView
//

class CTagcloudView : public CFileView
{
public:
	CTagcloudView();
	virtual ~CTagcloudView();

	void Create(CWnd* _pParentWnd, LFSearchResult* _result);
	virtual void SelectItem(int n, BOOL select=TRUE, BOOL InternalCall=FALSE);
	virtual int GetSelectedItem();
	virtual int GetNextSelectedItem(int n);

protected:
	CFont m_Fonts[22];
	Tag* m_Tags;
	HTHEME hTheme;

	virtual void SetViewOptions(UINT _ViewID, BOOL Force);
	virtual void SetSearchResult(LFSearchResult* _result);
	virtual BOOL IsSelected(int n);
	virtual int ItemAtPosition(CPoint point);
	virtual CMenu* GetContextMenu();

	CFont* GetFont(int idx);
	void AdjustLayout();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSortName();
	afx_msg void OnSortCount();
	afx_msg void OnOmitRare();
	afx_msg void OnUseSize();
	afx_msg void OnUseColors();
	afx_msg void OnUseOpacity();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSysColorChange();
	DECLARE_MESSAGE_MAP()
};
