
// CMapCtrl.h: Schnittstelle der Klasse CMapCtrl
//

#pragma once
#include "LFCore.h"


// CMapCtrl
//

#define MAP_UPDATE_LOCATION     1

struct NM_GPSDATA
{
	NMHDR hdr;
	LFGeoCoordinates* pLocation;
};

class CMapCtrl : public CWnd
{
public:
	CMapCtrl();

	virtual void PreSubclassWindow();

	void SetMenu(UINT BackgroundMenuID=0, BOOL HighlightFirst=FALSE);
	void SetLocation(const LFGeoCoordinates& Coord);

protected:
	virtual void Init();

	void LocationFromPoint(const CPoint& point, DOUBLE& Latitude, DOUBLE& Longitude) const;
	void PointFromLocation(INT& PosX, INT& PosY) const;
	void SetLocation(const CPoint& point);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	DECLARE_MESSAGE_MAP()

	LFGeoCoordinates m_Location;

private:
	void SendUpdateMsg();

	INT m_BackBufferL;
	INT m_BackBufferH;
	HBRUSH hBackgroundBrush;
	CPoint m_LastTrack;
	BOOL m_Blink;
	UINT m_RemainVisible;
	UINT m_BackgroundMenuID;
	BOOL m_HighlightFirst;
};
