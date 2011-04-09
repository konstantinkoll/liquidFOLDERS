
// CStoreSelector.h: Schnittstelle der Klasse CStoreSelector
//

#pragma once
#include "LFCommDlg.h"
#include "liquidFOLDERS.h"


// CStoreDropdownWindow
//

class AFX_EXT_CLASS CStoreDropdownWindow : public CDropdownWindow
{
public:
	CStoreDropdownWindow();

	void PopulateList();

protected:
	LFSearchResult* p_Result;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnUpdateStores(WPARAM wParam, LPARAM lParam);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCreateNewStore();
	DECLARE_MESSAGE_MAP()
};


// CStoreSelector
//

class AFX_EXT_CLASS CStoreSelector : public CDropdownSelector
{
public:
	CStoreSelector();
	~CStoreSelector();

	virtual void CreateDropdownWindow();
	virtual void SetEmpty(BOOL Repaint=TRUE);
	virtual void GetTooltipData(HICON& hIcon, CSize& Size, CString& Caption, CString& Hint);

	void SetItem(LFItemDescriptor* pItem, BOOL Repaint=TRUE, UINT NotifyCode=NM_SELCHANGED);
	void SetItem(LFStoreDescriptor* s, BOOL Repaint=TRUE, UINT NotifyCode=NM_SELCHANGED);
	void SetItem(CHAR* Key, BOOL Repaint=TRUE);
	BOOL GetStoreID(CHAR* StoreID);
	void Update();

protected:
	LFItemDescriptor* p_Item;

	afx_msg LRESULT OnSetItem(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
