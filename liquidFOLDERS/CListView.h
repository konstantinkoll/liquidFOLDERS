
// CListView.h: Schnittstelle der Klasse CListView
//

#pragma once
#include "CFileView.h"


// Item Data

struct ListItemData
{
	ItemData Hdr;
	BOOL DrawTrailingSeparator;
};


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
	virtual INT GetHeaderIndent() const;
	virtual void GetHeaderContextMenu(CMenu& Menu, UINT Attr);
	virtual BOOL AllowHeaderColumnDrag(UINT Attr) const;
	virtual BOOL AllowHeaderColumnTrack(UINT Attr) const;
	virtual void UpdateHeaderColumnOrder(UINT Attr, INT Position);
	virtual void UpdateHeaderColumnWidth(UINT Attr, INT Width);
	virtual void UpdateHeaderColumn(UINT Attr, HDITEM& HeaderItem) const;
	virtual void HeaderColumnClicked(UINT Attr);
	virtual void AdjustLayout();
	virtual RECT GetLabelRect(INT Index) const;
	virtual void DrawStage(CDC& dc, Graphics& g, const CRect& rect, const CRect& rectUpdate, BOOL Themed);
	virtual void ScrollWindow(INT dx, INT dy, LPCRECT lpRect=NULL, LPCRECT lpClipRect=NULL);

	void DrawFolder(CDC& dc, Graphics& g, CRect& rect, INT Index, BOOL Themed);
	void DrawItem(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();

	afx_msg void OnToggleAttribute(UINT nID);
	afx_msg void OnUpdateToggleCommands(CCmdUI* pCmdUI);

	afx_msg void OnAutosizeAll();
	afx_msg void OnAutosize();
	afx_msg void OnChooseDetails();
	afx_msg void OnUpdateDetailsCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	BOOL m_HasFolders;
	INT m_PreviewAttribute;
	CSize m_PreviewSize;

private:
	void UpdateHeader();
	INT GetMaxAttributeWidth(UINT Attr) const;
	void AutosizeColumn(UINT Attr);
	static INT GetMinColumnWidth(UINT Attr);

	LFDynArray<FolderData, 128, 128> m_Folders;
	LFSearchResult* m_pFolderItems;
};

inline void CListView::UpdateHeader()
{
	CFileView::UpdateHeader(p_ContextViewSettings->ColumnOrder, m_ContextViewSettings.ColumnWidth, p_CookedFiles && m_ItemCount, m_PreviewAttribute);
}

inline INT CListView::GetMinColumnWidth(UINT Attr)
{
	ASSERT(LFGetApp()->m_Attributes[Attr].TypeProperties.DefaultColumnWidth>=60);

	return (Attr==LFAttrFileName) ? 200 : 60;
}
