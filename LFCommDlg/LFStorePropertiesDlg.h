
// LFStorePropertiesDlg.h: Schnittstelle der Klasse LFStorePropertiesDlg
//

#pragma once
#include "CFrontstageItemView.h"
#include "CHeaderArea.h"
#include "CIconCtrl.h"
#include "CIcons.h"
#include "LFCore.h"
#include "LFTabbedDialog.h"


// CUsageList
//

struct UsageItemData
{
	ItemData Hdr;
	ITEMCONTEXT Context;
	UINT FileCount;
	INT64 FileSize;
};

class CUsageList sealed : public CFrontstageItemView
{
public:
	CUsageList();

	void SetUsage(LFStatistics Statistics);
	void SetUsage(const LFStoreDescriptor& Store);

protected:
	virtual void AdjustLayout();
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);

	static CIcons m_ContextIcons;

private:
	UsageItemData* GetUsageItemData(INT Index) const;
	void AddContext(const LFStatistics& Statistics, ITEMCONTEXT Context);
	static INT __stdcall CompareItems(UsageItemData* pData1, UsageItemData* pData2, const SortParameters& Parameters);

	static CString m_OtherFiles;
};

inline UsageItemData* CUsageList::GetUsageItemData(INT Index) const
{
	return (UsageItemData*)GetItemData(Index);
}

inline void CUsageList::SetUsage(const LFStoreDescriptor& Store)
{
	SetUsage(Store.Statistics);
}


// LFStorePropertiesDlg
//

class LFStorePropertiesDlg : public LFTabbedDialog
{
public:
	LFStorePropertiesDlg(const ABSOLUTESTOREID& StoreID, CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void AdjustLayout(const CRect& rectLayout, UINT nFlags);
	virtual BOOL InitSidebar(LPSIZE pszTabArea);
	virtual BOOL InitDialog();

	afx_msg void OnShowUsageTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRunMaintenance();
	afx_msg void OnRunSynchronize();
	afx_msg void OnRunBackup();

	afx_msg LRESULT OnUpdateStore(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	LFStoreDescriptor m_StoreDescriptor;
	BOOL m_StoreDescriptorValid;
	UINT m_StoreIcon;
	UINT m_StoreType;
	static UINT m_LastTab;

	CIconCtrl m_wndStoreIcon;
	CEdit m_wndStoreName;
	CEdit m_wndStoreComment;
	CButton m_wndMakeDefault;
	CButton m_wndMakeSearchable;

	CHeaderArea m_wndUsageHeader;
	CUsageList m_wndUsageList;

	CIconCtrl m_wndSynchronizeIcon;
	CIconCtrl m_wndMaintenanceIcon;
	CIconCtrl m_wndBackupIcon;

private:
	static CString MakeHex(LPBYTE x, UINT bCount);
	static void CEscape(CString &strEscape);

	GUID m_StoreUniqueID;
	CString m_MaskContents;
	CString m_MaskMaintenance;
	CString m_MaskSynchronized;
};
