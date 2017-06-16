
// CFileView.h: Schnittstelle der Klasse CFileView
//

#pragma once
#include "LFCommDlg.h"


// View parameters

#define VIEWSETTINGSVERSION     4

struct LFContextViewSettings
{
	UINT SortBy;
	BOOL Descending;

	UINT View;
	INT ColumnOrder[LFAttributeCount];
	INT ColumnWidth[LFAttributeCount];
};

struct LFGlobalViewSettings
{
	UINT LastViewSelected[LFAttributeCount];

	BOOL CalendarShowDays;

	INT GlobeLatitude;
	INT GlobeLongitude;
	INT GlobeZoom;
	BOOL GlobeShowSpots;
	BOOL GlobeShowAirportNames;
	BOOL GlobeShowGPS;
	BOOL GlobeShowDescription;

	BOOL TagcloudCanonical;
	BOOL TagcloudShowRare;
	BOOL TagcloudUseSize;
	BOOL TagcloudUseColors;
	BOOL TagcloudUseOpacity;
};

struct GlobeParameters
{
	GLfloat Latitude;
	GLfloat Longitude;
	INT Zoom;
};

struct FVPersistentData
{
	INT FocusItem;
	INT HScrollPos;
	INT VScrollPos;
	UINT Year;
	GlobeParameters Location;
	BOOL LocationValid;
};


// SendTo Item

struct SendToItemData
{
	HICON hIcon;
	INT cx;
	INT cy;
	BOOL IsStore;
	CHAR StoreID[LFKeySize];
	WCHAR Path[MAX_PATH];
};


// Item Data

struct FVItemData
{
	RECT Rect;
	INT RectInflate;
	BOOL Selected;
	BOOL Valid;
};


// Theming

struct CachedSelectionBitmap
{
	HBITMAP hBitmap;
	INT Width;
	INT Height;
};


// CFileView
//

#define WM_UPDATESELECTION     WM_USER+100
#define WM_SELECTALL           WM_USER+101
#define WM_SELECTNONE          WM_USER+102
#define WM_RENAMEITEM          WM_USER+103

#define FF_ENABLESCROLLING          0x0001
#define FF_ENABLEHOVER              0x0002
#define FF_ENABLETOOLTIPS           0x0004
#define FF_ENABLEFOLDERTOOLTIPS     0x0008
#define FF_ENABLETOOLTIPICONS       0x0010
#define FF_ENABLESHIFTSELECTION     0x0020
#define FF_ENABLELABELEDIT          0x0040

#define BM_REFLECTION     0
#define BM_SELECTED       1

class CFileView : public CFrontstageWnd
{
public:
	CFileView(SIZE_T DataSize=sizeof(FVItemData), UINT Flags=FF_ENABLESCROLLING | FF_ENABLEHOVER | FF_ENABLETOOLTIPS | FF_ENABLEFOLDERTOOLTIPS | FF_ENABLETOOLTIPICONS | FF_ENABLESHIFTSELECTION | FF_ENABLELABELEDIT);
	virtual ~CFileView();

	virtual BOOL Create(CWnd* pParentWnd, UINT nID, const CRect& rect, LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData=NULL, UINT nClassStyle=CS_DBLCLKS);
	virtual CMenu* GetViewContextMenu();
	virtual void GetPersistentData(FVPersistentData& Data) const;
	virtual void EditLabel(INT Index);

