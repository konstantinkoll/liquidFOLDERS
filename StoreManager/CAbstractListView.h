
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

	virtual void SelectItem(INT n, BOOL select, BOOL InternalCall);
	virtual INT GetFocusItem();
	virtual INT GetSelectedItem();
	virtual INT GetNextSelectedItem(INT n);
	virtual void EditLabel(INT n);
	virtual BOOL IsEditing();
	virtual BOOL HasCategories();

	void OnUpdateCommands(CCmdUI* pCmdUI);

protected:
	CFileList m_FileList;

	virtual BOOL IsSelected(INT n);

	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnToggleAttribute(UINT nID);
	afx_msg void OnUpdateToggleAttribute(CCmdUI* pCmdUI);
	afx_msg void OnAutosizeColumns();
	afx_msg void OnToggleCategories();
	afx_msg void OnSysColorChange();
	DECLARE_MESSAGE_MAP()
};
