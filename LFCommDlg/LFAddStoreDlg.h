
// LFAddStoreDlg.h: Schnittstelle der Klasse LFAddStoreDlg
//

#pragma once
#include "LFDialog.h"


// LFAddStoreDlg
//

class LFAddStoreDlg : public LFDialog
{
public:
	LFAddStoreDlg(CWnd* pParentWnd=NULL);

protected:
	virtual void DrawButtonForeground(CDC& dc, LPDRAWITEMSTRUCT lpDrawItemStruct, BOOL Hover, BOOL Themed);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnUpdateStores(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBtnLiquidfolders();
	afx_msg void OnBtnWindows();
	DECLARE_MESSAGE_MAP()

private:
	void CheckInternetConnection();

	CImageList* p_Icons;
	INT m_IconSize;
};
