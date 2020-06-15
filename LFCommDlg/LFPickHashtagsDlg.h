
// LFPickHashtagsDlg.h: Schnittstelle der Klasse LFPickHashtagsDlg
//

#pragma once
#include "CExplorerList.h"
#include "CInspectorGrid.h"


// LFPickHashtagsDlg
//

class LFPickHashtagsDlg : public CAttributePickDlg
{
public:
	LFPickHashtagsDlg(ATTRIBUTE Attr, ITEMCONTEXT Context, const CString& Hashtags, CWnd* pParentWnd=NULL);

	CString m_Hashtags;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	//afx_msg void OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult);
	//DECLARE_MESSAGE_MAP()

	CExplorerList m_wndAssignedHashtags;
	CEdit m_wndNewHashtags;
};
