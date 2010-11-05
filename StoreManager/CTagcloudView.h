
// CTagcloudView.h: Schnittstelle der Klasse CTagcloudView
//

#pragma once
#include "liquidFOLDERS.h"
#include "StoreManager.h"
#include "CFileView.h"


struct Tag
{
	INT cnt;
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

	virtual void SelectItem(INT n, BOOL select=TRUE, BOOL InternalCall=FALSE);
	virtual INT GetSelectedItem();
	virtual INT GetNextSelectedItem(INT n);

	void Create(CWnd* _pParentWnd, LFSearchResult* _result, INT _FocusItem);

protected:
	CFont m_Fonts[20];
	Tag* m_Tags;
	HTHEME hTheme;

	virtual void SetViewOptions(UINT _ViewID, BOOL Force);
	virtual void SetSearchResult(LFSearchResult* _result);
	virtual BOOL IsSelected(INT n);
	virtual INT ItemAtPosition(CPoint point);
	virtual void InvalidateItem(INT n);
	virtual CMenu* GetContextMenu();

	CFont* GetFont(INT idx);
	void AdjustLayout();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSortValue();
	afx_msg void OnSortCount();
	afx_msg void OnOmitRare();
	afx_msg void OnUseSize();
	afx_msg void OnUseColors();
	afx_msg void OnUseOpacity();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSysColorChange();
	DECLARE_MESSAGE_MAP()
};
