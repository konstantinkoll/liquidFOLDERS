
// LFSelectLocationIATADlg.h: Schnittstelle der Klasse LFSelectLocationIATADlg
//

#pragma once
#include "CExplorerList.h"
#include "LFDialog.h"


// LFSelectLocationIATADlg
//

#define MaxAirportsPerCountry     2500

class LFSelectLocationIATADlg : public LFDialog
{
public:
	LFSelectLocationIATADlg(CWnd* pParentWnd=NULL, LPCSTR pAirport=NULL, UINT nIDTemplate=IDD_SELECTLOCATIONIATA);

	LPCAIRPORT p_Airport;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	void Sort();
	void LoadCountry(UINT Country);

	afx_msg void OnSelectCountry();
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequestTextColor(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortItems(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	UINT m_LastCountrySelected;
	UINT m_LastSortColumn;
	BOOL m_LastSortDirection;

	CExplorerList m_wndAirportList;
	LPCAIRPORT p_Airports[MaxAirportsPerCountry];
	INT m_AirportCount;

private:
	INT Compare(INT n1, INT n2);
	static void Swap(LPCAIRPORT& Eins, LPCAIRPORT& Zwei);
	void Heap(INT Element, INT Count);
};

inline void LFSelectLocationIATADlg::Swap(LPCAIRPORT& Eins, LPCAIRPORT& Zwei)
{
	LPCAIRPORT lpcAirport = Eins;
	Eins = Zwei;
	Zwei = lpcAirport;
}
