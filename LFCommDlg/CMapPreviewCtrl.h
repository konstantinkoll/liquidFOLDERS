
// CMapPreviewCtrl.h: Schnittstelle der Klasse CMapPreviewCtrl
//

#pragma once
#include "LFCore.h"


// CMapPreviewCtrl
//

class CMapPreviewCtrl : public CWnd
{
public:
	CMapPreviewCtrl();

	void Update(LFAirport* pAirport);

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

	LFAirport* p_Airport;
	LFGeoCoordinates m_Location;
	GraphicsPath m_TextPath;
	BOOL m_FirstPathDraw;
};
