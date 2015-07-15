
// CGlobeView.h: Schnittstelle der Klasse CGlobeView
//

#pragma once
#include "CFileView.h"
#include "GLFont.h"
#include "GLTexture.h"


// Item Data

struct GlobeItemData
{
	FVItemData Hdr;
	GLfloat World[3];
	INT ScreenPoint[2];
	GLfloat Alpha;
	WCHAR CoordString[32];
	WCHAR DescriptionString[32];
};


// CGlobeView
//

class CGlobeView : public CFileView
{
public:
	CGlobeView();

	virtual BOOL Create(CWnd* pParentWnd, UINT nID, CRect rect, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data=NULL, UINT nClassStyle=CS_DBLCLKS);
	virtual CMenu* GetViewContextMenu();
	virtual void GetPersistentData(FVPersistentData& Data);

protected:
	virtual void SetViewOptions(BOOL Force);
	virtual void SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data);
	virtual INT ItemAtPosition(CPoint point);
	virtual CMenu* GetItemContextMenu(INT Index);

	void PrepareModel();
	void PrepareTexture();
	void Normalize();
	void CalcAndDrawSpots(GLfloat ModelView[4][4], GLfloat Projection[4][4]);
	void CalcAndDrawLabel(BOOL Themed);
	void DrawLabel(GlobeItemData* d, UINT cCaption, WCHAR* Caption, WCHAR* Subcaption, WCHAR* Coordinates, WCHAR* Description, BOOL Focused, BOOL Hot, BOOL Themed);
	void DrawStatusBar(INT Height, COLORREF BarColor, BOOL Themed);
	void DrawScene(BOOL InternalCall=FALSE);
	BOOL UpdateScene(BOOL Redraw=FALSE);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt);
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

	GlobeParameters m_GlobeTarget;
	GlobeParameters m_GlobeCurrent;
	CClientDC* m_pDC;
	HGLRC hRC;
	INT m_Width;
	INT m_Height;
	GLTexture* m_pTextureGlobe;
	GLTexture* m_pTextureIcons;
	GLFont m_Fonts[2];

private:
	BOOL CursorOnGlobe(CPoint point);
	void UpdateCursor();

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
};
