
// CInspectorGrid: Schnittstelle der Klasse CInspectorGrid
//

#pragma once
#include "LFCore.h"
#include "CIcons.h"


// CProperty
//

#define WM_PROPERTYCHANGED     WM_USER+8

class CProperty
{
public:
	CProperty(LFVariantData* pVData, CWnd* pOwnerWnd);

	virtual INT GetMinWidth() const;
	virtual void DrawValue(CDC& dc, LPCRECT lpcRect) const;
	virtual HCURSOR SetCursor(INT x) const;
	virtual CMFCMaskedEdit* CreateEditControl(const CRect& rect) const;
	virtual BOOL CanDelete() const;
	virtual BOOL HasButton() const;
	virtual BOOL WantsChars() const;
	virtual void OnSetString(CString& Value) const;
	virtual BOOL OnClickValue(INT x) const;
	virtual void OnClickButton(const STOREID& StoreID=DEFAULTSTOREID());
	virtual BOOL OnPushChar(UINT nChar) const;

	static CProperty* CreateProperty(LFVariantData* pVData, CWnd* pOwnerWnd);
	void ToString(LPWSTR pStr, SIZE_T nCount) const;
	void UpdateState(BOOL Multiple, const LFVariantData* pVDataRange=NULL);
	LFVariantData* GetVData() const;
	BOOL HasMultipleValues() const;
	void SetModified();

	static CString m_MultipleValues;

protected:
	void NotifyOwner(SHORT Attr2=-1, SHORT Attr3=-1) const;

	CWnd* p_OwnerWnd;
	LFVariantData* p_VData;
	const LFVariantData* p_VDataRange;
	BOOL m_Modified;
	BOOL m_Multiple;
	BOOL m_ShowRange;
};

inline LFVariantData* CProperty::GetVData() const
{
	return p_VData;
}

inline BOOL CProperty::HasMultipleValues() const
{
	return m_Multiple;
}

inline void CProperty::SetModified()
{
	m_Modified = TRUE;
	m_Multiple = FALSE;
}

inline void CProperty::NotifyOwner(SHORT Attr2, SHORT Attr3) const
{
	ASSERT(p_OwnerWnd);

	p_OwnerWnd->PostMessage(WM_PROPERTYCHANGED, (SHORT)p_VData->Attr, Attr2 | (Attr3 << 16));
}


// CPropertyHashtags
//

class CPropertyHashtags : public CProperty
{
public:
	CPropertyHashtags(LFVariantData* pVData, CWnd* pOwnerWnd);

	virtual BOOL HasButton() const;
	virtual void OnClickButton(const STOREID& StoreID=DEFAULTSTOREID());
};


// CPropertyIATA
//

class CPropertyIATA : public CProperty
{
friend class CInspectorGrid;

public:
	CPropertyIATA(LFVariantData* pVData, CWnd* pOwnerWnd);

	virtual CMFCMaskedEdit* CreateEditControl(const CRect& rect) const;
	virtual BOOL HasButton() const;
	virtual void OnSetString(CString& Value) const;
	virtual void OnClickButton(const STOREID& StoreID=DEFAULTSTOREID());

protected:
	LFVariantData* p_VDataLocationName;
	LFVariantData* p_VDataLocationGPS;

private:
	void SetAdditionalVData(LFVariantData* pLocationName, LFVariantData* pLocationGPS);
	SHORT OnSetLocationName(LPCAIRPORT lpcAirport) const;
	SHORT OnSetLocationGPS(LPCAIRPORT lpcAirport) const;
};


// CPropertyRating
//

class CPropertyRating : public CProperty
{
public:
	CPropertyRating(LFVariantData* pVData, CWnd* pOwnerWnd);

	virtual INT GetMinWidth() const;
	virtual void DrawValue(CDC& dc, LPCRECT lpcRect) const;
	virtual HCURSOR SetCursor(INT x) const;
	virtual BOOL CanDelete() const;
	virtual BOOL WantsChars() const;
	virtual BOOL OnClickValue(INT x) const;
	virtual BOOL OnPushChar(UINT nChar) const;

protected:
	void OnSetRating(UCHAR Rating) const;
};


// CPropertyNumber
//

class CPropertyNumber : public CProperty
{
public:
	CPropertyNumber(LFVariantData* pVData, CWnd* pOwnerWnd);

	virtual CMFCMaskedEdit* CreateEditControl(const CRect& rect) const;
};


// CPropertySize
//

class CPropertySize : public CProperty
{
public:
	CPropertySize(LFVariantData* pVData, CWnd* pOwnerWnd);

	virtual CMFCMaskedEdit* CreateEditControl(const CRect& rect) const;
};


// CPropertyColor
//

class CPropertyColor : public CProperty
{
public:
	CPropertyColor(LFVariantData* pVData, CWnd* pOwnerWnd);

	virtual INT GetMinWidth() const;
	virtual void DrawValue(CDC& dc, LPCRECT lpcRect) const;
	virtual HCURSOR SetCursor(INT x) const;
	virtual BOOL CanDelete() const;
	virtual BOOL WantsChars() const;
	virtual BOOL OnClickValue(INT x) const;
	virtual BOOL OnPushChar(UINT nChar) const;

protected:
	void OnSetColor(BYTE Color) const;

	static CIcons m_ColorDots;
};


// CPropertyGeoCoordinates
//

