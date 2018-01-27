
// LFItemTemplateDlg.h: Schnittstelle der Klasse LFItemTemplateDlg
//

#pragma once
#include "LFDialog.h"
#include "CHeaderArea.h"
#include "CInspectorGrid.h"


// LFItemTemplateDlg
//

class LFItemTemplateDlg : public LFDialog
{
public:
	LFItemTemplateDlg(LFItemDescriptor* pItem, const LPCSTR pStoreID, CWnd* pParentWnd=NULL, BOOL AllowChooseStore=FALSE, LFFilter* pFilter=NULL);

	CHAR m_StoreID[LFKeySize];
	LFItemDescriptor* m_pItem;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void AdjustLayout(const CRect& rectLayout, UINT nFlags);
	virtual BOOL InitDialog();

	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnChooseStore();
	afx_msg void OnSkip();

	afx_msg void OnToggleSort();
	afx_msg void OnReset();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);

	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	BOOL m_AllowChooseStore;

private:
	LFVariantData m_AttributeValues[LFAttributeCount];
	CHeaderArea m_wndHeaderArea;
	CInspectorGrid m_wndInspectorGrid;
	BOOL m_SortAlphabetic;
};
