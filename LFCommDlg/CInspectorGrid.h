
// CInspectorGrid: Schnittstelle der Klasse CInspectorGrid
//

#pragma once
#include "liquidFOLDERS.h"
#include "DynArray.h"
#include "LFTooltip.h"


// CInspectorProperty
//

class AFX_EXT_CLASS CInspectorProperty
{
public:
	CInspectorProperty(LFVariantData* pData);

	virtual void DrawValue(CDC& dc, CRect rect);

protected:
	LFVariantData* p_Data;
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
public:
	CInspectorGrid();

	virtual void PreSubclassWindow();
	virtual void AdjustLayout();

	BOOL Create(CWnd* pParentWnd, UINT nID, BOOL bBorder);
	void AddProperty(CInspectorProperty* pProperty, UINT Category, WCHAR* Name, BOOL Editable=FALSE);
	void AddAttributes(LFVariantData* pData);
	void SetAlphabeticMode(BOOL SortAlphabetic);
	void UpdatePropertyState(UINT nID, BOOL Multiple, BOOL Editable, BOOL Visible);

protected:
	LFApplication* p_App;
	DynArray<Property> m_Properties;
	PropertyCategory m_Categories[LFAttrCategoryCount];
	HTHEME hTheme;
	LFTooltip m_TooltipCtrl;
	INT m_FontHeight[2];
	INT m_RowHeight;
	INT m_LabelWidth;
	BOOL m_SortAlphabetic;

	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	DECLARE_MESSAGE_MAP()
};
