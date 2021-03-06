
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

class CMapCtrl : public CFrontstageWnd
{
public:
	CMapCtrl();

	virtual void PreSubclassWindow();

	void SetLocation(const LFGeoCoordinates& Coord);

protected:
	virtual void ShowTooltip(const CPoint& point);
	virtual BOOL GetContextMenu(CMenu& Menu, INT Index);

	void LocationFromPoint(const CPoint& point, DOUBLE& Latitude, DOUBLE& Longitude) const;
	void PointFromLocation(INT& PosX, INT& PosY) const;
	void SetLocation(const CPoint& point);

	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()

	LFGeoCoordinates m_Location;

private:
	void SendNotifyMessage();
	void PrepareBitmap(const CRect& rect);

	INT m_BackBufferL;
	INT m_BackBufferH;
	HBRUSH hBackgroundBrush;
	BOOL m_Blink;
	UINT m_RemainVisible;
	UINT m_BackgroundMenuID;
	BOOL m_HighlightFirst;
};
