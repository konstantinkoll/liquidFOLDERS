
// CListView.h: Schnittstelle der Klasse CListView
//

#pragma once
#include "CGridView.h"
#include "LFCommDlg.h"


// CListView
//

class CListView : public CGridView
{
public:
	CListView(UINT DataSize=sizeof(GridItemData));

protected:
	virtual void SetViewOptions(BOOL Force);
	virtual void SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data);
	virtual void AdjustLayout();
	virtual RECT GetLabelRect(INT Index);
	virtual void DrawItem(CDC& dc, LPRECT rectItem, INT Index, BOOL Themed);
	virtual void ScrollWindow(INT dx, INT dy);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnToggleAttribute(UINT nID);
	afx_msg void OnUpdateToggleCommands(CCmdUI* pCmdUI);
	afx_msg void OnAutosizeAll();
	afx_msg void OnAutosize();
	afx_msg void OnChooseDetails();
	afx_msg void OnUpdateDetailsCommands(CCmdUI* pCmdUI);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginTrack(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemClick(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	CTooltipHeader m_wndHeader;

private:
	void AdjustHeader(BOOL bShow);
	void DrawIcon(CDC& dc, CRect& rect, LFItemDescriptor* i);
	void AttributeToString(LFItemDescriptor* i, UINT Attr, WCHAR* tmpStr, size_t cCount);
	void DrawTileRows(CDC& dc, CRect& rect, LFItemDescriptor* i, GridItemData* d, INT* Rows, BOOL Themed);
	void DrawColumn(CDC& dc, CRect& rect, LFItemDescriptor* i, UINT Attr);
	void DrawProperty(CDC& dc, CRect& rect, LFItemDescriptor* i, GridItemData* d, UINT Attr, BOOL Themed, BOOL AlwaysNewRow=TRUE);
	INT GetMaxLabelWidth(INT Max);
	INT GetMaxColumnWidth(UINT Col, INT Max);
	void AutosizeColumn(UINT Col);
	void SortCategories(LFSearchResult* Result);

	CImageList* m_Icons[2];
	SIZE m_IconSize[2];
	INT m_HeaderItemClicked;
	BOOL m_IgnoreHeaderItemChange;
};