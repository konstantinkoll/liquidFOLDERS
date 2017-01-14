
// LFDropboxDlg.h: Schnittstelle der Klasse LFDropboxDlg
//

#pragma once
#include "LFDialog.h"
#include "Dropbox.h"


// LFDropboxDlg
//

class LFDropboxDlg : public LFDialog
{
public:
	LFDropboxDlg(const Dropbox& Dropbox, CWnd* pParentWnd=NULL);

	WCHAR m_FolderPath[MAX_PATH];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	afx_msg void OnAddPersonal();
	afx_msg void OnAddBusiness();
	DECLARE_MESSAGE_MAP()

private:
	DropboxData m_DropboxData;
	CString m_SubscriptionTypes[2];
	CItemPanel m_wndPersonalPanel;
	CItemPanel m_wndBusinessPanel;
};
