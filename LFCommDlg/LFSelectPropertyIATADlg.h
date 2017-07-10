
// LFSelectPropertyIATADlg.h: Schnittstelle der Klasse LFSelectPropertyIATADlg
//

#pragma once
#include "CExplorerList.h"
#include "LFSelectLocationIATADlg.h"


// LFSelectPropertyIATADlg
//

class LFSelectPropertyIATADlg : public LFSelectLocationIATADlg
{
public:
	LFSelectPropertyIATADlg(CWnd* pParentWnd=NULL, const LPCSTR pAirport=NULL, BOOL AllowOverwriteName=FALSE, BOOL AllowOverwriteGPS=FALSE);

	BOOL m_OverwriteName;
	BOOL m_OverwriteGPS;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	BOOL m_AllowOverwriteName;
	BOOL m_AllowOverwriteGPS;
};
