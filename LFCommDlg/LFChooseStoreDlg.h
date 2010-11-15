
// LFChooseStoreDlg.h: Schnittstelle der Klasse LFChooseStoreDlg
//

#pragma once
#include "LFCore.h"
#include "liquidFOLDERS.h"
#include "CExplorerHeader.h"
#include "CExplorerList.h"
#include "LFDialog.h"


// LFChooseStoreDlg
//

#define LFCSD_Normal             0
#define LFCSD_Mounted            1
#define LFCSD_Internal           2
#define LFCSD_ChooseDefault      3

class AFX_EXT_CLASS LFChooseStoreDlg : public LFDialog
{
public:
	LFChooseStoreDlg(CWnd* pParentWnd, UINT Mode);
	~LFChooseStoreDlg();

	CHAR StoreID[LFKeySize];

	virtual void AdjustLayout();
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	CExplorerHeader m_wndExplorerHeader;
	CExplorerList m_wndExplorerList;
	LFSearchResult* p_Result;
	UINT m_Mode;

	void UpdateOkButton();

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg LRESULT OnUpdateStores(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNewStore();
	afx_msg void OnMaintainAll();
	afx_msg void OnMakeDefault();
	afx_msg void OnMakeHybrid();
	afx_msg void OnMaintain();
	afx_msg void OnRename();
	afx_msg void OnDelete();
	afx_msg void OnProperties();
	DECLARE_MESSAGE_MAP()
};
