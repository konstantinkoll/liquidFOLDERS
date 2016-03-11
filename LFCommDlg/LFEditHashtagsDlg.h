
// LFEditHashtagsDlg.h: Schnittstelle der Klasse LFEditHashtagsDlg
//

#pragma once
#include "CExplorerList.h"
#include "LFDialog.h"


// LFEditHashtagsDlg
//

class LFEditHashtagsDlg : public LFDialog
{
public:
	LFEditHashtagsDlg(CString Hashtags, CHAR* pStoreID, CWnd* pParentWnd=NULL);

	CString m_Hashtags;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	afx_msg void OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateAssignedHashtags();
	DECLARE_MESSAGE_MAP()

	CExplorerList m_wndAssignedHashtags;
	CButton m_wndHashtagsFromAllStores;
	CEdit m_wndNewHashtags;

private:
	CHAR m_StoreID[LFKeySize];
};
