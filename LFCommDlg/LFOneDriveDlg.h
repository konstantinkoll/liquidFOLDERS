
// LFOneDriveDlg.h: Schnittstelle der Klasse LFOneDriveDlg
//

#pragma once
#include "LFDialog.h"
#include "OneDrive.h"


// LFOneDriveDlg
//

class LFOneDriveDlg : public LFDialog
{
public:
	LFOneDriveDlg(const OneDrive& OneDrive, CWnd* pParentWnd=NULL);

	WCHAR m_FolderPath[MAX_PATH];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	afx_msg void OnAddOneDrive();
	afx_msg void OnAddCameraRoll();
	afx_msg void OnAddDocuments();
	afx_msg void OnAddPictures();
	DECLARE_MESSAGE_MAP()

private:
	LFOneDrivePaths m_OneDrivePaths;
	CItemPanel m_wndOneDrivePanel;
	CItemPanel m_wndCameraRollPanel;
	CItemPanel m_wndDocumentsPanel;
	CItemPanel m_wndPicturesPanel;
};
