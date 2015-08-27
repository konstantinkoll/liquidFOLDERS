
// CInspectorGrid: Schnittstelle der Klasse CInspectorGrid
//

#pragma once
#include "CImageListTransparent.h"
#include "LFCore.h"


// CPropertyHolder
//

#define WM_PROPERTYCHANGED     WM_USER+4

class CPropertyHolder : public CWnd
{
friend class CProperty;
friend class CPropertyTags;
friend class CPropertyRating;
friend class CPropertyIATA;
friend class CPropertyGPS;
friend class CPropertyTime;

public:
	CPropertyHolder();

	void SetStore(CHAR* StoreID);

protected:
	virtual void NotifyOwner(SHORT Attr1, SHORT Attr2=-1, SHORT Attr3=-1)=NULL;

	void CreateFonts();
	CProperty* CreateProperty(LFVariantData* pData);

	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	DECLARE_MESSAGE_MAP()

	CString m_MultipleValues;
	CFont m_BoldFont;
	CFont m_ItalicFont;
	BOOL m_StoreIDValid;
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

	virtual void ToString(WCHAR* tmpStr, INT nCount);
	virtual void DrawValue(CDC& dc, CRect rect);
	virtual HCURSOR SetCursor(INT x);
	virtual CString GetValidChars();
	virtual void SetEditMask(CMFCMaskedEdit* pEdit);
	virtual BOOL CanDelete();
	virtual BOOL HasButton();
	virtual BOOL WantsChars();
	virtual void OnSetString(CString Value);
	virtual BOOL OnClickValue(INT x);
	virtual void OnClickButton();
	virtual BOOL OnPushChar(UINT nChar);

	void SetParent(CPropertyHolder* pParentWnd);
	void SetMultiple(BOOL Multiple, LFVariantData* pRangeFirst, LFVariantData* pRangeSecond);
	void ResetModified();
	LFVariantData* GetData();

protected:
	CPropertyHolder* p_Parent;
	LFVariantData* p_Data;
	BOOL m_Modified;
	BOOL m_Multiple;
	BOOL m_ShowRange;
	LFVariantData m_RangeFirst;
	LFVariantData m_RangeSecond;
};


// CPropertyTags
//

class CPropertyTags : public CProperty
{
public:
	CPropertyTags(LFVariantData* pData);

	virtual BOOL HasButton();
	virtual void OnClickButton();
};


// CPropertyRating
//

class CPropertyRating : public CProperty
{
public:
	CPropertyRating(LFVariantData* pData);

	virtual void DrawValue(CDC& dc, CRect rect);
	virtual HCURSOR SetCursor(INT x);
	virtual BOOL CanDelete();
	virtual BOOL WantsChars();
	virtual BOOL OnClickValue(INT x);
	virtual BOOL OnPushChar(UINT nChar);
};


// CPropertyIATA
//

class CPropertyIATA : public CProperty
{
friend class CInspectorGrid;

public:
	CPropertyIATA(LFVariantData* pData, LFVariantData* pLocationName, LFVariantData* pLocationGPS);

	virtual CString GetValidChars();
	virtual BOOL HasButton();
	virtual void OnSetString(CString Value);
	virtual void OnClickButton();

protected:
	LFVariantData* p_LocationName;
	LFVariantData* p_LocationGPS;
};


// CPropertyGPS
//

class CPropertyGPS : public CProperty
{
public:
	CPropertyGPS(LFVariantData* pData);

	virtual BOOL HasButton();
	virtual HCURSOR SetCursor(INT x);
	virtual BOOL OnClickValue(INT x);
	virtual void OnClickButton();
};


// CPropertyTime
//

class CPropertyTime : public CProperty
{
public:
	CPropertyTime(LFVariantData* pData);

	virtual BOOL HasButton();
	virtual HCURSOR SetCursor(INT x);
	virtual BOOL OnClickValue(INT x);
	virtual void OnClickButton();
};


// CPropertyNumber
//

class CPropertyNumber : public CProperty
{
public:
	CPropertyNumber(LFVariantData* pData);

	virtual CString GetValidChars();
};


// CPropertySize
//

class CPropertySize : public CProperty
{
public:
	CPropertySize(LFVariantData* pData);

	virtual CString GetValidChars();
};


// CPropertyDuration
//

class CPropertyDuration : public CPropertyNumber
{
public:
	CPropertyDuration(LFVariantData* pData);

	virtual void SetEditMask(CMFCMaskedEdit* pEdit);
};


// CInspectorHeader
//

class CInspectorHeader
{
public:
	virtual INT GetPreferredHeight();
	virtual void DrawHeader(CDC& dc, CRect rect, BOOL Themed);
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
	void AddProperty(CProperty* pProperty, UINT Category, WCHAR* Name, BOOL Editable=FALSE);
	void AddAttributes(LFVariantData* pData);
	void SetAlphabeticMode(BOOL SortAlphabetic);
	void UpdatePropertyState(UINT nID, BOOL Multiple, BOOL Editable, BOOL Visible, LFVariantData* pRangeFirst=NULL, LFVariantData* pRangeSecond=NULL);
	CString GetName(UINT nID);
	CString GetValue(UINT nID);

protected:
	virtual void Init();
	virtual void ScrollWindow(INT dx, INT dy);
	virtual void NotifyOwner(SHORT Attr1, SHORT Attr2=-1, SHORT Attr3=-1);

	RECT GetItemRect(INT Item);
	INT HitTest(CPoint point, UINT* PartID=NULL);
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

	LFDynArray<Property> m_Properties;
	PropertyCategory m_Categories[LFAttrCategoryCount];
	HICON hIconResetNormal;
	HICON hIconResetSelected;
	HICON hIconResetHot;
	HICON hIconResetPressed;
	CInspectorHeader* m_pHeader;
	CMFCMaskedEdit* p_Edit;
	INT m_FontHeight[2];
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
	INT Compare(INT Eins, INT Zwei);
	void Heap(INT Wurzel, INT Anzahl);
	void CreateSortArray();

	INT* m_pSortArray;
	CImageListTransparent m_AttributeIcons;
};