	void UpdateViewSettings(INT Context=-1, BOOL UpdateSearchResultPending=FALSE);
	void UpdateSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData, BOOL InternalCall=FALSE);
	INT GetFocusItem() const;
	INT GetSelectedItem() const;
	INT GetNextSelectedItem(INT Index) const;
	void SelectItem(INT Index, BOOL Select=TRUE, BOOL InternalCall=FALSE);
	void EnsureVisible(INT Index);
	BOOL MultiSelectAllowed() const;
	BOOL IsEditing() const;
	void UnselectAllAfterTransaction();

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void SetViewSettings(BOOL UpdateSearchResultPending);
	virtual void SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData);
	virtual void AdjustLayout();
	virtual RECT GetLabelRect(INT Index) const;
	virtual INT ItemAtPosition(CPoint point) const;
	virtual void InvalidateItem(INT Index);
	virtual CMenu* GetItemContextMenu(INT Index);
	virtual void ScrollWindow(INT dx, INT dy, LPCRECT lpRect=NULL, LPCRECT lpClipRect=NULL);

	void ValidateAllItems();
	BOOL IsItemSelected(INT Index) const;
	void ChangedItem(INT Index);
	void ChangedItems();
	void SetFocusItem(INT FocusItem, BOOL ShiftSelect, BOOL Deselect=TRUE);
	RECT GetItemRect(INT Index) const;
	CMenu* GetSendToMenu();
	CString GetLabel(LFItemDescriptor* pItemDescriptor) const;
	BOOL BeginDragDrop();
	void DrawItemBackground(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed, BOOL Cached=TRUE);
	void DrawItemForeground(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed, BOOL Cached=TRUE);
	void DrawJumboIcon(CDC& dc, Graphics& g, CPoint pt, LFItemDescriptor* pItemDescriptor, INT ThumbnailYOffset=1) const;
	BOOL DrawNothing(CDC& dc, LPCRECT lpRectClient, BOOL Themed);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt);
	afx_msg void OnMouseHWheel(UINT nFlags, SHORT zDelta, CPoint pt);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);

	afx_msg void OnMeasureItem(INT nIDCtl, LPMEASUREITEMSTRUCT lpmis);
	afx_msg void OnDrawItem(INT nID, LPDRAWITEMSTRUCT lpdis);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	afx_msg void OnSelectAll();
	afx_msg void OnSelectNone();
	afx_msg void OnSelectInvert();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	afx_msg void OnDestroyEdit();
	DECLARE_MESSAGE_MAP()

	INT m_Context;
	LFContextViewSettings m_ContextViewSettings;
	LFGlobalViewSettings m_GlobalViewSettings;
	LFContextViewSettings* p_ContextViewSettings;
	LFGlobalViewSettings* p_GlobalViewSettings;
	LFFilter* p_Filter;
	LFSearchResult* p_RawFiles;
	LFSearchResult* p_CookedFiles;
	SIZE_T m_DataSize;
	LPBYTE m_pItemData;
	UINT m_ItemDataAllocated;
	BOOL m_Nothing;
	UINT m_HeaderHeight;
	UINT m_Flags;
	BOOL m_HideFileExt;
	BOOL m_ShowFocusRect;
	BOOL m_AllowMultiSelect;
	INT m_FocusItem;
	INT m_HotItem;
	INT m_SelectionAnchor;
	INT m_EditLabel;
	INT m_ScrollWidth;
	INT m_ScrollHeight;
	INT m_HScrollPos;
	INT m_VScrollPos;
	INT m_HScrollMax;
	INT m_VScrollMax;
	INT m_RowHeight;
	INT m_LargeFontHeight;
	INT m_DefaultFontHeight;
	INT m_SmallFontHeight;
	BOOL m_Hover;
	BOOL m_BeginDragDrop;
	CPoint m_DragPos;
	WCHAR m_TypingBuffer[256];
	DWORD m_TypingTicks;

private:
	void AppendSendToItem(CMenu* pMenu, UINT nIDCtl, LPCWSTR lpszNewItem, HICON hIcon, INT cx, INT cy);
	void ResetScrollbars();
	void AdjustScrollbars();
	void DestroyEdit(BOOL Accept=FALSE);

	CachedSelectionBitmap m_Bitmaps[2];
	CEdit* m_pWndEdit;
	SendToItemData m_SendToItems[256];
};


inline BOOL CFileView::MultiSelectAllowed() const
{
	return m_AllowMultiSelect;
}

inline BOOL CFileView::IsEditing() const
{
	return (m_pWndEdit!=NULL);
}

inline void CFileView::ChangedItem(INT Index)
{
	InvalidateItem(Index);

	GetParent()->SendMessage(WM_UPDATESELECTION);
}

inline void CFileView::ChangedItems()
{
	Invalidate();

	GetParent()->SendMessage(WM_UPDATESELECTION);
}