class CPropertyGeoCoordinates : public CProperty
{
public:
	CPropertyGeoCoordinates(LFVariantData* pVData, CWnd* pOwnerWnd);

	virtual BOOL HasButton() const;
	virtual HCURSOR SetCursor(INT x) const;
	virtual BOOL OnClickValue(INT x) const;
	virtual void OnClickButton(const STOREID& StoreID=DEFAULTSTOREID());
};


// CPropertyTime
//

class CPropertyTime : public CProperty
{
public:
	CPropertyTime(LFVariantData* pVData, CWnd* pOwnerWnd);

	virtual BOOL HasButton() const;
	virtual HCURSOR SetCursor(INT x) const;
	virtual BOOL OnClickValue(INT x) const;
	virtual void OnClickButton(const STOREID& StoreID=DEFAULTSTOREID());
};


// CPropertyDuration
//

class CPropertyDuration : public CPropertyNumber
{
public:
	CPropertyDuration(LFVariantData* pData, CWnd* pOwnerWnd);

	virtual CMFCMaskedEdit* CreateEditControl(const CRect& rect) const;
};


// CPropertyGenre
//

class CPropertyGenre : public CProperty
{
public:
	CPropertyGenre(LFVariantData* pVData, CWnd* pOwnerWnd);

virtual BOOL HasButton() const;
	virtual BOOL OnClickValue(INT x) const;
	virtual void OnClickButton(const STOREID& StoreID=DEFAULTSTOREID());
};


// CInspectorHeader
//

class CInspectorHeader
{
public:
	virtual INT GetPreferredHeight() const=0;
	virtual void DrawHeader(CDC& dc, Graphics& g, const CRect& rect, BOOL Themed) const=NULL;
};


// CInspectorGrid
//

struct PropertyItemData
{
	ItemData Hdr;
	CProperty* pProperty;
	UINT Attr;
	UINT CategoryID;
	WCHAR Name[LFAttributeNameSize];
	INT LabelWidth;
	BOOL Editable;
};

#define INSPECTORGRIDMARGIN     2

class CInspectorGrid : public CFrontstageItemView
{
public:
	CInspectorGrid();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL GetContextMenu(CMenu& Menu, INT Index);
	virtual void AdjustLayout();

	BOOL Create(CWnd* pOwnerWnd, UINT nID, UINT ContextMenuID, CInspectorHeader* pHeader=NULL);
	void SetContext(UINT Context);
	void SetStore(const STOREID& StoreID);
	void AddProperty(LFVariantData* pVData, LPCWSTR pszName, UINT CategoryID=LFAttrCategoryInternal, BOOL Visible=FALSE, BOOL Editable=FALSE);
	void AddAttributeProperties(LFVariantData* pVDataArray, SIZE_T ItemSize=sizeof(LFVariantData), UINT ItemCount=LFAttributeCount);
	void UpdatePropertyState(UINT Attr, BOOL Multiple, BOOL Editable, BOOL Visible, const LFVariantData* pVDataRange=NULL);
	void SetAlphabeticMode(BOOL SortAlphabetic);
	INT GetMinWidth() const;

protected:
	virtual INT GetItemCategory(INT Index) const;
	virtual void ShowTooltip(const CPoint& point);
	virtual void FireSelectedItem();
	virtual void DeleteSelectedItem();
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);
	virtual void DrawStage(CDC& dc, Graphics& g, const CRect& rect, const CRect& rectUpdate, BOOL Themed);
	virtual BOOL AllowItemEditLabel(INT Index) const;
	virtual LFFont* GetLabelFont() const;
	virtual RECT GetLabelRect() const;
	virtual CEdit* CreateLabelEditControl();
	virtual void EndLabelEdit(INT Index, CString& Value);

	void ResetProperty(INT Index, BOOL Force=FALSE);
	void SortItems();
	UINT PartAtPosition(const CPoint& point, INT Index) const;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);

	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	static HICON hIconResetNormal;
	static HICON hIconResetSelected;
	static HICON hIconResetHot;
	static HICON hIconResetPressed;
	STOREID m_StoreID;
	CInspectorHeader* m_pHeader;
	INT m_LabelWidth;
	INT m_MinWidth;
	INT m_IconSize;
	UINT m_Context;
	BOOL m_SortAlphabetic;
	BOOL m_PartPressed;
	UINT m_HoverPart;

private:
	PropertyItemData* GetPropertyItemData(INT Index) const;
	PropertyItemData* GetPropertyItemDataForAttribute(UINT Attr) const;
	INT GetItemHeight() const;
	static void SetPropertyName(PropertyItemData& Data, LPCWSTR pszName);
	static INT __stdcall CompareItems(PropertyItemData* pData1, PropertyItemData* pData2, const SortParameters& Parameters);
	void ModifyProperty(SHORT Attr);
	void NotifyOwner(INT Attr) const;

	UINT m_ContextMenuID;
};

inline PropertyItemData* CInspectorGrid::GetPropertyItemData(INT Index) const
{
	return (PropertyItemData*)GetItemData(Index);
}

inline void CInspectorGrid::SetStore(const STOREID& StoreID)
{
	m_StoreID = StoreID;
}

inline INT CInspectorGrid::GetMinWidth() const
{
	return m_MinWidth;
}

inline void CInspectorGrid::NotifyOwner(INT Attr) const
{
	SendMessage(WM_PROPERTYCHANGED, (WPARAM)Attr, (LPARAM)-1);
}
