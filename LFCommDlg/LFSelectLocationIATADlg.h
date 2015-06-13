
// LFSelectLocationIATADlg.h: Schnittstelle der Klasse LFSelectLocationIATADlg
//

#pragma once
#include "CMapPreviewCtrl.h"


// LFSelectLocationIATADlg
//

#define MaxAirportsPerCountry   2500

class LFSelectLocationIATADlg : public CDialog
{
public:
	LFSelectLocationIATADlg(UINT nIDTemplate, CWnd* pParentWnd=NULL, CHAR* Airport=NULL, BOOL AllowOverwriteName=FALSE, BOOL AllowOverwriteGPS=FALSE);

	virtual void DoDataExchange(CDataExchange* pDX);

	LFAirport* p_Airport;
	UINT m_LastCountrySelected;
	UINT m_LastSortColumn;
	BOOL m_LastSortDirection;
	BOOL m_OverwriteName;
	BOOL m_OverwriteGPS;

protected:
	void Sort();
	void LoadCountry(UINT country);
	void UpdatePreview();

private:
	INT Compare(INT n1, INT n2);
	void Heap(INT Wurzel, INT Anz);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortItems(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelectCountry();
	afx_msg void OnReportError(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	CListCtrl m_wndList;
	CMapPreviewCtrl m_wndMap;
	LFAirport* m_Airports[MaxAirportsPerCountry];
	INT m_nAirports;
	UINT m_nIDTemplate;
	WCHAR m_Buffer[256];
	BOOL m_AllowOverwriteName;
	BOOL m_AllowOverwriteGPS;
};
