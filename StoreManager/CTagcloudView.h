
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
	UINT alpha;
	int x;
	int y;
	int h;
	int w;
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
	CFont m_Fonts[24];
	Tag* m_Tags;
	HTHEME hTheme;

	virtual void SetSearchResult(LFSearchResult* _result);
	virtual BOOL IsSelected(int n);
	virtual int ItemAtPosition(CPoint point);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
