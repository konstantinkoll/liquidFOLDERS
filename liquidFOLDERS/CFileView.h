
// CFileView.h: Schnittstelle der Klasse CFileView
//

#pragma once
#include "CInspectorPane.h"
#include "LFCommDlg.h"


// View parameters

#define VIEWSETTINGSVERSION     8

struct LFContextViewSettings
{
	ATTRIBUTE SortBy;
	BOOL SortDescending;

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
	BOOL GlobeShowLocations;
	BOOL GlobeShowAirportNames;
	BOOL GlobeShowCoordinates;
	BOOL GlobeShowDescriptions;

	BOOL IconsShowCapacity;

	INT TagcloudSort;
	BOOL TagcloudShowRare;
	BOOL TagcloudUseSize;
	BOOL TagcloudUseColor;
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
	BOOL FocusItemSelected;
	INT HScrollPos;
	INT VScrollPos;
	
	UINT Year;
	
	GlobeParameters Location;
	BOOL LocationValid;
};


// SendTo Item

#define CHOOSESTOREID     "CHOOSE"

struct SendToItemData
{
	HICON hIcon;
	INT cx;
	INT cy;
	BOOL IsStore;
	STOREID StoreID;
	WCHAR Path[MAX_PATH];
};


// CFileView
//

#define FF_ENABLEFOLDERTOOLTIPS     0x10000000
#define FF_ENABLETOOLTIPICONS       0x20000000

#define FIRSTSENDTO     0xFF00
#define LASTSENDTO      0xFFFF

class CFileView : public CAbstractFileView
{
public:
	CFileView(UINT Flags=FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLESELECTION | FRONTSTAGE_ENABLESHIFTSELECTION | FRONTSTAGE_ENABLELABELEDIT | FF_ENABLEFOLDERTOOLTIPS | FF_ENABLETOOLTIPICONS, SIZE_T szData=sizeof(ItemData), const CSize& szItemInflate=CSize(0, 0));

	virtual BOOL Create(CWnd* pParentWnd, UINT nID, const CRect& rect, CIcons* pTaskIcons, CInspectorPane* pInspectorPane, LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData=NULL, UINT nClassStyle=0);
	virtual void GetPersistentData(FVPersistentData& Data, BOOL ForReload=FALSE) const;
	virtual BOOL GetContextMenu(CMenu& Menu, INT Index);

	void UpdateViewSettings(ITEMCONTEXT Context, BOOL UpdateSearchResultPending=FALSE);
	void UpdateSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData, BOOL InternalCall=FALSE);
	UINT GetSortAttribute() const;
	const SendToItemData* GetSendToItemData(UINT nID) const;

