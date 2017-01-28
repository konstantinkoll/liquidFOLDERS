
// LFICloudDlg.h: Schnittstelle der Klasse LFICloudDlg
//

#pragma once
#include "LFDialog.h"
#include "ICloud.h"


// LFICloudDlg
//

class LFICloudDlg : public LFDialog
{
public:
	LFICloudDlg(const ICloud& ICloud, CWnd* pParentWnd=NULL);

	WCHAR m_FolderPath[MAX_PATH];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

private:
	CItemPanel m_wndPanel;
};
