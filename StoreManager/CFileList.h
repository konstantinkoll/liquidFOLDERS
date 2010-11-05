
// CFileList.h: Schnittstelle der Klasse CFileList
//

#pragma once
#include "liquidFOLDERS.h"
#include "CFileView.h"
#include "LFCommDlg.h"


// CFileList
//

#define FileListExtendedStyles     LVS_EX_DOUBLEBUFFER | LVS_EX_HEADERDRAGDROP | LVS_EX_LABELTIP

class CFileList : public CExplorerList
{
public:
	friend class CAbstractListView;
	friend class CListView;
	friend class CCalendarDayView;

	CFileList();
	~CFileList();

	BOOL Create(CFileView* pViewWnd, BOOL _OwnerData);
	INT GetFontHeight();
	void SetHeader(BOOL sorting=FALSE, BOOL selectCol=TRUE);
	BOOL SetColumnWidth(INT nCol, INT cx);

protected:
	CFileView* View;
	BOOL OwnerData;
	BOOL Editing;
	INT LastSortBy;
	UINT ColumnCount;
	UINT ColumnMapping[LFAttributeCount];
	UINT TooltipCount;

	void CreateColumns();
	void AddColumn(UINT attr);
	INT FindColumn(UINT attr);

	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHeaderCanResize(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHeaderResize(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHeaderCanReorder(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHeaderReorder(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	DECLARE_MESSAGE_MAP()

private:
	UINT ItemChanged;
	WCHAR m_StrBuffer[LFAttributeCount][300];
};
