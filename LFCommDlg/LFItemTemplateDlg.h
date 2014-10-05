
// LFItemTemplateDlg.h: Schnittstelle der Klasse LFItemTemplateDlg
//

#pragma once
#include "LFCore.h"
#include "LFApplication.h"
#include "LFDialog.h"
#include "CHeaderArea.h"
#include "CInspectorGrid.h"


// LFItemTemplateDlg
//

class AFX_EXT_CLASS LFItemTemplateDlg : public LFDialog
{
public:
	LFItemTemplateDlg(CWnd* pParentWnd, LFItemDescriptor* pItem, CHAR* StoreID, BOOL AllowChooseStore=FALSE, LFFilter* pFilter=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void AdjustLayout();

	CHAR m_StoreID[LFKeySize];
	LFItemDescriptor* m_pItem;

protected:
	LFApplication* p_App;
	BOOL m_AllowChooseStore;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	afx_msg void OnChooseStore();
	afx_msg void OnToggleSort();
	afx_msg void OnReset();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);

	afx_msg void OnSkip();
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	LFVariantData m_AttributeValues[LFAttributeCount];
	CHeaderArea m_wndHeaderArea;
	CInspectorGrid m_wndInspectorGrid;
	BOOL m_SortAlphabetic;
};
