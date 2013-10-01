
// LFItemTemplateDlg.h: Schnittstelle der Klasse LFItemTemplateDlg
//

#pragma once
#include "LFCore.h"
#include "LFApplication.h"
#include "CStorePanel.h"
#include "CInspectorGrid.h"
#include "CFrameCtrl.h"


// LFItemTemplateDlg
//

class AFX_EXT_CLASS LFItemTemplateDlg : public CDialog
{
public:
	LFItemTemplateDlg(CWnd* pParentWnd, LFItemDescriptor* pItem, CHAR* StoreID, BOOL AllowChooseStore=FALSE, LFFilter* pFilter=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	CHAR m_StoreID[LFKeySize];
	LFItemDescriptor* m_pItem;

protected:
	LFApplication* p_App;
	BOOL m_AllowChooseStore;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnChooseStore();
	afx_msg void OnSortAlphabetic();
	afx_msg void OnReset();
	afx_msg void OnSkip();
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	LFVariantData m_AttributeValues[LFAttributeCount];
	CStorePanel m_wndStorePanel;
	CInspectorGrid m_wndInspectorGrid;
	CFrameCtrl m_FrameCtrl;
	BOOL m_SortAlphabetic;
};
