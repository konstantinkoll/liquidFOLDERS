
// LFAddStoreDlg.h: Schnittstelle der Klasse LFAddStoreDlg
//

#pragma once
#include "CStoreButton.h"
#include "LFDialog.h"


// LFAddStoreDlg
//

class LFAddStoreDlg : public LFDialog
{
public:
	LFAddStoreDlg(CWnd* pParentWnd);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnUpdateStores(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBtnLiquidfolders();
	afx_msg void OnBtnWindows();
	DECLARE_MESSAGE_MAP()

	CStoreButton m_wndStoreButtons[10];

private:
	void CheckInternetConnection();
};
