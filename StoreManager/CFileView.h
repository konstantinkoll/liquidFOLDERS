
// CFileView.h: Schnittstelle der Klasse CFileView
//

#pragma once
#include "liquidFOLDERS.h"
#include "LFCommDlg.h"


// View IDs

#define LFViewLargeIcons                0
#define LFViewSmallIcons                1
#define LFViewList                      2
#define LFViewDetails                   3
#define LFViewTiles                     4
#define LFViewSearchResult              5
#define LFViewPreview                   6
#define LFViewCalendarYear              7
#define LFViewCalendarDay               8
#define LFViewGlobe                     9
#define LFViewTagcloud                  10
#define LFViewTimeline                  11

#define LFViewCount                     12


// View parameters

struct LFViewParameters
{
	UINT Mode;
	BOOL FullRowSelect;
	INT ColumnOrder[LFAttributeCount];
	INT ColumnWidth[LFAttributeCount];

	UINT SortBy;
	BOOL Descending;
	BOOL AutoDirs;

	INT GlobeLatitude;
	INT GlobeLongitude;
	INT GlobeZoom;
	BOOL GlobeShowBubbles;
	BOOL GlobeShowAirportNames;
	BOOL GlobeShowGPS;
	BOOL GlobeShowHints;
	BOOL GlobeShowSpots;
	BOOL GlobeShowViewpoint;

	BOOL TagcloudCanonical;
	BOOL TagcloudOmitRare;
	BOOL TagcloudUseSize;
	BOOL TagcloudUseColors;
	BOOL TagcloudUseOpacity;
};


BOOL AttributeSortableInView(UINT Attr, UINT ViewMode);


// CFileView
//

class CFileView : public CWnd
{
public:
	friend class CFileList;

	CFileView();
	virtual ~CFileView();

	virtual void SelectItem(INT n, BOOL select=TRUE, BOOL InternalCall=FALSE);
	virtual INT GetFocusItem();
	virtual INT GetSelectedItem();
	virtual INT GetNextSelectedItem(INT n);
	virtual void EditLabel(INT n);							// Direkt in der Liste neuen Dateinamen setzen
	virtual BOOL IsEditing();								// Liefert zurück ob gerade editiert wird
	virtual BOOL HasCategories();
	virtual void OnContextMenu(CPoint point);				// Kontextmenü für das View
	virtual void OnItemContextMenu(INT idx, CPoint point);	// Kontextmenu für ein Item

	void Create(LFSearchResult* _result, UINT _ViewID, INT _FocusItem=0, BOOL _EnableHover=TRUE, BOOL _EnableShiftSelection=TRUE);
	void OnUpdateViewOptions(INT _ActiveContextID=-1, INT _ViewID=-1, BOOL Force=FALSE);
	void OnUpdateSearchResult(LFSearchResult* _result, INT _FocusItem);
	BOOL HandleDefaultKeys(UINT nChar, UINT nRepCnt, UINT nFlags);
	INT GetFontHeight();

protected:
	LFViewParameters m_ViewParameters;
	LFViewParameters* pViewParameters;
	LFSearchResult* result;
	LFDropTarget m_DropTarget;
	UINT ActiveContextID;
	UINT RibbonColor;
	UINT ViewID;
	BOOL HideFileExt;
	BOOL EnableHover;
	BOOL EnableShiftSelection;
	INT FocusItem;
	INT SelectionAnchor;
	INT HoverItem;

	virtual void SetViewOptions(UINT _ViewID, BOOL Force);
	virtual void SetSearchResult(LFSearchResult* _result);
	virtual BOOL IsSelected(INT n);
	virtual INT ItemAtPosition(CPoint point);
	virtual void InvalidateItem(INT n);
	virtual CMenu* GetContextMenu();

	void SetFocusItem(INT _FocusItem, BOOL ShiftSelect);
	void AppendContextMenu(CMenu* menu);

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSelectAll();
	afx_msg void OnSelectNone();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	afx_msg LRESULT OnItemsDropped(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	BOOL MouseInView;
};
