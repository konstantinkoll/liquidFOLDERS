
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
	LFSearchResult* result;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
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
	virtual void GetTooltipData(HICON& hIcon, CSize& size, CString& caption, CString& hint);

	void SetItem(LFItemDescriptor* _item, BOOL Repaint=TRUE, UINT NotifyCode=NM_SELCHANGED);
	void SetItem(LFStoreDescriptor* s, BOOL Repaint=TRUE, UINT NotifyCode=NM_SELCHANGED);
	BOOL GetStoreID(char* StoreID);
	void Update();

protected:
	LFItemDescriptor* item;

	afx_msg LRESULT OnSetItem(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
