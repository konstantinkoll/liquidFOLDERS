
// CExplorerList: Schnittstelle der Klasse CExplorerList
//

#pragma once
#include "CTooltipHeader.h"


// CExplorerList
//

#define REQUEST_TEXTCOLOR                3

struct NM_TEXTCOLOR
{
	NMHDR hdr;
	INT Item;
	COLORREF Color;
};

class CExplorerList : public CListCtrl
{
public:
	CExplorerList();

	virtual void PreSubclassWindow();

	void AddColumn(INT ID, LPCWSTR Name=L"", INT Width=100, BOOL Right=FALSE);

protected:
	virtual void Init();

	void SetWidgetSize();
	void DrawItem(INT nID, CDC* pDC);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	DECLARE_TOOLTIP()

	HTHEME m_hThemeButton;
	CSize m_CheckboxSize;

private:
	void DrawIcon(CDC* pDC, CRect& rect, LVITEM& Item, UINT State);

	CTooltipHeader m_wndHeader;

	INT m_ColumnCount;
	LVCOLUMN m_Columns[16];
	CImageList* p_ImageList;
	INT m_IconSize;
	UINT m_View;
};
