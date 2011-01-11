
// CGlobeView.h: Schnittstelle der Klasse CGlobeView
//

#pragma once
#include "liquidFOLDERS.h"
#include "StoreManager.h"
#include "CFileView.h"
#include "CTextures.h"
#include "CGLFont.h"


// Item data

struct GlobeItemData
{
	FVItemData Hdr;
	BOOL Valid;
	double World[3];
	INT ScreenPoint[2];
	GLfloat Alpha;
	WCHAR CoordString[32];
};


// CGlobeView
//

struct LocalSettings
{
	GLfloat Latitude;
	GLfloat Longitude;
	INT GlobeZoom;
};

class CGlobeView : public CFileView
{
public:
	CGlobeView();

	LocalSettings m_LocalSettings;
	BOOL m_CameraChanged;

	virtual CMenu* GetBackgroundContextMenu();

	BOOL Create(CWnd* pParentWnd, UINT nID, LFSearchResult* Result, INT FocusItem=0);

protected:
	CClientDC* m_pDC;
	HGLRC m_hrc;
	HCURSOR hCursor;
	CTexture* m_TextureGlobe;
	CGLFont m_SpecialFont;
	CGLFont m_Fonts[2];
	INT m_Width;
	INT m_Height;
	BOOL m_Grabbed;
	CPoint m_GrabPoint;
	CString YouLookAt;
	COLORREF m_ColorBack;
	COLORREF m_ColorText;
	COLORREF m_ColorHighlight;

	virtual void SetViewOptions(BOOL Force);
	virtual void SetSearchResult(LFSearchResult* Result);
	virtual INT ItemAtPosition(CPoint point);
	virtual CMenu* GetItemContextMenu(INT idx);

	void Init();
	void PrepareFont(BOOL large, BOOL granny);
	void PrepareTexture();
	void PrepareModel(BOOL HQ);
	void Done();
	BOOL SetupPixelFormat();
	void Normalize();
	BOOL UpdateScene(BOOL Redraw=FALSE);
	void DrawScene(BOOL InternalCall=FALSE);
	void CalcAndDrawPoints();
	void CalcAndDrawLabel();
	void DrawLabel(GlobeItemData* d, UINT cCaption, WCHAR* caption, WCHAR* subcaption, WCHAR* coordinates, WCHAR* description, BOOL focused);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnZoomIn();
	afx_msg void OnZoomOut();
	afx_msg void OnAutosize();
	afx_msg void OnJumpToLocation();
	afx_msg void OnOptions();
	afx_msg void OnGoogleEarth();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

private:
	GLint m_GlobeList[2];
	GLfloat m_Latitude;
	GLfloat m_Longitude;
	GLfloat m_Zoom;
	GLfloat m_Scale;
	GLfloat m_Radius;
	GLfloat m_FogEnd;
	GLfloat m_FogStart;
	GLfloat m_AnimStartLatitude;
	GLfloat m_AnimStartLongitude;
	UINT m_AnimCounter;
	LPCTSTR lpszCursorName;
	CPoint m_CursorPos;
	INT m_nTexture;
	BOOL m_LockUpdate;

	BOOL CursorOnGlobe(CPoint point);
	void UpdateCursor();
};
