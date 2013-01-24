
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
	GLfloat World[3];
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
	virtual void GetPersistentData(FVPersistentData& Data);

	BOOL Create(CWnd* pParentWnd, UINT nID, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data);

protected:
	GlobeParameters m_GlobeTarget;
	GlobeParameters m_GlobeCurrent;

	CClientDC* m_pDC;
	HGLRC hRC;
	INT m_Width;
	INT m_Height;
	GLTexture* m_pTextureGlobe;
	GLTexture* m_pTextureIcons;
	GLFont m_Fonts[2];

	virtual void SetViewOptions(BOOL Force);
	virtual void SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data);
	virtual INT ItemAtPosition(CPoint point);
	virtual CMenu* GetItemContextMenu(INT idx);

	void PrepareModel();
	void PrepareTexture();
	void Normalize();
	void CalcAndDrawSpots(GLfloat ModelView[4][4], GLfloat Projection[4][4]);
	void CalcAndDrawLabel();
	void DrawLabel(GlobeItemData* d, UINT cCaption, WCHAR* Caption, WCHAR* Subcaption, WCHAR* Coordinates, WCHAR* Description, BOOL Focused);
	void DrawStatusBar(INT Height, GLfloat BackColor[], BOOL Themed);
	void DrawScene(BOOL InternalCall=FALSE);
	BOOL UpdateScene(BOOL Redraw=FALSE);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);

	afx_msg void OnJumpToLocation();
	afx_msg void OnZoomIn();
	afx_msg void OnZoomOut();
	afx_msg void OnAutosize();
	afx_msg void OnSettings();
	afx_msg void OnGoogleEarth();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	LPCTSTR lpszCursorName;
	HCURSOR hCursor;
	CPoint m_CursorPos;

	GLint m_GlobeModel;
	BOOL m_IsHQModel;
	INT m_CurrentGlobeTexture;

	GLfloat m_Scale;
	GLfloat m_Radius;
	GLfloat m_FogStart;
	GLfloat m_FogEnd;
	UINT m_AnimCounter;
	GLfloat m_AnimStartLatitude;
	GLfloat m_AnimStartLongitude;
	UINT m_MoveCounter;
	GLfloat m_LastMove;
	GLfloat m_Momentum;

	CPoint m_GrabPoint;
	BOOL m_Grabbed;
	BOOL m_LockUpdate;
	CString m_YouLookAt;

	BOOL CursorOnGlobe(CPoint point);
	void UpdateCursor();
};
