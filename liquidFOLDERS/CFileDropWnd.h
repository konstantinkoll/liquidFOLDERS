
// CFileDropWnd.h: Schnittstelle der Klasse CFileDropWnd
//

#pragma once
#include "LFCommDlg.h"


// CFileDropWnd

#define WM_OPENFILEDROP     WM_USER+200

class CFileDropWnd : public CBackstageWnd
{
public:
	CFileDropWnd();

	BOOL Create(const ABSOLUTESTOREID& StoreID);

protected:
	virtual BOOL HasDocumentSheet() const;
	virtual void PaintBackground(CPaintDC& pDC, CRect rect);
	virtual INT ItemAtPosition(CPoint point) const;
	virtual void ShowTooltip(const CPoint& point);
	virtual BOOL GetContextMenu(CMenu& Menu, INT Index);

	void SetTopMost(BOOL AlwaysOnTop);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg LRESULT OnOpenFileDrop(WPARAM wParam, LPARAM lParam);

	afx_msg void OnStoreOpenNewWindow();
	afx_msg void OnStoreSynchronize();
	afx_msg void OnStoreMakeDefault();
	afx_msg void OnStoreShortcut();
	afx_msg void OnStoreDelete();
	afx_msg void OnStoreRename();
	afx_msg void OnStoreProperties();
	afx_msg void OnUpdateStoreCommands(CCmdUI* pCmdUI);

	afx_msg LRESULT OnUpdateStore(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	ABSOLUTESTOREID m_StoreID;
	LFDropTarget m_DropTarget;
	CRect m_rectIcon;
	LFStoreDescriptor m_StoreDescriptor;
	UINT m_StoreIcon;
	UINT m_StoreType;
	BOOL m_AlwaysOnTop;
};
