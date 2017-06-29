
// CInspectorGrid: Schnittstelle der Klasse CInspectorGrid
//

#pragma once
#include "LFCore.h"


// CPropertyHolder
//

#define WM_PROPERTYCHANGED     WM_USER+4

class CPropertyHolder : public CFrontstageWnd
{
friend class CProperty;
friend class CPropertyTags;
friend class CPropertyRating;
friend class CPropertyIATA;
friend class CPropertyGPS;
friend class CPropertyTime;
friend class CPropertyNumber;
friend class CPropertyGenre;

public:
	CPropertyHolder();

	void SetStore(const LPCSTR pStoreID);

protected:
	virtual void NotifyOwner(SHORT Attr1, SHORT Attr2=-1, SHORT Attr3=-1)=NULL;

	CProperty* CreateProperty(LFVariantData* pData);

	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	DECLARE_MESSAGE_MAP()

	static CString m_MultipleValues;
	CHAR m_StoreID[LFKeySize];
};


// CProperty
//

class CProperty
{
friend class CInspectorGrid;
friend class CPropertyEdit;

public:
	CProperty(LFVariantData* pData);

	virtual void ToString(LPWSTR pStr, INT nCount) const;
	virtual void DrawValue(CDC& dc, LPCRECT lpRect) const;
	virtual HCURSOR SetCursor(INT x) const;
	virtual CString GetValidChars() const;
	virtual void SetEditMask(CMFCMaskedEdit* pWndEdit) const;
	virtual BOOL CanDelete() const;
	virtual BOOL HasButton() const;
	virtual BOOL WantsChars() const;
	virtual void OnSetString(CString& Value) const;
	virtual BOOL OnClickValue(INT x);
	virtual void OnClickButton();
	virtual BOOL OnPushChar(UINT nChar);

	void SetParent(CPropertyHolder* pParentWnd);
	void SetMultiple(BOOL Multiple, const LFVariantData* pRangeFirst, const LFVariantData* pRangeSecond);
	void ResetModified();
	LFVariantData* GetData() const;

protected:
	void NotifyOwner(SHORT Attr2=-1, SHORT Attr3=-1) const;

	CPropertyHolder* p_Parent;
	LFVariantData* p_Data;
	BOOL m_Modified;
	BOOL m_Multiple;
	BOOL m_ShowRange;
	LFVariantData m_RangeFirst;
	LFVariantData m_RangeSecond;
};

inline void CProperty::ResetModified()
{
	m_Modified = FALSE;
}

inline LFVariantData* CProperty::GetData() const
{
	return p_Data;
}

inline void CProperty::NotifyOwner(SHORT Attr2, SHORT Attr3) const
{
	ASSERT(p_Parent);

	p_Parent->NotifyOwner((SHORT)p_Data->Attr, Attr2, Attr3);
}


// CPropertyTags
//

class CPropertyTags : public CProperty
{
public:
	CPropertyTags(LFVariantData* pData);

	virtual BOOL HasButton() const;
	virtual void OnClickButton();
};


// CPropertyRating
//

class CPropertyRating : public CProperty
{
public:
	CPropertyRating(LFVariantData* pData);

	virtual void DrawValue(CDC& dc, LPCRECT lpRect) const;
	virtual HCURSOR SetCursor(INT x) const;
	virtual BOOL CanDelete() const;
	virtual BOOL WantsChars() const;
	virtual BOOL OnClickValue(INT x);
	virtual BOOL OnPushChar(UINT nChar);

protected:
	void OnSetRating(UCHAR Rating);
};


// CPropertyIATA
//

class CPropertyIATA : public CProperty
{
friend class CInspectorGrid;

public:
	CPropertyIATA(LFVariantData* pData, LFVariantData* pLocationName, LFVariantData* pLocationGPS);

	virtual CString GetValidChars() const;
	virtual BOOL HasButton() const;
	virtual void OnSetString(CString& Value) const;
	virtual void OnClickButton();

	void SetAdditionalData(LFVariantData* pLocationName, LFVariantData* pLocationGPS);

protected:
	LFVariantData* p_LocationName;
	LFVariantData* p_LocationGPS;
};

inline void CPropertyIATA::SetAdditionalData(LFVariantData* pLocationName, LFVariantData* pLocationGPS)
{
	p_LocationName = pLocationName;
	p_LocationGPS = pLocationGPS;
}


// CPropertyGPS
//

class CPropertyGPS : public CProperty
{
public:
	CPropertyGPS(LFVariantData* pData);

