
// CFileView.h: Schnittstelle der Klasse CFileView
//

#pragma once
#include "liquidFOLDERS.h"
#include "LFCommDlg.h"


// CFileView
//

class CFileView : public CWnd
{
public:
	friend class CFileList;

	CFileView();
	virtual ~CFileView();

	virtual void SelectItem(int n, BOOL select=TRUE, BOOL InternalCall=FALSE);
	virtual int GetFocusItem();
	virtual int GetSelectedItem();
	virtual int GetNextSelectedItem(int n);
	virtual void EditLabel(int n);							// Direkt in der Liste neuen Dateinamen setzen
	virtual BOOL IsEditing();								// Liefert zurück ob gerade editiert wird
	virtual BOOL HasCategories();
	virtual void OnContextMenu(CPoint point);				// Kontextmenü für das View
	virtual void OnItemContextMenu(int idx, CPoint point);	// Kontextmenu für ein Item

	void Create(LFSearchResult* _result, UINT _ViewID);
	void OnUpdateViewOptions(int _ActiveContextID=-1, int _ViewID=-1, BOOL Force=FALSE);
	void OnUpdateSearchResult(LFSearchResult* _result, int _FocusItem);
	BOOL HandleDefaultKeys(UINT nChar, UINT nRepCnt, UINT nFlags);
	int GetFontHeight(BOOL GrannyMode);

protected:
	LFViewParameters m_ViewParameters;
	LFViewParameters* pViewParameters;
	LFSearchResult* result;
	LFDropTarget m_DropTarget;
	UINT ActiveContextID;
	UINT RibbonColor;
	UINT ViewID;
	int FocusItem;

	virtual void SetViewOptions(UINT _ViewID, BOOL Force);
	virtual void SetSearchResult(LFSearchResult* _result);
	virtual BOOL IsSelected(int n);
	virtual int ItemAtPosition(CPoint point);
	virtual CMenu* GetContextMenu();

	void AppendContextMenu(CMenu* menu);
	void OnViewOptionsChanged(BOOL LocalSettings=FALSE);

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnToggleGrannyMode();
	afx_msg void OnSelectAll();
	afx_msg void OnSelectNone();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
