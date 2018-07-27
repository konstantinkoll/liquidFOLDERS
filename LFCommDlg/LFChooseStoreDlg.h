
// LFChooseStoreDlg.h: Schnittstelle der Klasse LFChooseStoreDlg
//

#pragma once
#include "CHeaderArea.h"
#include "CAbstractFileView.h"
#include "LFCore.h"
#include "LFDialog.h"


// CStoreList
//

class CStoreList : public CAbstractFileView
{
public:
	CStoreList();

	virtual BOOL GetContextMenu(CMenu& Menu, INT Index);

protected:
	virtual void AdjustLayout();
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);
	virtual RECT GetLabelRect(INT Index) const;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
};

// LFChooseStoreDlg
//

class LFChooseStoreDlg : public LFDialog
{
public:
	LFChooseStoreDlg(CWnd* pParentWnd=NULL, BOOL Writeable=TRUE);

	ABSOLUTESTOREID m_StoreID;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void AdjustLayout(const CRect& rectLayout, UINT nFlags);
	virtual BOOL InitDialog();

	INT GetSelectedStore() const;
	void UpdateOkButton();

	afx_msg void OnDestroy();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnRenameItem(WPARAM wParam, LPARAM lParam);

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
	BOOL m_Writeable;
};

inline INT LFChooseStoreDlg::GetSelectedStore() const
{
	return m_wndStoreList.GetSelectedItem();
}
