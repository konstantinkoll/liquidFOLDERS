
// LFChooseStoreDlg.h: Schnittstelle der Klasse LFChooseStoreDlg
//

#pragma once
#include "CHeaderArea.h"
#include "CExplorerList.h"
#include "LFCore.h"
#include "LFDialog.h"


// CStoreList
//

class CStoreList : public CExplorerList
{
public:
	void AddStoreColumns();
	void AddItemCategories();
	void SetSearchResult(LFSearchResult* pSearchResult);

protected:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

private:
	void AddColumn(INT ID, UINT Attr);
};


// LFChooseStoreDlg
//

class LFChooseStoreDlg : public LFDialog
{
public:
	LFChooseStoreDlg(CWnd* pParentWnd=NULL, BOOL Mounted=TRUE);

	CHAR m_StoreID[LFKeySize];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void AdjustLayout(const CRect& rectLayout, UINT nFlags);
	virtual BOOL InitDialog();

	void UpdateOkButton();

	afx_msg void OnDestroy();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg LRESULT OnUpdateStores(WPARAM wParam, LPARAM lParam);

	afx_msg void OnStoreMakeDefault();
	afx_msg void OnStoreShortcut();
	afx_msg void OnStoreDelete();
	afx_msg void OnStoreRename();
	afx_msg void OnStoreProperties();
	afx_msg void OnUpdateStoreCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CHeaderArea m_wndHeaderArea;
	CStoreList m_wndStoreList;
	LFSearchResult* m_pSearchResult;
	BOOL m_Mounted;
};
