
// LFSelectLocationIATADlg.h: Schnittstelle der Klasse LFSelectLocationIATADlg
//

#pragma once
#include "LFCommDlg.h"
#include "CMapPreviewCtrl.h"


// LFSelectLocationIATADlg
//

#define MaxAirportsPerCountry   2500

class AFX_EXT_CLASS LFSelectLocationIATADlg : public CDialog
{
public:
	LFSelectLocationIATADlg(UINT nIDTemplate, CWnd* pParentWnd, CHAR* _Airport=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	LFAirport* m_Airport;
	UINT m_LastCountrySelected;
	BOOL m_IATA_OverwriteName;
	BOOL m_IATA_OverwriteGPS;

protected:
	void LoadCountry(UINT country, BOOL SelectFirst=TRUE);
	void UpdatePreview();

private:
	CMapPreviewCtrl m_Map;
	LFAirport* m_Airports[MaxAirportsPerCountry];
	INT m_nAirports;
	UINT m_nIDTemplate;
	WCHAR m_Buffer[256];
	LFApplication* p_App;

	INT Compare(INT col, INT n1, INT n2);
	void Heap(INT col, INT wurzel, INT anz);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortItems(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelectCountry();
	afx_msg void OnReportError(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
