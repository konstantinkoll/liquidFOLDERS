
// CPropertyEdit: Schnittstelle der Klasse CPropertyEdit
//

#pragma once
#include "CInspectorGrid.h"


// CPropertyEdit
//

class CPropertyEdit : public CFrontstageWnd
{
public:
	CPropertyEdit();

	virtual void PreSubclassWindow();

	BOOL IsNullData() const;
	void SetInitialData(const LFVariantData& VData, const STOREID& StoreID=DEFAULTSTOREID());
	void SetAttribute(UINT Attr);

	LFVariantData m_VData;

protected:
	void AdjustLayout();
	void DestroyEdit();
	void CreateProperty();

	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pOldWnd);
	afx_msg UINT OnGetDlgCode();

	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTextChanged(WPARAM wParam, LPARAM lParam);

	afx_msg void OnClick();
	afx_msg void OnChange();
	DECLARE_MESSAGE_MAP()

	CProperty* m_pProperty;
	CMFCMaskedEdit* m_pWndEdit;
	CHoverButton m_wndButton;
	STOREID m_StoreID;
	INT m_ButtonWidth;
};

inline BOOL CPropertyEdit::IsNullData() const
{
	return LFIsNullVariantData(m_VData);
}
