
// CListView.h: Schnittstelle der Klasse CListView
//

#pragma once
#include "CFileView.h"


// CListView
//

struct FolderData
{
	RECT Rect;
	LFItemDescriptor* pItemDescriptor;
	INT First;
	INT Last;
};

class CListView : public CFileView
{
public:
	CListView();

protected:
	virtual void SetViewSettings(BOOL UpdateSearchResultPending);
	virtual void SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData);
	virtual void AdjustLayout();
	virtual RECT GetLabelRect(INT Index) const;
	virtual void ScrollWindow(INT dx, INT dy, LPCRECT lpRect=NULL, LPCRECT lpClipRect=NULL);

	void DrawFolder(CDC& dc, Graphics& g, CRect& rect, INT Index, BOOL Themed);
	void DrawItem(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

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
	INT m_IconSize;
	BOOL m_HasFolders;
	CSize m_PreviewSize;

private:
	void AdjustHeader();
	INT GetMaxAttributeWidth(UINT Attr) const;
	void AutosizeColumn(UINT Attr);
	INT GetHeaderIndent() const;

	LFDynArray<FolderData, 128, 128> m_Folders;
	LFSearchResult* m_pFolderItems;
	INT m_PreviewAttribute;
	INT m_HeaderItemClicked;
	BOOL m_IgnoreHeaderItemChange;
};
