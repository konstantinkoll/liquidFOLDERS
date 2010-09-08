
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
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
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

	void SetItem(LFStoreDescriptor* _store, BOOL Repaint=TRUE);

	LFStoreDescriptor* store;

protected:
	afx_msg LRESULT OnSetItem(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
