#pragma once
#include "LFCommDlg.h"
#include "CMapPreviewCtrl.h"

#define MaxAirportsPerCountry   2500

class AFX_EXT_CLASS LFSelectLocationIATADlg : public CDialog
{
public:
	LFSelectLocationIATADlg(CWnd* pParentWnd, UINT nIDTemplate, char* _Airport=NULL);
	virtual ~LFSelectLocationIATADlg();

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
	int m_nAirports;
	UINT m_nIDTemplate;
	wchar_t m_Buffer[256];
	LFApplication* p_App;

	int Compare(int col, int n1, int n2);
	void Heap(int col, int wurzel, int anz);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortItems(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelectCountry();
	afx_msg void OnReportError();
	DECLARE_MESSAGE_MAP()
};
