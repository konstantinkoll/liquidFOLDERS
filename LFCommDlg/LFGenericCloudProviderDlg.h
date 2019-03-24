
// LFGenericCloudProviderDlg.h: Schnittstelle der Klasse LFGenericCloudProviderDlg
//

#pragma once
#include "LFDialog.h"
#include "Box.h"


// LFGenericCloudProviderDlg
//

class LFGenericCloudProviderDlg : public LFDialog
{
public:
	LFGenericCloudProviderDlg(const CString& ProviderName, LPCWSTR lpcPath, CWnd* pParentWnd=NULL);

	WCHAR m_FolderPath[MAX_PATH];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	CString m_ProviderName;

private:
	CItemPanel m_wndPanel;
};
