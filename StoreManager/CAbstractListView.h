
// CAbstractListView.h: Schnittstelle der Klasse CAbstractListView
//

#pragma once
#include "liquidFOLDERS.h"
#include "StoreManager.h"
#include "CFileView.h"
#include "CFileList.h"


// CAbstractListView

class CAbstractListView : public CFileView
{
public:
	CAbstractListView();
	virtual ~CAbstractListView();

	virtual void SelectItem(int n, BOOL select, BOOL InternalCall);
	virtual int GetFocusItem();
	virtual int GetSelectedItem();
	virtual int GetNextSelectedItem(int n);
	virtual void EditLabel(int n);
	virtual BOOL IsEditing();
	virtual BOOL HasCategories();

	void OnUpdateCommands(CCmdUI* pCmdUI);

protected:
	CFileList m_FileList;

	virtual BOOL IsSelected(int n);

	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnToggleAttribute(UINT nID);
	afx_msg void OnUpdateToggleAttribute(CCmdUI* pCmdUI);
	afx_msg void OnAutosizeColumns();
	afx_msg void OnToggleCategories();
	DECLARE_MESSAGE_MAP()
};
