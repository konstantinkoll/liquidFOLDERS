
// LFCreateStoreDlg.h: Schnittstelle der Klasse LFCreateStoreDlg
//

#pragma once
#include "CExplorerList.h"
#include "CIconCtrl.h"
#include "LFDialog.h"


// LFCreateStoreDlg
//

class LFCreateStoreDlg : public LFDialog
{
public:
	LFCreateStoreDlg(CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	void UpdateVolumes();

	afx_msg BOOL OnInitDialog();
	afx_msg void OnUpdate();
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnVolumeFormat();
	afx_msg void OnVolumeEject();
	afx_msg void OnVolumeProperties();
	afx_msg void OnUpdateVolumeCommands(CCmdUI* pCmdUI);
	afx_msg LRESULT OnVolumesChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	CHAR m_DriveLetters[26];
	CIconCtrl m_wndIcon;
	CButton m_wndAutoPath;
	CButton m_wndMakeSearchable;
	CExplorerList m_wndExplorerList;
};