	virtual BOOL HasButton() const;
	virtual HCURSOR SetCursor(INT x) const;
	virtual BOOL OnClickValue(INT x);
	virtual void OnClickButton();
};


// CPropertyTime
//

class CPropertyTime : public CProperty
{
public:
	CPropertyTime(LFVariantData* pData);

	virtual BOOL HasButton() const;
	virtual HCURSOR SetCursor(INT x) const;
	virtual BOOL OnClickValue(INT x);
	virtual void OnClickButton();
};


// CPropertyNumber
//

class CPropertyNumber : public CProperty
{
public:
	CPropertyNumber(LFVariantData* pData);

	virtual CString GetValidChars() const;
};


// CPropertySize
//

class CPropertySize : public CProperty
{
public:
	CPropertySize(LFVariantData* pData);

	virtual CString GetValidChars() const;
};


// CPropertyDuration
//

class CPropertyDuration : public CPropertyNumber
{
public:
	CPropertyDuration(LFVariantData* pData);

	virtual void SetEditMask(CMFCMaskedEdit* pWndEdit) const;
};


// CPropertyGenre
//

class CPropertyGenre : public CProperty
{
public:
	CPropertyGenre(LFVariantData* pData);

	virtual CString GetValidChars() const;
	virtual BOOL HasButton() const;
	virtual BOOL OnClickValue(INT x);
	virtual void OnClickButton();
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

struct Property
{
	CProperty* pProperty;
	LFVariantData* pData;
	UINT Category;
	WCHAR Name[256];
	BOOL Visible;
	BOOL Editable;
	INT LabelWidth;
	INT Top;
	INT Bottom;
};

struct PropertyCategory
{
	INT Top;
	INT Bottom;
};

class CInspectorGrid : public CPropertyHolder
{
public:
	CInspectorGrid();
	~CInspectorGrid();

	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void AdjustLayout();

	BOOL Create(CWnd* pParentWnd, UINT nID, CInspectorHeader* pHeader=NULL);
	void AddProperty(CProperty* pProperty, UINT Category, LPCWSTR Name, BOOL Editable=FALSE);
	CProperty* AddAttributeProperty(LFVariantData* pData);
	void AddAttributeProperties(LFVariantData* pDataArray);
	void SetAlphabeticMode(BOOL SortAlphabetic);
	void UpdatePropertyState(UINT nID, BOOL Multiple, BOOL Editable, BOOL Visible, const LFVariantData* pRangeFirst=NULL, const LFVariantData* pRangeSecond=NULL);
	CString GetName(UINT nID) const;
	CString GetValue(UINT nID) const;

protected:
	virtual void Init();
	virtual void ScrollWindow(INT dx, INT dy, LPCRECT lpRect=NULL, LPCRECT lpClipRect=NULL);
	virtual void NotifyOwner(SHORT Attr1, SHORT Attr2=-1, SHORT Attr3=-1);

	RECT GetItemRect(INT Item) const;
	INT HitTest(const CPoint& point, UINT* PartID=NULL) const;
	void InvalidateItem(INT Item);
	void EnsureVisible(INT Item);
	void SelectItem(INT Item);
	void ResetScrollbars();
	void AdjustScrollbars();
	void MakeSortArrayDirty();
	void ResetProperty(UINT Attr);
	void EditProperty(UINT Attr);
	void DestroyEdit(BOOL Accept=FALSE);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg UINT OnGetDlgCode();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);
	afx_msg void OnDestroyEdit();
	DECLARE_MESSAGE_MAP()

	LFDynArray<Property, LFAttributeCount, 8> m_Properties;
	PropertyCategory m_Categories[LFAttrCategoryCount];
	static HICON hIconResetNormal;
	static HICON hIconResetSelected;
	static HICON hIconResetHot;
	static HICON hIconResetPressed;
	CInspectorHeader* m_pHeader;
	CMFCMaskedEdit* p_WndEdit;
	INT m_RowHeight;
	INT m_LabelWidth;
	INT m_IconSize;
	BOOL m_SortAlphabetic;
	BOOL m_Hover;
	BOOL m_PartPressed;
	INT m_ScrollHeight;
	INT m_VScrollPos;
	INT m_VScrollMax;
	INT m_HotItem;
	UINT m_HotPart;
	INT m_SelectedItem;
	INT m_EditItem;

private:
	INT Compare(INT Eins, INT Zwei) const;
	void Heap(INT Element, INT Count);
	void CreateSortArray();

	INT* m_pSortArray;
};

inline void CInspectorGrid::MakeSortArrayDirty()
{
	if (m_pSortArray)
	{
		delete[] m_pSortArray;
		m_pSortArray = NULL;
	}
}
