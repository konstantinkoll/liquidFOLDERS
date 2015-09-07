
// LFSelectLocationIATADlg.h: Schnittstelle der Klasse LFSelectLocationIATADlg
//

#pragma once
#include "CExplorerList.h"
#include "LFDialog.h"


// LFSelectLocationIATADlg
//

#define MaxAirportsPerCountry   2500

class LFSelectLocationIATADlg : public LFDialog
{
public:
	LFSelectLocationIATADlg(BOOL IsPropertyDialog, CWnd* pParentWnd=NULL, CHAR* Airport=NULL, BOOL AllowOverwriteName=FALSE, BOOL AllowOverwriteGPS=FALSE);

	virtual void DoDataExchange(CDataExchange* pDX);

	LFAirport* p_Airport;
	BOOL m_OverwriteName;
	BOOL m_OverwriteGPS;

protected:
	void Sort();
	void LoadCountry(UINT Country);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnSelectCountry();
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequestTextColor(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortItems(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	BOOL m_IsPropertyDialog;
	BOOL m_AllowOverwriteName;
	BOOL m_AllowOverwriteGPS;
	UINT m_LastCountrySelected;
	UINT m_LastSortColumn;
	BOOL m_LastSortDirection;

	CExplorerList m_wndAirportList;
	LFAirport* p_Airports[MaxAirportsPerCountry];
	INT m_AirportCount;

private:
	INT Compare(INT n1, INT n2);
	void Heap(INT Wurzel, INT Anzahl);
};
