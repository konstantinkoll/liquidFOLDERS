
// ChooseDetailsDlg.h: Schnittstelle der Klasse ChooseDetailsDlg
//

#pragma once
#include "LFCommDlg.h"
#include "liquidFOLDERS.h"


// ChooseDetailsDlg
//

class ChooseDetailsDlg : public LFAttributeListDlg
{
public:
	ChooseDetailsDlg(UINT Context, CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void TestAttribute(UINT Attr, BOOL& Add, BOOL& Check);
	virtual BOOL InitDialog();

	afx_msg void OnSelectionChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();
	afx_msg void OnCheckAll();
	afx_msg void OnUncheckAll();
	DECLARE_MESSAGE_MAP()

	UINT m_Context;
	LFContextViewSettings* p_ContextViewSettings;

	CExplorerList m_wndAttributes;

private:
	void SwapItems(INT FocusItem, INT NewPos);
};
