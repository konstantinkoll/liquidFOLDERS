
// LFSelectLocationIATADlg.h: Schnittstelle der Klasse LFSelectLocationIATADlg
//

#pragma once
#include "CFrontstageItemView.h"
#include "LFDialog.h"
#include "IATA.h"


// CAirportList
//

struct AirportItemData
{
	ItemData Hdr;
	LPCAIRPORT lpcAirport;
};

#define AIRPORTCOLUMNS     2

class CAirportList sealed : public CFrontstageItemView
{
public:
	CAirportList();

	BOOL SetAirports(UINT CountryID, LPCAIRPORT lpcAirportSelected=NULL);
	LPCAIRPORT GetSelectedAirport() const;

protected:
	virtual void UpdateHeaderColumn(ATTRIBUTE Attr, HDITEM& HeaderItem) const;
	virtual void HeaderColumnClicked(ATTRIBUTE Attr);
	virtual void AdjustLayout();
	virtual void ShowTooltip(const CPoint& point);
	virtual COLORREF GetItemTextColor(INT Index, BOOL Themed) const;
	virtual void DrawItemCell(CDC& dc, CRect& rectCell, INT Index, ATTRIBUTE Attr, BOOL Themed);
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);

	void SortItems();

private:
	void UpdateHeader();
	LPCAIRPORT GetAirport(INT Index) const;
	void AddAirport(LPCAIRPORT lpcAirport);
	static INT __stdcall CompareItems(AirportItemData* pData1, AirportItemData* pData2, const SortParameters& Parameters);

	static CString m_SubitemName;
	static UINT m_SortAttribute;
	static BOOL m_SortDescending;
	INT m_ColumnOrder[AIRPORTCOLUMNS];
	INT m_ColumnWidth[AIRPORTCOLUMNS];
};

inline LPCAIRPORT CAirportList::GetAirport(INT Index) const
{
	return ((AirportItemData*)GetItemData(Index))->lpcAirport;
}

inline void CAirportList::SortItems()
{
	CFrontstageItemView::SortItems((PFNCOMPARE)CompareItems, m_SortAttribute, m_SortDescending);
}

inline void CAirportList::UpdateHeader()
{
	CFrontstageItemView::UpdateHeader(m_ColumnOrder, m_ColumnWidth);
}


// LFSelectLocationIATADlg
//

class LFSelectLocationIATADlg : public LFDialog
{
public:
	LFSelectLocationIATADlg(CWnd* pParentWnd=NULL, LPCSTR lpcAirport=NULL, UINT nIDTemplate=IDD_SELECTLOCATIONIATA);

	LPCAIRPORT p_Airport;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	LPCAIRPORT GetSelectedAirport() const;

	afx_msg void OnCloseUp();
	afx_msg void OnSelectCountry();
	DECLARE_MESSAGE_MAP()

	UINT m_LastCountrySelected;
	UINT m_LastSortColumn;
	BOOL m_LastSortDirection;

	CAirportList m_wndAirportList;
};

inline LPCAIRPORT LFSelectLocationIATADlg::GetSelectedAirport() const
{
	return m_wndAirportList.GetSelectedAirport();
}
