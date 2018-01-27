
// CPropertyEdit: Schnittstelle der Klasse CPropertyEdit
//

#pragma once
#include "CInspectorGrid.h"


// CPropertyEdit
//

class CPropertyEdit : public CPropertyHolder
{
public:
	CPropertyEdit();

	virtual void PreSubclassWindow();

	BOOL Create(CWnd* pParentWnd, UINT nID);
	BOOL IsNullData() const;
	void SetInitialData(const LFVariantData& VData);
	void SetAttribute(UINT Attr);

	LFVariantData m_VData;

protected:
	virtual void AdjustLayout();
	virtual void DrawStage(CDC& dc, Graphics& g, const CRect& rect, const CRect& rectUpdate, BOOL Themed);
	virtual void NotifyOwner(SHORT Attr1, SHORT Attr2=-1, SHORT Attr3=-1);

	void CreateProperty();
	void DestroyEdit();

	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	afx_msg void OnChange();
	afx_msg void OnClick();
	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	CProperty* m_pProperty;
	CMFCMaskedEdit* m_pWndEdit;
	CHoverButton m_wndButton;
	INT m_ButtonWidth;
};

inline BOOL CPropertyEdit::IsNullData() const
{
	return LFIsNullVariantData(m_VData);
}
