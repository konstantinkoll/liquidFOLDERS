
// CMapSelectionCtrl.h: Schnittstelle der Klasse CMapSelectionCtrl
//

#pragma once
#include "liquidFOLDERS.h"
#include "CGdiPlusBitmap.h"


// CMapSelectionCtrl
//

#define MAP_UPDATE_LOCATION     1

struct tagGPSDATA
{
	NMHDR hdr;
	LFGeoCoordinates* pCoord;
};

static CGdiPlusBitmapResource* Map1 = NULL;

class CMapSelectionCtrl : public CWnd
{
public:
	CMapSelectionCtrl();

	void OnBlink();
	void SetGeoCoordinates(const LFGeoCoordinates Coord);

protected:
	CGdiPlusBitmapResource m_Indicator;
	LFGeoCoordinates m_Coord;

	void UpdateLocation(CPoint point);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

private:
	BOOL m_Blink;
	UINT m_RemainVisible;

	void SendUpdateMsg();
};
