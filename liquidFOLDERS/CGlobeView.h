
// CGlobeView.h: Schnittstelle der Klasse CGlobeView
//

#pragma once
#include "CFileView.h"
#include "GLFont.h"


// Item Data

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

	virtual BOOL Create(CWnd* pParentWnd, UINT nID, const CRect& rect, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData=NULL, UINT nClassStyle=CS_DBLCLKS);
	virtual CMenu* GetViewContextMenu();
	virtual void GetPersistentData(FVPersistentData& Data) const;

protected:
	virtual void SetViewSettings(BOOL Force);
	virtual void SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData);
	virtual INT ItemAtPosition(CPoint point) const;
	virtual CMenu* GetItemContextMenu(INT Index);

	void CalcAndDrawSpots(const GLfloat ModelView[4][4], const GLfloat Projection[4][4]);
	void CalcAndDrawLabel(BOOL Themed);
	void DrawLabel(GlobeItemData* pData, SIZE_T cCaption, LPCWSTR pCaption, LPCWSTR pSubcaption, LPCWSTR pCoordinates, LPCWSTR pDescription, BOOL Focused, BOOL Hot, BOOL Themed);
	BOOL UpdateScene(BOOL Redraw=FALSE);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
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
	GLRenderContext m_RenderContext;

	GLuint m_nHaloModel;
	GLuint m_nGlobeModel;
	GLuint m_nTextureBlueMarble;
	GLuint m_nTextureClouds;
	GLuint m_nTextureLocationIndicator;

	GLFont m_Fonts[2];

	GLcolor m_AttrColor;
	GLcolor m_BottomColorHot;
	GLcolor m_BottomColorSelected;
	GLcolor m_CaptionColor;
	GLcolor m_SelectedColor;
	GLcolor m_TextColor;
	GLcolor m_TopColorHot;
	GLcolor m_TopColorSelected;

private:
	BOOL CursorOnGlobe(const CPoint& point) const;
	void UpdateCursor();
	static void WriteGoogleAttribute(CStdioFile& f, LFItemDescriptor* pItemDescriptor, UINT Attr);
	void RenderScene(BOOL Themed);

	static const GLcolor m_lAmbient;
	static const GLcolor m_lDiffuse;
	static const GLcolor m_lSpecular;
	static const GLcolor m_FogColor;

	LPCWSTR lpszCursorName;
	HCURSOR hCursor;
	CPoint m_CursorPos;

	GLfloat m_GlobeRadius;
	UINT m_AnimCounter;
	GLfloat m_AnimStartLatitude;
	GLfloat m_AnimStartLongitude;
	UINT m_MoveCounter;
	GLfloat m_LastMove;
	GLfloat m_Momentum;

	CPoint m_GrabPoint;
	BOOL m_Grabbed;
};
