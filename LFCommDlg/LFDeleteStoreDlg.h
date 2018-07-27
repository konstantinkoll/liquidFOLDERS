
// LFDeleteStoreDlg.h: Schnittstelle der Klasse LFDeleteStoreDlg
//

#pragma once
#include "LFCore.h"
#include "LFDialog.h"


// LFDeleteStoreDlg
//

class LFDeleteStoreDlg : public LFDialog
{
public:
	LFDeleteStoreDlg(const ABSOLUTESTOREID& StoreID, CWnd* pParentWnd=NULL);

protected:
	virtual BOOL InitDialog();

	afx_msg void OnUpdateOkButton();
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	ABSOLUTESTOREID m_StoreID;
	LFStoreDescriptor m_StoreDescriptor;
};
