
// CGlobeView.h: Schnittstelle der Klasse CGlobeView
//

#pragma once
#include "liquidFOLDERS.h"
#include "StoreManager.h"
#include "CFileView.h"
#include "CTextures.h"
#include "CGLFont.h"


// CGlobeView
//

struct Location
{
	BOOL valid;
	BOOL selected;
	double world[3];
	int screenpoint[2];
	int screenlabel[4];
	GLfloat alpha;
	wchar_t coordstring[32];
};

struct LocalSettings
{
	GLfloat AngleY;
	GLfloat AngleZ;
	int GlobeZoom;
};

class CGlobeView : public CFileView
{
public:
	CGlobeView();
	virtual ~CGlobeView();

	LocalSettings m_LocalSettings;
	BOOL m_CameraChanged;

	void Create(CWnd* pParentWnd,  LFSearchResult* _result);
	virtual void SelectItem(int n, BOOL select=TRUE, BOOL InternalCall=FALSE);
	virtual int GetSelectedItem();
	virtual int GetNextSelectedItem(int n);

protected:
	CClientDC* m_pDC;
	HGLRC m_hrc;
	HCURSOR hCursor;
	CTexture* m_TextureGlobe;
	CGLFont* m_pSpecialFont;
	CGLFont* m_pFonts[2][2];
	CGLFont* SmallFont;
	CGLFont* LargeFont;
	int m_Width;
	int m_Height;
	GLint m_GlobeList[2];
	GLfloat m_AngleY;
	GLfloat m_AngleZ;
	GLfloat m_Zoom;
	GLfloat m_Scale;
	GLfloat m_Radius;
	GLfloat m_FogEnd;
	GLfloat m_FogStart;
	BOOL m_Grabbed;
	CPoint m_GrabPoint;
	COLORREF m_ColorBack;
	COLORREF m_ColorText;
	COLORREF m_ColorHighlight;
	Location* m_Locations;
	BOOL m_LockUpdate;

	virtual void SetViewOptions(UINT _ViewID, BOOL Force);
	virtual void SetSearchResult(LFSearchResult* _result);
	virtual BOOL IsSelected(int n);
	virtual int ItemAtPosition(CPoint point);
	virtual CMenu* GetContextMenu();
	void Init();
	void PrepareFont(BOOL large, BOOL granny);
	void PrepareTexture();
	void PrepareModel(BOOL HQ);
	void Done();
	BOOL SetupPixelFormat();
	BOOL UpdateScene(BOOL Redraw=FALSE);
	void DrawScene(BOOL InternalCall=FALSE);
	void CalcAndDrawPoints();
	void CalcAndDrawLabel();
	void DrawLabel(Location* loc, UINT cCaption, wchar_t* caption, wchar_t* subcaption, wchar_t* coordinates, wchar_t* description, BOOL focused);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnZoomIn();
	afx_msg void OnZoomOut();
	afx_msg void OnScaleToFit();
	afx_msg void OnSaveCamera();
	afx_msg void OnJumpToLocation();
	afx_msg void OnGoogleEarth();
	afx_msg void OnHQModel();
	afx_msg void OnShowBubbles();
	afx_msg void OnShowAirportNames();
	afx_msg void OnShowGPS();
	afx_msg void OnShowHints();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	DECLARE_MESSAGE_MAP()

private:
	LPCTSTR lpszCursorName;
	CPoint mPoint;
	int m_nTexture;

	BOOL CursorOnGlobe(CPoint point);
	void DisplayCursor(LPCTSTR lpszCursorName);
	void UpdateCursor();
};
