
// LFPickStringDlg.h: Schnittstelle der Klasse LFPickStringDlg
//

#pragma once
#include "CFrontstageItemView.h"
#include "CInspectorGrid.h"


// CUnicodeStringList
//

struct StringItemData
{
	ItemData Hdr;
	WCHAR Label[256];
	WCHAR Description[256];
	UINT FileCount;
};

class CUnicodeStringList sealed : public CFrontstageItemView
{
public:
	CUnicodeStringList();

	void SetStrings(LFSearchResult* pSearchResult);
	void SelectString(const CString& UnicodeString);
	CString GetSelectedString() const;

protected:
	virtual void AdjustLayout();
	virtual INT GetItemCategory(INT Index) const;
	virtual void ShowTooltip(const CPoint& point);
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);

private:
	StringItemData* GetStringItemData(INT Index) const;
	void AddItem(const LFItemDescriptor* pItemDescriptor);
};

inline StringItemData* CUnicodeStringList::GetStringItemData(INT Index) const
{
	return (StringItemData*)GetItemData(Index);
}


// LFPickStringDlg
//

class LFPickStringDlg : public CAttributePickDlg
{
public:
	LFPickStringDlg(ATTRIBUTE Attr, ITEMCONTEXT Context, const CString& UnicodeString, CWnd* pParentWnd=NULL);

	CString m_UnicodeString;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void AdjustLayout(const CRect& rectLayout, UINT nFlags);
	virtual BOOL InitDialog();

	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	DECLARE_MESSAGE_MAP()

	CUnicodeStringList m_wndStringList;

private:
	BOOL m_SelectString;
};
