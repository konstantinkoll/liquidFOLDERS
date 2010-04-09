
// CMapPreviewCtrl.h: Schnittstelle der Klasse CMapPreviewCtrl
//

#pragma once
#include "liquidFOLDERS.h"
#include "CGdiPlusBitmap.h"


// CMapPreviewCtrl
//

static CGdiPlusBitmapResource* Map2 = NULL;

class CMapPreviewCtrl : public CWnd
{
public:
	CMapPreviewCtrl();
	virtual ~CMapPreviewCtrl();

	void Update(LFAirport* _Airport);

protected:
	CGdiPlusBitmapResource* m_Indicator;
	LFAirport* m_Airport;
	LFGeoCoordinates m_Location;
	GraphicsPath m_TextPath;
	BOOL m_FirstPathDraw;

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

private:
	OSVERSIONINFO osInfo;
};
