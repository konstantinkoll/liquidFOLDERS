
// LFICloudDlg.h: Schnittstelle der Klasse LFICloudDlg
//

#pragma once
#include "LFDialog.h"
#include "iCloud.h"


// LFICloudDlg
//

class LFICloudDlg : public LFDialog
{
public:
	LFICloudDlg(const ICloud& iCloud, CWnd* pParentWnd=NULL);

	WCHAR m_FolderPath[MAX_PATH];
	WCHAR m_StoreName[256];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	afx_msg void OnAddDrive();
	afx_msg void OnAddPhotos();
	DECLARE_MESSAGE_MAP()

private:
	LFICloudPaths m_iCloudPaths;
	CItemPanel m_wndDrivePanel;
	CItemPanel m_wndPhotosPanel;
};
