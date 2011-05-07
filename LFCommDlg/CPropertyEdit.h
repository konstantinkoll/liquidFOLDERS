
// CPropertyEdit: Schnittstelle der Klasse CPropertyEdit
//

#pragma once
#include "liquidFOLDERS.h"
#include "CInspectorGrid.h"


// CPropertyDisplay
//

class AFX_EXT_CLASS CPropertyDisplay : public CWnd
{
public:
	CPropertyDisplay();

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetProperty(CProperty* pProperty);

protected:
	CProperty* p_Property;

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg UINT OnGetDlgCode();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	DECLARE_MESSAGE_MAP()
};


// CPropertyEdit
//

class AFX_EXT_CLASS CPropertyEdit : public CPropertyHolder
{
public:
	CPropertyEdit();

	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void AdjustLayout();

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetAttribute(UINT Attr);
	void SetData(LFVariantData* pData);

protected:
	LFVariantData m_Data;
	CProperty* p_Property;
	CPropertyDisplay* p_wndDisplay;
	CMFCMaskedEdit* p_wndEdit;
	CButton m_wndButton;

	virtual void Init();
	virtual void NotifyOwner(SHORT Attr1, SHORT Attr2=-1, SHORT Attr3=-1);

	void CreateProperty();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnClick();
	DECLARE_MESSAGE_MAP()
};
