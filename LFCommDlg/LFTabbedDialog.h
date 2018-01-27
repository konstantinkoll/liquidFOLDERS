
// LFTabbedDialog.h: Schnittstelle der Klasse LFTabbedDialog
//

#pragma once
#include "CBackstageSidebar.h"
#include "LFDialog.h"
#include "LFDynArray.h"


// LFTabbedDialog
//

#define MAXTABS     16

struct ControlOnTab
{
	HWND hWnd;
	UINT Index;
};

class LFTabbedDialog : public LFDialog
{
public:
	LFTabbedDialog(UINT nCaptionID, CWnd* pParentWnd=NULL, UINT* pLastTab=NULL);

protected:
	virtual void ShowTab(UINT Index);
	virtual BOOL InitSidebar(LPSIZE pszTabArea);
	virtual BOOL InitDialog();

	BOOL AddTab(const CString& Caption);
	BOOL AddTab(UINT nResID, LPSIZE pszTabArea);
	void AddControl(const ControlOnTab& Ctrl);
	void AddControl(HWND hWnd, UINT Index);
	void SelectTab(UINT Index);

	afx_msg void OnDestroy();
	afx_msg void OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnSelectTab(UINT nCmdID);
	afx_msg void OnUpdateTabCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	UINT m_CurrentTab;

private:
	CBackstageSidebar m_wndSidebar;
	CString m_Caption;
	UINT m_TabCount;
	UINT* p_LastTab;
	LFDynArray<ControlOnTab, 16, 16>m_ControlsOnTab;
	WCHAR m_TabHints[MAXTABS][4096];
};

inline void LFTabbedDialog::AddControl(const ControlOnTab& Ctrl)
{
	m_ControlsOnTab.AddItem(Ctrl);
}
