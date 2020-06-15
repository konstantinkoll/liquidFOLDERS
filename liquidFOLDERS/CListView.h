
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

class CListView sealed : public CFileView
{
public:
	CListView();

protected:
	virtual void SetViewSettings(BOOL UpdateSearchResultPending);
	virtual void SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData);
	virtual INT GetHeaderIndent() const;
	virtual void GetHeaderContextMenu(CMenu& Menu);
	virtual BOOL AllowHeaderColumnDrag(ATTRIBUTE Attr) const;
	virtual BOOL AllowHeaderColumnTrack(ATTRIBUTE Attr) const;
	virtual void UpdateHeaderColumnOrder(ATTRIBUTE Attr, INT Position);
	virtual void UpdateHeaderColumnWidth(ATTRIBUTE Attr, INT Width);
	virtual void UpdateHeaderColumn(ATTRIBUTE Attr, HDITEM& HeaderItem) const;
	virtual void HeaderColumnClicked(ATTRIBUTE Attr);
	virtual void AdjustLayout();
	virtual void DrawItemCell(CDC& dc, CRect& rectCell, INT Index, ATTRIBUTE Attr, BOOL Themed);
	virtual void DrawStage(CDC& dc, Graphics& g, const CRect& rect, const CRect& rectUpdate, BOOL Themed);
	virtual RECT GetLabelRect() const;

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
	SUBFOLDERATTRIBUTE m_PreviewAttribute;
	CSize m_PreviewSize;

private:
	void UpdateHeader();
	INT GetMaxAttributeWidth(ATTRIBUTE Attr) const;
	void AutosizeColumn(ATTRIBUTE Attr);
	static INT GetMinColumnWidth(ATTRIBUTE Attr);

	LFDynArray<FolderData, 128, 128> m_Folders;
	LFSearchResult* m_pFolderItems;
};

inline void CListView::UpdateHeader()
{
	CFileView::UpdateHeader(m_ContextViewSettings.ColumnOrder, m_ContextViewSettings.ColumnWidth, p_CookedFiles && m_ItemCount, m_PreviewAttribute);
}

inline INT CListView::GetMinColumnWidth(ATTRIBUTE Attr)
{
	ASSERT(LFGetApp()->m_Attributes[Attr].TypeProperties.DefaultColumnWidth>=60);

	return (Attr==LFAttrFileName) ? 200 : 60;
}
