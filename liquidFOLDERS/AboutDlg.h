
// AboutDlg.h: Schnittstelle der Klasse AboutDlg
//

#pragma once
#include "LFCommDlg.h"


// AboutDlg
//

class AboutDlg : public LFAboutDialog
{
public:
	AboutDlg(CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitSidebar(LPSIZE pszTabArea);
	virtual BOOL InitDialog();

	CComboBox m_wndStartWith;
	CComboBox m_wndModelQuality;
	CComboBox m_wndTextureQuality;

private:
	static void AddContextName(CComboBox& wndCombobox, UINT ContextID);
	static void AddQualityString(CComboBox& wndCombobox, UINT nResID);
};