protected:
	virtual void SetViewSettings(BOOL UpdateSearchResultPending);
	virtual void SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData);
	virtual void AdjustScrollbars();
	virtual void ShowTooltip(const CPoint& point);
	virtual BOOL IsItemSelected(INT Index) const;
	virtual void SelectItem(INT Index, BOOL Select=TRUE);
	virtual void FireSelectedItem();
	virtual void DeleteSelectedItem();
	virtual void DrawNothing(CDC& dc, CRect rect, BOOL Themed) const;

	CString GetItemLabel(const LFItemDescriptor* pItemDescriptor, BOOL AllowExtension=TRUE) const;
	COLORREF SetLightTextColor(CDC& dc, const LFItemDescriptor* pItemDescriptor, BOOL Themed) const;
	COLORREF SetDarkTextColor(CDC& dc, const LFItemDescriptor* pItemDescriptor, BOOL Themed) const;
	static UINT GetColorDotCount(const LFItemDescriptor* pItemDescriptor);
	INT GetColorDotWidth(const LFItemDescriptor* pItemDescriptor, const CIcons& Icons=m_DefaultColorDots) const;
	INT GetColorDotWidth(INT Index, const CIcons& Icons=m_DefaultColorDots) const;
	void DrawColorDots(CDC& dc, CRect& rect, const LFItemDescriptor* pItemDescriptor, INT FontHeight=0, CIcons& Icons=m_DefaultColorDots) const;
	void DrawJumboIcon(CDC& dc, Graphics& g, CPoint pt, LFItemDescriptor* pItemDescriptor, INT ThumbnailYOffset=1) const;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	afx_msg void OnDrawButtonForeground(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonClicked();

	afx_msg void OnMeasureItem(INT nIDCtl, LPMEASUREITEMSTRUCT lpmis);
	afx_msg void OnDrawItem(INT nID, LPDRAWITEMSTRUCT lpdis);
	DECLARE_MESSAGE_MAP()

	ITEMCONTEXT m_Context;
	LFContextViewSettings m_ContextViewSettings;
	LFGlobalViewSettings m_GlobalViewSettings;
	LFContextViewSettings* p_ContextViewSettings;
	LFGlobalViewSettings* p_GlobalViewSettings;
	LFFilter* p_Filter;
	SUBFOLDERATTRIBUTE m_SubfolderAttribute;
	LFSearchResult* p_RawFiles;
	BOOL m_HideFileExt;
	static CIcons m_LargeColorDots;
	static CIcons m_DefaultColorDots;

private:
	void AppendMoveToItem(CMenu& Menu, UINT FromContext, UINT ToContext) const;
	void GetMoveToMenu(CMenu& Menu) const;
	void AppendSendToItem(CMenu& Menu, UINT nIDCtl, LPCWSTR lpszNewItem, HICON hIcon, INT cx, INT cy);
	void GetSendToMenu(CMenu& Menu);
	void SelectItem(LFItemDescriptor* pItemDescriptor, BOOL Select=TRUE);

	static CString m_WelcomeCaption;
	static CString m_WelcomeMessage;
	INT m_WelcomeCaptionHeight;
	INT m_WelcomeMessageHeight;
	CHoverButton m_wndCommandButton;
	CIcons* p_TaskIcons;
	CInspectorPane* p_InspectorPane;

	SendToItemData m_SendToItems[256];
};

inline UINT CFileView::GetSortAttribute() const
{
	return m_ContextViewSettings.SortBy;
}

inline const SendToItemData* CFileView::GetSendToItemData(UINT nID) const
{
	return (nID>=FIRSTSENDTO) && (nID<=LASTSENDTO) ? &m_SendToItems[nID-FIRSTSENDTO] : NULL;
}

inline void CFileView::SelectItem(LFItemDescriptor* pItemDescriptor, BOOL Select)
{
	ASSERT(pItemDescriptor);

	if (Select)
	{
		pItemDescriptor->Flags |= LFFlagsItemSelected;
	}
	else
	{
		pItemDescriptor->Flags &= ~LFFlagsItemSelected;
	}
}

inline COLORREF CFileView::SetLightTextColor(CDC& dc, const LFItemDescriptor* pItemDescriptor, BOOL Themed) const
{
	if (!LFIsItemSelected(pItemDescriptor) && !(pItemDescriptor->CoreAttributes.State & LFItemStateMissing))
		return dc.SetTextColor(Themed ? 0xA39791 : GetSysColor(COLOR_3DSHADOW));

	return dc.GetTextColor();
}

inline COLORREF CFileView::SetDarkTextColor(CDC& dc, const LFItemDescriptor* pItemDescriptor, BOOL Themed) const
{
	if (!LFIsItemSelected(pItemDescriptor) && !(pItemDescriptor->CoreAttributes.State & LFItemStateMissing))
		return dc.SetTextColor(Themed ? 0x4C4C4C : GetSysColor(COLOR_WINDOWTEXT));

	return dc.GetTextColor();
}

inline INT CFileView::GetColorDotWidth(INT Index, const CIcons& Icons) const
{
	assert(p_CookedFiles);
	assert(Index>=0);
	assert(Index<m_ItemCount);

	return GetColorDotWidth((*p_CookedFiles)[Index], Icons);
}
