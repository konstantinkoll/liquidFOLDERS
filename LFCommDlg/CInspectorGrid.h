
// CInspectorGrid: Schnittstelle der Klasse CInspectorGrid
//

#pragma once
#include "liquidFOLDERS.h"
#include "CImageListTransparent.h"
#include "DynArray.h"
#include "LFTooltip.h"


// CInspectorProperty
//

class AFX_EXT_CLASS CInspectorGrid;

class AFX_EXT_CLASS CInspectorProperty
{
public:
	CInspectorProperty(LFVariantData* pData);

	virtual void ToString(WCHAR* tmpStr, INT nCount);
	virtual void DrawValue(CDC& dc, CRect rect);

	void SetParent(CInspectorGrid* pParent);
	void SetMultiple(BOOL Multiple);
	void ResetModified();

protected:
	CInspectorGrid* p_Parent;
	LFVariantData* p_Data;
	BOOL m_Modified;
	BOOL m_Multiple;
};


// CInspectorPropertyRating
//

class AFX_EXT_CLASS CInspectorPropertyRating : public CInspectorProperty
{
public:
	CInspectorPropertyRating(LFVariantData* pData);

	virtual void ToString(WCHAR* tmpStr, INT nCount);
	virtual void DrawValue(CDC& dc, CRect rect);
};


// CInspectorHeader
//

class AFX_EXT_CLASS CInspectorHeader
{
public:
	virtual INT GetPreferredHeight();
	virtual void DrawHeader(CDC& dc, CRect rect, BOOL Themed);
};


// CInspectorGrid
//

struct Property
{
	CInspectorProperty* pProperty;
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

class AFX_EXT_CLASS CInspectorGrid : public CWnd
{
friend class CInspectorProperty;
friend class CInspectorPropertyRating;

public:
	CInspectorGrid();
	~CInspectorGrid();

	virtual void PreSubclassWindow();
	virtual void AdjustLayout();

	BOOL Create(CWnd* pParentWnd, UINT nID, CInspectorHeader* pHeader=NULL);
	void AddProperty(CInspectorProperty* pProperty, UINT Category, WCHAR* Name, BOOL Editable=FALSE);
	void AddAttributes(LFVariantData* pData);
	void ShowHeader(BOOL ShowHeader);
	void SetAlphabeticMode(BOOL SortAlphabetic);
	void UpdatePropertyState(UINT nID, BOOL Multiple, BOOL Editable, BOOL Visible);

protected:
	LFApplication* p_App;
	DynArray<Property> m_Properties;
	PropertyCategory m_Categories[LFAttrCategoryCount];
	HTHEME hThemeButton;
	HTHEME hThemeList;
	HICON hIconResetNormal;
	HICON hIconResetHot;
	LFTooltip m_TooltipCtrl;
	CInspectorHeader* m_pHeader;
	CString m_MultipleValues;
	CFont m_BoldFont;
	CFont m_ItalicFont;
	INT m_FontHeight[2];
	INT m_RowHeight;
	INT m_LabelWidth;
	INT m_IconSize;
	BOOL m_ShowHeader;
	BOOL m_SortAlphabetic;
	BOOL m_Hover;
	INT m_ScrollHeight;
	INT m_VScrollPos;
	INT m_VScrollMax;
	INT m_HotItem;
	INT m_SelectedItem;

	virtual void Init();

	RECT GetItemRect(INT Item);
	INT HitTest(CPoint point);
	void InvalidateItem(INT Item);
	void SelectItem(INT Item);
	void ResetScrollbars();
	void AdjustScrollbars();
	void MakeSortArrayDirty();
	void DrawCategory(CDC& dc, CRect& rectCategory, WCHAR* Text);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	DECLARE_MESSAGE_MAP()

private:
	INT* m_pSortArray;
	CImageListTransparent m_AttributeIcons;

	void CreateFonts();
	INT Compare(INT eins, INT zwei);
	void Heap(INT wurzel, INT anz);
	void CreateSortArray();
};
