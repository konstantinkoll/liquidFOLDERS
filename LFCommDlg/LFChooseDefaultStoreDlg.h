
// LFChooseDefaultStoreDlg.h: Schnittstelle der Klasse LFChooseDefaultStoreDlg
//

#pragma once
#include "LFCore.h"
#include "liquidFOLDERS.h"
#include "CExplorerHeader.h"
#include "CExplorerList.h"
#include "LFDialog.h"


// LFChooseDefaultStoreDlg
//

class AFX_EXT_CLASS LFChooseDefaultStoreDlg : public LFDialog
{
public:
	LFChooseDefaultStoreDlg(CWnd* pParentWnd);
	~LFChooseDefaultStoreDlg();

	virtual void AdjustLayout();
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	CExplorerHeader m_wndExplorerHeader;
	CExplorerList m_wndExplorerList;
	LFSearchResult* result;

	void MakeDefault(HWND hWnd);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg LRESULT OnUpdateStores(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNewStore();
	afx_msg void OnMakeDefault();
	afx_msg void OnRename();
	afx_msg void OnDelete();
	afx_msg void OnProperties();
	DECLARE_MESSAGE_MAP()
};
