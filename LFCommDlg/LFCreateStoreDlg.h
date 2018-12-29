
// LFCreateStoreDlg.h: Schnittstelle der Klasse LFCreateStoreDlg
//

#pragma once
#include "CFrontstageItemView.h"
#include "CIconCtrl.h"
#include "LFDialog.h"


// CVolumeList
//

struct VolumeItemData
{
	ItemData Hdr;
	CHAR cVolume;
	WCHAR DisplayName[256];
	INT iIcon;
};

class CVolumeList sealed : public CFrontstageItemView
{
public:
	CVolumeList();

	virtual BOOL GetContextMenu(CMenu& Menu, INT Index);

	void SetVolumes(UINT Mask=LFGLV_INTERNAL | LFGLV_EXTERNAL | LFGLV_NETWORK);
	CHAR GetSelectedVolume() const;

protected:
	virtual void ShowTooltip(const CPoint& point);
	virtual void AdjustLayout();
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);

private:
	VolumeItemData* GetVolumeItemData(INT Index) const;
	void AddVolume(CHAR cVolume, LPCWSTR DisplayName, INT iIcon);
};

inline VolumeItemData* CVolumeList::GetVolumeItemData(INT Index) const
{
	return (VolumeItemData*)GetItemData(Index);
}


// LFCreateStoreDlg
//

class LFCreateStoreDlg : public LFDialog
{
public:
	LFCreateStoreDlg(CWnd* pParentWnd=NULL);

	UINT m_Result;
	WCHAR m_StoreName[256];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	CHAR GetSelectedVolume() const;
	void UpdateVolumes();

	afx_msg void OnDestroy();
	afx_msg void OnUpdateControls();
	afx_msg void OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnVolumeChange(WPARAM wParam, LPARAM lParam);

	afx_msg void OnVolumeFormat();
	afx_msg void OnVolumeEject();
	afx_msg void OnVolumeProperties();
	afx_msg void OnUpdateVolumeCommands(CCmdUI* pCmdUI);

	DECLARE_MESSAGE_MAP()

	CIconCtrl m_wndIcon;
	CButton m_wndAutoPath;
	CButton m_wndMakeSearchable;
	CVolumeList m_wndVolumeList;

private:
	ULONG m_SHChangeNotifyRegister;
};

inline CHAR LFCreateStoreDlg::GetSelectedVolume() const
{
	return m_wndVolumeList.GetSelectedVolume();
}
