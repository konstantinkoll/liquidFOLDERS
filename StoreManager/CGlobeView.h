
// CGlobeView.h: Schnittstelle der Klasse CGlobeView
//

#pragma once
#include "liquidFOLDERS.h"
#include "StoreManager.h"
#include "CFileView.h"
#include "GLTexture.h"
#include "GLFont.h"


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

class CGlobeView : public CFileView
{
public:
	CGlobeView();

	virtual CMenu* GetBackgroundContextMenu();

	BOOL Create(CWnd* pParentWnd, UINT nID, LFSearchResult* Result, INT FocusItem=0);

protected:
	GLfloat m_GlobeLatitude;
	GLfloat m_GlobeLongitude;
	INT m_GlobeZoom;

	CClientDC* m_pDC;
	HGLRC m_hRC;
	INT m_Width;
	INT m_Height;
	GLTexture* m_TextureGlobe;
	GLTexture* m_TextureIcons;
	GLFont m_Fonts[2];

	virtual void SetViewOptions(BOOL Force);
	virtual void SetSearchResult(LFSearchResult* Result);
	virtual INT ItemAtPosition(CPoint point);
	virtual CMenu* GetItemContextMenu(INT idx);

	void PrepareModel();
	void CalcAndDrawSpots(GLdouble ModelView[4][4], GLdouble Projection[4][4]);
	void DrawStatusBar(INT Height, GLfloat BackColor[], BOOL Themed);
	void DrawScene(BOOL InternalCall=FALSE);
	void Normalize();

	void Init();
	void PrepareTexture();
	void Done();
	BOOL UpdateScene(BOOL Redraw=FALSE);
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
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

private:
	LPCTSTR lpszCursorName;
	HCURSOR hCursor;

	GLint m_GlobeModel;
	INT m_CurrentGlobeTexture;

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
	CPoint m_CursorPos;
	BOOL m_LockUpdate;
	BOOL m_IsHQModel;

	CPoint m_GrabPoint;
	BOOL m_Grabbed;
	CString YouLookAt;

	BOOL CursorOnGlobe(CPoint point);
	void UpdateCursor();
};
