
// CGlobeView.cpp: Implementierung der Klasse CGlobeView
//

#include "stdafx.h"
#include "CGlobeView.h"
#include "GlobeOptionsDlg.h"
#include "liquidFOLDERS.h"
#include <math.h>


// CGlobeView
//

#define GetItemData(Index)     ((GlobeItemData*)(m_ItemData+Index*m_DataSize))
#define DISTANCENEAR           3.0f
#define DISTANCEFAR            17.0f
#define DOLLY                  0.09f
#define BLENDOUT               0.075f
#define BLENDIN                0.275f
#define ARROWSIZE              8
#define ANIMLENGTH             200
#define MOVEDELAY              10
#define MOVEDIVIDER            8.0f
#define WHITE                  100

CGlobeView::CGlobeView()
	: CFileView(sizeof(GlobeItemData), FALSE, TRUE, TRUE, FALSE, FALSE)
{
	m_RenderContext.pDC = NULL;
	m_RenderContext.hRC = NULL;

	lpszCursorName = IDC_WAIT;
	hCursor = theApp.LoadStandardCursor(IDC_WAIT);
	m_CursorPos.x = m_CursorPos.y = 0;

	m_nTextureBlueMarble = m_nTextureClouds = m_nTextureLocationIndicator = m_nGlobeModel = m_nHaloModel = 0;
	m_GlobeRadius = m_Momentum = 0.0f;
	m_Grabbed = FALSE;
	m_AnimCounter = m_MoveCounter = 0;
}

BOOL CGlobeView::Create(CWnd* pParentWnd, UINT nID, const CRect& rect, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data, UINT nClassStyle)
{
	return CFileView::Create(pParentWnd, nID, rect, pRawFiles, pCookedFiles, Data, nClassStyle | CS_OWNDC);
}

void CGlobeView::SetViewOptions(BOOL Force)
{
	// Settings
	if (Force)
	{
		m_GlobeCurrent.Latitude = m_GlobeTarget.Latitude = p_ViewParameters->GlobeLatitude/1000.0f;
		m_GlobeCurrent.Longitude = m_GlobeTarget.Longitude = p_ViewParameters->GlobeLongitude/1000.0f;
		m_GlobeCurrent.Zoom = m_GlobeTarget.Zoom = p_ViewParameters->GlobeZoom;
	}

	// Textures
	if (m_RenderContext.hRC)
	{
		CWaitCursor csr;

		theRenderer.MakeCurrent(m_RenderContext);

		theRenderer.CreateTextureBlueMarble(m_nTextureBlueMarble);
		theRenderer.CreateTextureClouds(m_nTextureClouds);
		theRenderer.CreateTextureLocationIndicator(m_nTextureLocationIndicator);

		Invalidate();
	}
}

void CGlobeView::SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data)
{
	CFileView::SetSearchResult(pRawFiles, pCookedFiles, Data);

	if (p_CookedFiles)
		if (p_CookedFiles->m_ItemCount)
			for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
			{
				LFGeoCoordinates Location = { 0.0, 0.0 };

				if (m_ViewParameters.SortBy==LFAttrLocationIATA)
				{
					LFAirport* pAirport;
					if (LFIATAGetAirportByCode((CHAR*)p_CookedFiles->m_Items[a]->AttributeValues[LFAttrLocationIATA], &pAirport))
						Location = pAirport->Location;
				}
				else
					if (p_CookedFiles->m_Items[a]->AttributeValues[m_ViewParameters.SortBy])
					{
						ASSERT(theApp.m_Attributes[m_ViewParameters.SortBy].Type==LFTypeGeoCoordinates);

						Location = *((LFGeoCoordinates*)p_CookedFiles->m_Items[a]->AttributeValues[m_ViewParameters.SortBy]);
					}

				if ((Location.Latitude!=0.0) || (Location.Longitude!=0))
				{
					GlobeItemData* pData = GetItemData(a);

					// Calculate world coordinates
					const DOUBLE LatitudeRad = -theRenderer.DegToRad(Location.Latitude);
					const DOUBLE LongitudeRad = theRenderer.DegToRad(Location.Longitude);

					const DOUBLE D = cos(LatitudeRad);

					pData->World[0] = (GLfloat)(sin(LongitudeRad)*D);
					pData->World[1] = (GLfloat)(sin(LatitudeRad));
					pData->World[2] = (GLfloat)(cos(LongitudeRad)*D);

					LFGeoCoordinatesToString(Location, pData->CoordString, 32, FALSE);
					wcscpy_s(pData->DescriptionString, 32, p_CookedFiles->m_Items[a]->Description);

					pData->Hdr.Valid = TRUE;
					pData->Hdr.RectInflate = ARROWSIZE;
				}
			}

	if (Data)
		if (Data->LocationValid)
			m_GlobeCurrent = Data->Location;
}

INT CGlobeView::ItemAtPosition(CPoint point) const
{
	if (!p_CookedFiles)
		return -1;

	INT Result = -1;
	GLfloat Alpha = 0.0f;

	for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
	{
		GlobeItemData* pData = GetItemData(a);

		if (pData->Hdr.Valid)
			if ((pData->Alpha>0.75f) || ((pData->Alpha>0.1f) && (pData->Alpha>Alpha-0.05f)))
				if (PtInRect(&pData->Hdr.Rect, point))
				{
					Alpha = pData->Alpha;
					Result = (INT)a;
				}
	}

	return Result;
}

CMenu* CGlobeView::GetViewContextMenu()
{
	CMenu* pMenu = new CMenu();
	pMenu->LoadMenu(IDM_GLOBE);

	return pMenu;
}

CMenu* CGlobeView::GetItemContextMenu(INT Index)
{
	CMenu* pMenu = CFileView::GetItemContextMenu(Index);
	
	CMenu* pPopup = pMenu->GetSubMenu(0);
	ASSERT_VALID(pPopup);

	CString tmpStr((LPCSTR)IDS_CONTEXTMENU_OPENGOOGLEEARTH);
	pPopup->InsertMenu(1, MF_STRING | MF_BYPOSITION, IDM_GLOBE_GOOGLEEARTH, tmpStr);

	return pMenu;
}

void CGlobeView::GetPersistentData(FVPersistentData& Data) const
{
	CFileView::GetPersistentData(Data);

	Data.Location = m_GlobeCurrent;
	Data.LocationValid = TRUE;
}

BOOL CGlobeView::CursorOnGlobe(const CPoint& point) const
{
	const GLfloat DistX = (GLfloat)point.x-(GLfloat)m_RenderContext.Width/2.0f;
	const GLfloat DistY = (GLfloat)point.y-(GLfloat)m_RenderContext.Height/2.0f;

	return DistX*DistX + DistY*DistY < m_GlobeRadius*m_GlobeRadius;
}

void CGlobeView::UpdateCursor()
{
	LPCTSTR Cursor;

	if (m_Grabbed)
	{
		Cursor = IDC_HAND;
	}
	else
	{
		Cursor = IDC_ARROW;

		if (CursorOnGlobe(m_CursorPos))
			if (ItemAtPosition(m_CursorPos)==-1)
				Cursor = IDC_HAND;
	}

	if (Cursor!=lpszCursorName)
	{
		hCursor = theApp.LoadStandardCursor(Cursor);

		SetCursor(hCursor);
		lpszCursorName = Cursor;
	}
}

void CGlobeView::WriteGoogleAttribute(CStdioFile& f, LFItemDescriptor* pItemDescriptor, UINT Attr)
{
	WCHAR tmpStr[256];
	LFAttributeToString(pItemDescriptor, Attr, tmpStr, 256);

	if (tmpStr[0]!='\0')
	{
		f.WriteString(_T("&lt;b&gt;"));
		f.WriteString(theApp.m_Attributes[Attr].Name);
		f.WriteString(_T("&lt;/b&gt;: "));
		f.WriteString(tmpStr);
		f.WriteString(_T("&lt;br&gt;"));
	}
}


// OpenGL
//

__forceinline void CGlobeView::CalcAndDrawSpots(const GLfloat ModelView[4][4], const GLfloat Projection[4][4])
{
	GLfloat SizeX = m_RenderContext.Width/2.0f;
	GLfloat SizeY = m_RenderContext.Height/2.0f;

	GLfloat MVP[4][4];
	theRenderer.MatrixMultiplication4f(MVP, ModelView, Projection);

	for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
	{
		GlobeItemData* pData = GetItemData(a);

		if (pData->Hdr.Valid)
		{
			pData->Alpha = 0.0f;

			GLfloat Z = ModelView[0][2]*pData->World[0] + ModelView[1][2]*pData->World[1] + ModelView[2][2]*pData->World[2];
			if (Z>BLENDOUT)
			{
				const GLfloat W = MVP[0][3]*pData->World[0] + MVP[1][3]*pData->World[1] + MVP[2][3]*pData->World[2] + MVP[3][3];
				const GLfloat X = (MVP[0][0]*pData->World[0] + MVP[1][0]*pData->World[1] + MVP[2][0]*pData->World[2] + MVP[3][0])*SizeX/W + SizeX + 0.5f;
				const GLfloat Y = -(MVP[0][1]*pData->World[0] + MVP[1][1]*pData->World[1] + MVP[2][1]*pData->World[2] + MVP[3][1])*SizeY/W + SizeY + 0.5f;

				pData->ScreenPoint[0] = (INT)X;
				pData->ScreenPoint[1] = (INT)Y;
				pData->Alpha = (Z<BLENDIN) ? (GLfloat)((Z-BLENDOUT)/(BLENDIN-BLENDOUT)) : 1.0f;

				if (m_ViewParameters.GlobeShowSpots)
					theRenderer.DrawIcon(X, Y, 6.0f+8.0f*pData->Alpha, pData->Alpha);
			}
		}
	}
}

__forceinline void CGlobeView::CalcAndDrawLabel(BOOL Themed)
{
	for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
	{
		GlobeItemData* pData = GetItemData(a);

		if (pData->Hdr.Valid)
			if (pData->Alpha>0.0f)
			{
				// Beschriftung
				WCHAR* Caption = p_CookedFiles->m_Items[a]->CoreAttributes.FileName;
				UINT cCaption = (UINT)wcslen(Caption);

				WCHAR* Subcaption = NULL;
				WCHAR* Coordinates = (m_ViewParameters.GlobeShowGPS ? pData->CoordString : NULL);
				
				WCHAR* Description = (m_ViewParameters.GlobeShowDescription ? pData->DescriptionString : NULL);
				if (Description)
					if (*Description==L'\0')
						Description = NULL;

				// Beschriftung aufbereiten
				switch (m_ViewParameters.SortBy)
				{
				case LFAttrLocationIATA:
					if (cCaption>6)
					{
						if (m_ViewParameters.GlobeShowAirportNames)
						{
							Subcaption = &Caption[3];
							while ((*Subcaption==L' ') || (*Subcaption==L'–') || (*Subcaption==L'—'))
								Subcaption++;
						}

						cCaption = 3;
					}

					break;

				case LFAttrLocationGPS:
					if ((wcscmp(Caption, pData->CoordString)==0) && (m_ViewParameters.GlobeShowGPS))
						Coordinates = NULL;

					break;
				}

				DrawLabel(pData, cCaption, Caption, Subcaption, Coordinates, Description, m_FocusItem==(INT)a, m_HotItem==(INT)a, Themed);
			}
	}
}

__forceinline void CGlobeView::DrawLabel(GlobeItemData* pData, UINT cCaption, WCHAR* Caption, WCHAR* Subcaption, WCHAR* Coordinates, WCHAR* Description, BOOL Focused, BOOL Hot, BOOL Themed)
{
	ASSERT(ARROWSIZE>3);

	// Width
	UINT W1 = m_Fonts[1].GetTextWidth(Caption, cCaption);
	UINT W2 = m_Fonts[0].GetTextWidth(Subcaption);
	UINT W3 = m_Fonts[0].GetTextWidth(Coordinates);
	UINT W4 = m_Fonts[0].GetTextWidth(Description);
	UINT Width = max(2*ARROWSIZE, max(W1, max(W2, max(W3, W4)))+11);

	// Height
	UINT Height = 8;
	Height += m_Fonts[1].GetTextHeight(Caption);
	Height += m_Fonts[0].GetTextHeight(Subcaption);
	Height += m_Fonts[0].GetTextHeight(Coordinates);
	Height += m_Fonts[0].GetTextHeight(Description);

	// Position and bounding rectangle
	INT Top = (pData->ScreenPoint[1]<m_RenderContext.Height/2) ? -1 : 1;

	INT x = pData->Hdr.Rect.left = pData->ScreenPoint[0]-ARROWSIZE-(((INT)Width-2*ARROWSIZE)*(m_RenderContext.Width-pData->ScreenPoint[0])/m_RenderContext.Width);
	INT y = pData->Hdr.Rect.top = pData->ScreenPoint[1]+(ARROWSIZE-2)*Top-(Top<0 ? (INT)Height : 0);
	pData->Hdr.Rect.right = x+Width;
	pData->Hdr.Rect.bottom = y+Height;

	// Visible?
	if ((x+Width+6<0) || (x-1>m_RenderContext.Width) || (y+Height+ARROWSIZE+6<0) || (y-ARROWSIZE-6>m_RenderContext.Height))
	{
		pData->Alpha = 0.0f;
		return;
	}

	// Colors
	BOOL Selected = pData->Hdr.Selected;

	GLcolor BorderColor;
	theRenderer.ColorRef2GLColor(BorderColor, Themed ? (Focused && m_ShowFocusRect && (GetFocus()==this)) || Selected ? 0xE08010 : Hot ? 0xF0C08A : 0xD5D1D0 : GetSysColor(Selected ? COLOR_HIGHLIGHT : COLOR_3DSHADOW));

	GLcolor BackgroundColor;
	theRenderer.ColorRef2GLColor(BackgroundColor, Themed ? 0xFFFFFF : GetSysColor(Selected ? COLOR_HIGHLIGHT : COLOR_WINDOW));

	// Shadow
	if (Themed)
	{
		glColor4f(0.0f, 0.0f, 0.0f, pData->Alpha*(12.0f/256.0f));
		glBegin(GL_LINES);
		glVertex2i(x+2, y+Height);
		glVertex2i(x+Width-1, y+Height);
		glVertex2i(x+Width, y+2);
		glVertex2i(x+Width, y+Height-1);
		glVertex2i(x+Width-1, y+Height-1);
		glVertex2i(x+Width, y+Height-1);
		glEnd();
	}

	// Inner
	if (Themed && (Hot | Selected))
	{
		glBegin(GL_QUADS);
		theRenderer.SetColor(*(Selected ? &m_TopColorSelected : &m_TopColorHot), pData->Alpha);
		glVertex2i(x+1, y+1);
		glVertex2i(x+Width-1, y+1);

		theRenderer.SetColor(*(Selected ? &m_BottomColorSelected : &m_BottomColorHot), pData->Alpha);
		glVertex2i(x+Width-1, y+Height-1);
		glVertex2i(x+1, y+Height-1);
		glEnd();

		glColor4f(1.0f, 1.0f, 1.0f, (((Hot && !Selected) ? 0x60 : 0x48)*pData->Alpha)/255.0f);

		glBegin(GL_LINE_LOOP);
		glVertex2i(x+1, y+1);
		glVertex2i(x+Width-2, y+1);
		glVertex2i(x+Width-2, y+Height-2);
		glVertex2i(x+1, y+Height-2);
		glEnd();

		theRenderer.SetColor(*(Top>0 ? Selected ? &m_TopColorSelected : &m_TopColorHot : Selected ? &m_BottomColorSelected : &m_BottomColorHot), pData->Alpha);
	}
	else
	{
		theRenderer.SetColor(BackgroundColor, pData->Alpha);
		glRecti(x+1, y+1, x+Width-1, y+Height-1);
	}

	// Arrow
	glBegin(GL_TRIANGLES);
	glVertex2i(pData->ScreenPoint[0], pData->ScreenPoint[1]);

	if (Top>0)
	{
		glVertex2i(pData->ScreenPoint[0]+ARROWSIZE+1, pData->ScreenPoint[1]+ARROWSIZE);
		glVertex2i(pData->ScreenPoint[0]-ARROWSIZE, pData->ScreenPoint[1]+ARROWSIZE);
	}
	else
	{
		glVertex2i(pData->ScreenPoint[0]+ARROWSIZE, pData->ScreenPoint[1]-ARROWSIZE);
		glVertex2i(pData->ScreenPoint[0]-ARROWSIZE, pData->ScreenPoint[1]-ARROWSIZE);
	}
	glEnd();

	// Border
	glBegin(GL_LINES);
	theRenderer.SetColor(BorderColor, pData->Alpha);

	glVertex2i(x, y+2);					// Left
	glVertex2i(x, y+Height-2);
	glVertex2i(x+Width-1, y+2);			// Right
	glVertex2i(x+Width-1, y+Height-2);

	if (Top>0)
	{
		glVertex2i(x+2, y+Height-1);	// Bottom
		glVertex2i(x+Width-2, y+Height-1);
		glEnd();

		glBegin(GL_LINE_STRIP);
		glVertex2i(x+2, y);
		glVertex2i(pData->ScreenPoint[0]-(ARROWSIZE-1), y);
		glVertex2i(pData->ScreenPoint[0], pData->ScreenPoint[1]-1);
		glVertex2i(pData->ScreenPoint[0]+(ARROWSIZE-1), y);
		glVertex2i(x+Width-2, y);
	}
	else
	{
		glVertex2i(x+2, y);				// Top
		glVertex2i(x+Width-2, y);
		glEnd();

		glBegin(GL_LINE_STRIP);
		glVertex2i(x+2, y+Height-1);
		glVertex2i(pData->ScreenPoint[0]-(ARROWSIZE-1), y+Height-1);
		glVertex2i(pData->ScreenPoint[0], pData->ScreenPoint[1]);
		glVertex2i(pData->ScreenPoint[0]+(ARROWSIZE-1), y+Height-1);
		glVertex2i(x+Width-2, y+Height-1);
	}
	glEnd();

	theRenderer.SetColor(BorderColor, pData->Alpha*0.5f);

	glBegin(GL_POINTS);
	glVertex2i(x, y+1);					// Upper left
	glVertex2i(x+1, y+1);
	glVertex2i(x+1, y);
	glEnd();

	glBegin(GL_POINTS);
	glVertex2i(x, y+Height-2);			// Lower left
	glVertex2i(x+1, y+Height-2);
	glVertex2i(x+1, y+Height-1);
	glEnd();

	glBegin(GL_POINTS);
	glVertex2i(x+Width-1, y+1);			// Upper right
	glVertex2i(x+Width-2, y+1);
	glVertex2i(x+Width-2, y);
	glEnd();

	glBegin(GL_POINTS);
	glVertex2i(x+Width-1, y+Height-2);	// Lower right
	glVertex2i(x+Width-2, y+Height-2);
	glVertex2i(x+Width-2, y+Height-1);
	glEnd();

	x += 5;
	y += 3;

	// Caption
	m_Fonts[1].Begin(*(Selected ? &m_SelectedColor : &m_CaptionColor), pData->Alpha);
	y += m_Fonts[1].Render(Caption, x, y, cCaption);
	m_Fonts[1].End();

	// Hints
	m_Fonts[0].Begin(*(Selected ? &m_SelectedColor : &m_TextColor), pData->Alpha);

	if (Subcaption)
		y += m_Fonts[0].Render(Subcaption, x, y);

	if (Coordinates)
		y += m_Fonts[0].Render(Coordinates, x, y);

	// Description
	if (Description)
	{
		if (!Selected)
			m_Fonts[0].SetColor(m_AttrColor, pData->Alpha);

		y += m_Fonts[0].Render(Description, x, y);
	}

	m_Fonts[0].End();
}

BOOL CGlobeView::UpdateScene(BOOL Redraw)
{
	BOOL Result = Redraw;

	// Do not roll over ploes
	if (m_GlobeTarget.Latitude<-75.0f)
		m_GlobeTarget.Latitude = -75.0f;

	if (m_GlobeTarget.Latitude>75.0f)
		m_GlobeTarget.Latitude = 75.0f;

	// Normalize rotation
	if (m_GlobeTarget.Longitude<-180.0f)
		m_GlobeTarget.Longitude += 360.0f;

	if (m_GlobeTarget.Longitude>180.0f)
		m_GlobeTarget.Longitude -= 360.0f;

	// Zoom
	if (m_GlobeTarget.Zoom<0)
		m_GlobeTarget.Zoom = 0;

	if (m_GlobeTarget.Zoom>1000)
		m_GlobeTarget.Zoom = 1000;

	if (m_GlobeCurrent.Zoom<=m_GlobeTarget.Zoom-5)
	{
		m_GlobeCurrent.Zoom += 5;
		m_HotItem = -1;

		Result = TRUE;
	}
	else
		if (m_GlobeCurrent.Zoom>=m_GlobeTarget.Zoom+5)
		{
			m_GlobeCurrent.Zoom -= 5;
			m_HotItem = -1;

			Result = TRUE;
		}
		else
		{
			Result |= (m_GlobeCurrent.Zoom!=m_GlobeTarget.Zoom);

			m_GlobeCurrent.Zoom = m_GlobeTarget.Zoom;
		}

	// Animation
	if (m_AnimCounter)
	{
		const GLfloat Factor = (GLfloat)((cos(PI*m_AnimCounter/ANIMLENGTH)+1.0)/2.0);

		m_GlobeCurrent.Latitude = m_AnimStartLatitude*(1.0f-Factor) + m_GlobeTarget.Latitude*Factor;
		m_GlobeCurrent.Longitude = m_AnimStartLongitude*(1.0f-Factor) + m_GlobeTarget.Longitude*Factor;

		if (m_GlobeTarget.Zoom<600)
		{
			INT Distance = 600-m_GlobeTarget.Zoom;
			INT MaxDistance = (INT)((m_GlobeTarget.Zoom+100)*1.5f);

			const GLfloat Factor = (GLfloat)sin(PI*m_AnimCounter/ANIMLENGTH);

			m_GlobeCurrent.Zoom = (INT)(m_GlobeTarget.Zoom*(1.0f-Factor)+(m_GlobeTarget.Zoom+min(Distance, MaxDistance))*Factor);
		}

		m_AnimCounter--;

		Result = TRUE;
	}
	else
	{
		if (m_Momentum!=0.0f)
			m_GlobeTarget.Longitude += m_Momentum;

		Result |= (m_GlobeCurrent.Latitude!=m_GlobeTarget.Latitude) || (m_GlobeCurrent.Longitude!=m_GlobeTarget.Longitude);

		m_GlobeCurrent.Latitude = m_GlobeTarget.Latitude;
		m_GlobeCurrent.Longitude = m_GlobeTarget.Longitude;
	}

	if (Result)
	{
		Invalidate();
		UpdateCursor();
	}

	return Result;
}

__forceinline void CGlobeView::RenderScene(BOOL Themed)
{
	theRenderer.BeginRender(this, m_RenderContext);

	//Clear background
	//
	GLcolor BackColor;
	theRenderer.ColorRef2GLColor(BackColor, Themed ? 0xF8F5F4 : GetSysColor(COLOR_WINDOW));

	glClearColor(BackColor[0], BackColor[1], BackColor[2], 1.0f);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// White top gradient
	if (Themed)
	{
		theRenderer.Project2D();
		glBegin(GL_QUADS);

		theRenderer.SetColor(BackColor);
		glVertex2i(0, WHITE-1);
		glVertex2i(m_RenderContext.Width, WHITE-1);

		glColor3f(1.0, 1.0, 1.0);
		glVertex2i(m_RenderContext.Width, 0);
		glVertex2i(0, 0);

		glEnd();
	}

	// Setup colors
	//
	theRenderer.ColorRef2GLColor(m_AttrColor, Themed ? 0x333333 : GetSysColor(COLOR_WINDOWTEXT));
	theRenderer.ColorRef2GLColor(m_BottomColorHot, 0xFAEBE0);
	theRenderer.ColorRef2GLColor(m_BottomColorSelected, 0xE08010);
	theRenderer.ColorRef2GLColor(m_CaptionColor, Themed ? 0xCC3300 : GetSysColor(COLOR_WINDOWTEXT));
	theRenderer.ColorRef2GLColor(m_SelectedColor, Themed ? 0xFFFFFF : GetSysColor(COLOR_HIGHLIGHTTEXT));
	theRenderer.ColorRef2GLColor(m_TextColor, Themed ? 0xA39791 : GetSysColor(COLOR_3DSHADOW));
	theRenderer.ColorRef2GLColor(m_TopColorHot, 0xFFFCF9);
	theRenderer.ColorRef2GLColor(m_TopColorSelected, 0xFFA020);

	// Draw globe
	//

	// Distance
	GLfloat Distance = DISTANCENEAR+((DISTANCEFAR-DISTANCENEAR)*m_GlobeCurrent.Zoom)/1000.0f;
	ASSERT(Distance>0.0f);

	// Size
	m_GlobeRadius = (GLfloat)min(m_RenderContext.Width, m_RenderContext.Height)/(DOLLY*Distance*2.0f);

	// Modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Distance, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// Projection Matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	GLfloat ScaleX = (m_RenderContext.Width>m_RenderContext.Height) ? (GLfloat)m_RenderContext.Width/(GLfloat)(m_RenderContext.Height+1) : 1.0f;
	GLfloat ScaleY = (m_RenderContext.Height>m_RenderContext.Width) ? (GLfloat)m_RenderContext.Height/(GLfloat)(m_RenderContext.Width+1) : 1.0f;
	glFrustum(-ScaleX*DOLLY, ScaleX*DOLLY, -ScaleY*DOLLY, ScaleY*DOLLY, 1.0f, 1000.0f);

	// Halo
	if (min(theApp.m_ModelQuality, theRenderer.m_MaxModelQuality)>=MODELHIGH)
	{
		if (!m_nHaloModel)
		{
			m_nHaloModel = glGenLists(1);
			glNewList(m_nHaloModel, GL_COMPILE);

			GLcolor HaloColor;
			theRenderer.ColorRef2GLColor(HaloColor, BackColor[1]>=0.5f ? 0xFFFFFF : 0xFFD8C0);

			const GLfloat Radius = (BackColor[1]>=0.5f) ? 1.125f : 1.0125f;

			glBegin(GL_QUAD_STRIP);

			for (UINT a=0; a<=256; a++)
			{
				const GLfloat Winkel = 2*PI*a/256;
				const GLfloat X = sin(Winkel);
				const GLfloat Y = cos(Winkel);

				theRenderer.SetColor(HaloColor);
				glVertex3f(0.0f, X, Y);
	
				theRenderer.SetColor(HaloColor, 0.0f);
				glVertex3f(0.0f, X*Radius, Y*Radius);
			}

			glEnd();
			glEndList();
		}

		glCallList(m_nHaloModel);
	}

	// Lighting
	if (min(theApp.m_ModelQuality, theRenderer.m_MaxModelQuality)>=MODELMEDIUM)
	{
		GLfloat lAmbient[] = { 0.9f, 0.9f, 0.9f, 1.0f };
		GLfloat lDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat lSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glLightfv(GL_LIGHT0, GL_AMBIENT, lAmbient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lDiffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, lSpecular);

		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHTING);

		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lAmbient);
	}

	// Rotate globe (AFTER lighting)
	glMatrixMode(GL_MODELVIEW);
	glRotatef(m_GlobeCurrent.Latitude, 0.0f, 0.0f, 1.0f);
	glRotatef(m_GlobeCurrent.Longitude+90.0f, 0.0f, 1.0f, 0.0f);

	// Store matrices for later
	GLfloat MatrixModelView[4][4];
	GLfloat MatrixProjection[4][4];
	glGetFloatv(GL_MODELVIEW_MATRIX, &MatrixModelView[0][0]);
	glGetFloatv(GL_PROJECTION_MATRIX, &MatrixProjection[0][0]);

	// Atmosphere
	if (min(theApp.m_ModelQuality, theRenderer.m_MaxModelQuality)>=MODELHIGH)
	{
		glEnable(GL_FOG);
		glFogf(GL_FOG_START, Distance-0.5f);
		glFogf(GL_FOG_END, Distance+0.35f);

		const GLfloat FogColor[4] = { 0.65f, 0.75f, 0.95f, 1.0f };
		glFogfv(GL_FOG_COLOR, FogColor);
	}

	// Texture units
	if (min(theApp.m_ModelQuality, theRenderer.m_MaxModelQuality)>=MODELULTRA)
	{
		// Setup texture units for clouds
		// CloudAlpha depends on distance
		GLfloat CloudAlpha = (GLfloat)(m_GlobeCurrent.Zoom-300)/300.0f;
		if (CloudAlpha<0.0f)
			CloudAlpha = 0.0f;
		if (CloudAlpha>1.0f)
			CloudAlpha = 1.0f;

		// Texture unit 0
		// Multiply cloud texture with (0.4, 0.4, 0.4, CloudAlpha)
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_nTextureClouds);

		GLfloat AlphaColor[] = { 0.4f, 0.4f, 0.4f, CloudAlpha };
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, AlphaColor);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);

		// Texture unit 1
		// Interpolate cloud texture and Blue Marble texture depending on cloud's texture alpha
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_nTextureBlueMarble);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);

		// Texture unit 2
		// Modulate resulting texture with lighting-dependent vertex color
		glActiveTexture(GL_TEXTURE2);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_nTextureClouds);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	}
	else
	{
		// Simple texture mapping (either GL_MODULATE for lighting, or GL_REPLACE for "low" quality model)
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_nTextureBlueMarble);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, min(theApp.m_ModelQuality, theRenderer.m_MaxModelQuality)>=MODELMEDIUM ? GL_MODULATE : GL_REPLACE);
	}

	// Render globe model
	theRenderer.EnableMultisample();
	glEnable(GL_CULL_FACE);

	glCallList(m_nGlobeModel);

	glDisable(GL_CULL_FACE);
	theRenderer.DisableMultisample();

	// Disable texture units
	if (glActiveTexture)
	{
		glActiveTexture(GL_TEXTURE2);
		glDisable(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE0);
	}

	glDisable(GL_TEXTURE_2D);

	// Disable atmosphere
	glDisable(GL_FOG);

	// Disable lighting
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	// 2D overlay
	//
	theRenderer.Project2D();

	if (p_CookedFiles && !m_Nothing)
		if (p_CookedFiles->m_ItemCount)
		{
			// Draw locations
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, m_nTextureLocationIndicator);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			glBegin(GL_QUADS);
			CalcAndDrawSpots(MatrixModelView, MatrixProjection);
			glEnd();

			glDisable(GL_TEXTURE_2D);

			// Draw label
			CalcAndDrawLabel(Themed);
		}

	theRenderer.EndRender(this, m_RenderContext, Themed);
}


BEGIN_MESSAGE_MAP(CGlobeView, CFileView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEHOVER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()

	ON_COMMAND(IDM_GLOBE_JUMPTOLOCATION, OnJumpToLocation)
	ON_COMMAND(IDM_GLOBE_ZOOMIN, OnZoomIn)
	ON_COMMAND(IDM_GLOBE_ZOOMOUT, OnZoomOut)
	ON_COMMAND(IDM_GLOBE_AUTOSIZE, OnAutosize)
	ON_COMMAND(IDM_GLOBE_SETTINGS, OnSettings)
	ON_COMMAND(IDM_GLOBE_GOOGLEEARTH, OnGoogleEarth)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_GLOBE_JUMPTOLOCATION, IDM_GLOBE_GOOGLEEARTH, OnUpdateCommands)
END_MESSAGE_MAP()

INT CGlobeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFileView::OnCreate(lpCreateStruct)==-1)
		return -1;

	if (!theRenderer.Initialize())
		return -1;

	// OpenGL
	if (theRenderer.CreateRenderContext(this, m_RenderContext))
	{
		// Fonts
		m_Fonts[0].Create(&theApp.m_DefaultFont);
		m_Fonts[1].Create(&theApp.m_LargeFont);

		// Model
		m_nGlobeModel = theRenderer.CreateGlobe();

		// Timer
		SetTimer(1, 10, NULL);
	}

	return 0;
}

void CGlobeView::OnDestroy()
{
	// OpenGL
	if (m_RenderContext.hRC)
	{
		// Timer
		KillTimer(1);

		theRenderer.MakeCurrent(m_RenderContext);

		// Textures
		glDeleteTextures(1, &m_nTextureBlueMarble);
		glDeleteTextures(1, &m_nTextureClouds);
		glDeleteTextures(1, &m_nTextureLocationIndicator);

		// Models
		glDeleteLists(m_nHaloModel, 1);
		glDeleteLists(m_nGlobeModel, 1);
	}

	theRenderer.DeleteRenderContext(m_RenderContext);

	// Settings
	if (p_ViewParameters)
	{
		p_ViewParameters->GlobeLatitude = (INT)(m_GlobeTarget.Latitude*1000.0f);
		p_ViewParameters->GlobeLongitude = (INT)(m_GlobeTarget.Longitude*1000.0f);
		p_ViewParameters->GlobeZoom = m_GlobeTarget.Zoom;
	}

	CFileView::OnDestroy();
}

void CGlobeView::OnPaint()
{
	CPaintDC pDC(this);

	BOOL Themed = IsCtrlThemed();

	if (m_RenderContext.hRC)
	{
		RenderScene(Themed);
	}
	else
	{
		CRect rect;
		GetClientRect(rect);

		CDC dc;
		dc.CreateCompatibleDC(&pDC);
		dc.SetBkMode(TRANSPARENT);

		CBitmap MemBitmap;
		MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
		CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

		dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

		CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

		CRect rectText(rect);
		rectText.top += m_HeaderHeight+6;

		CString tmpStr((LPCSTR)IDS_NORENDERINGCONTEXT);

		dc.SetTextColor(0x0000FF);
		dc.DrawText(tmpStr, rectText, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

		DrawWindowEdge(dc, Themed);

		pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

		dc.SelectObject(pOldFont);
		dc.SelectObject(pOldBitmap);
	}
}

BOOL CGlobeView::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*Message*/)
{
	SetCursor(hCursor);

	return TRUE;
}

void CGlobeView::OnMouseMove(UINT nFlags, CPoint point)
{
	m_CursorPos = point;

	if (m_Grabbed)
	{
		m_MoveCounter = 0;

		const GLfloat Scale = ((GLfloat)min(m_RenderContext.Width, m_RenderContext.Height))/m_GlobeRadius;

		CSize szRotate = m_GrabPoint-point;
		m_GlobeTarget.Longitude = m_GlobeCurrent.Longitude += (m_LastMove=-szRotate.cx*Scale/4.0f)/4.0f;
		m_GlobeTarget.Latitude = m_GlobeCurrent.Latitude += szRotate.cy*Scale/16.0f;

		m_GrabPoint = point;

		UpdateScene(TRUE);
	}
	else
	{
		UpdateCursor();
	}

	CFileView::OnMouseMove(nFlags, point);
}

BOOL CGlobeView::OnMouseWheel(UINT /*nFlags*/, SHORT zDelta, CPoint /*pt*/)
{
	LFGetApp()->HideTooltip();

	if (zDelta<0)
	{
		OnZoomOut();
	}
	else
	{
		OnZoomIn();
	}

	return TRUE;
}

void CGlobeView::OnMouseHover(UINT nFlags, CPoint point)
{
	if (m_Momentum==0.0f)
	{
		CFileView::OnMouseHover(nFlags, point);
	}
	else
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = HOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
}

void CGlobeView::OnLButtonDown(UINT nFlags, CPoint point)
{
	INT Index = ItemAtPosition(point);
	if (Index==-1)
	{
		if (CursorOnGlobe(point))
		{
			m_Momentum = m_LastMove = 0.0f;

			m_GrabPoint = point;
			m_Grabbed = TRUE;

			if (m_AnimCounter)
			{
				m_AnimCounter = 0;
				m_GlobeTarget = m_GlobeCurrent;
			}

			SetCapture();

			UpdateCursor();
		}

		if (GetFocus()!=this)
			SetFocus();
	}
	else
	{
		CFileView::OnLButtonDown(nFlags, point);
	}
}

void CGlobeView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_Grabbed)
	{
		if (m_MoveCounter<MOVEDELAY)
			m_Momentum = m_LastMove/MOVEDIVIDER;

		m_Grabbed = FALSE;
		ReleaseCapture();

		UpdateCursor();
	}
	else
	{
		CFileView::OnLButtonUp(nFlags, point);
	}
}

void CGlobeView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CFileView::OnKeyDown(nChar, nRepCnt, nFlags);

	switch(nChar)
	{
	case VK_ADD:
	case VK_OEM_PLUS:
	case VK_PRIOR:
		OnZoomIn();
		break;

	case VK_SUBTRACT:
	case VK_OEM_MINUS:
	case VK_NEXT:
		OnZoomOut();
		break;

	case VK_HOME:
		OnAutosize();
		break;

	case 'L':
		if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
			OnJumpToLocation();

		break;
	}
}

void CGlobeView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
		UpdateScene();

	if (m_MoveCounter<1000)
		m_MoveCounter++;

	CFileView::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

void CGlobeView::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	m_Momentum = 0.0f;

	CFileView::OnContextMenu(pWnd, pos);
}


void CGlobeView::OnJumpToLocation()
{
	LFSelectLocationIATADlg dlg(FALSE, this);
	if (dlg.DoModal()==IDOK)
	{
		ASSERT(dlg.p_Airport);

		m_AnimCounter = ANIMLENGTH;
		m_AnimStartLatitude = m_GlobeCurrent.Latitude;
		m_AnimStartLongitude = m_GlobeCurrent.Longitude;
		m_GlobeTarget.Latitude = (GLfloat)dlg.p_Airport->Location.Latitude;
		m_GlobeTarget.Longitude = -(GLfloat)dlg.p_Airport->Location.Longitude;
		m_Momentum = 0.0f;

		UpdateScene();
	}
}

void CGlobeView::OnZoomIn()
{
	if (m_GlobeTarget.Zoom>0)
	{
		m_GlobeTarget.Zoom -= 100;

		UpdateScene();
	}
}

void CGlobeView::OnZoomOut()
{
	if (m_GlobeTarget.Zoom<1000)
	{
		m_GlobeTarget.Zoom += 100;

		UpdateScene();
	}
}

void CGlobeView::OnAutosize()
{
	m_GlobeTarget.Zoom = 600;

	UpdateScene();
}

void CGlobeView::OnSettings()
{
	GlobeOptionsDlg dlg(p_ViewParameters, this);
	if (dlg.DoModal()==IDOK)
		theApp.UpdateViewOptions(-1, LFViewGlobe);
}

void CGlobeView::OnGoogleEarth()
{
	if (theApp.m_PathGoogleEarth.IsEmpty())
		return;

	// Dateinamen finden
	TCHAR Pathname[MAX_PATH];
	if (!GetTempPath(MAX_PATH, Pathname))
		return;

	CString szTempName;
	srand(rand());
	szTempName.Format(_T("%sliquidFOLDERS%.4X%.4X.kml"), Pathname, 32768+rand(), 32768+rand());

	// Datei erzeugen
	FILE *fStream;
	if (_tfopen_s(&fStream, szTempName, _T("wt,ccs=UTF-8")))
	{
		LFErrorBox(this, LFDriveNotReady);
	}
	else
	{
		CStdioFile f(fStream);
		try
		{
			f.WriteString(_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<kml xmlns=\"http://earth.google.com/kml/2.0\">\n<Document>\n"));
			f.WriteString(_T("<Style id=\"A\"><IconStyle><scale>0.8</scale><Icon><href>http://maps.google.com/mapfiles/kml/pal4/icon57.png</href></Icon></IconStyle><LabelStyle><scale>0</scale></LabelStyle></Style>\n"));
			f.WriteString(_T("<Style id=\"B\"><IconStyle><scale>1.0</scale><Icon><href>http://maps.google.com/mapfiles/kml/pal4/icon57.png</href></Icon></IconStyle><LabelStyle><scale>1</scale></LabelStyle></Style>\n"));
			f.WriteString(_T("<StyleMap id=\"Coord\"><Pair><key>normal</key><styleUrl>#A</styleUrl></Pair><Pair><key>highlight</key><styleUrl>#B</styleUrl></Pair></StyleMap>\n"));

			INT Index = GetNextSelectedItem(-1);
			while (Index>-1)
			{
				LFGeoCoordinates Location = p_CookedFiles->m_Items[Index]->CoreAttributes.LocationGPS;
				if ((Location.Latitude!=0) || (Location.Longitude!=0))
				{
					f.WriteString(_T("<Placemark>\n<name>"));
					f.WriteString(p_CookedFiles->m_Items[Index]->CoreAttributes.FileName);
					f.WriteString(_T("</name>\n<description>"));

					WriteGoogleAttribute(f, p_CookedFiles->m_Items[Index], LFAttrLocationName);
					WriteGoogleAttribute(f, p_CookedFiles->m_Items[Index], LFAttrLocationIATA);
					WriteGoogleAttribute(f, p_CookedFiles->m_Items[Index], LFAttrLocationGPS);
					WriteGoogleAttribute(f, p_CookedFiles->m_Items[Index], LFAttrArtist);
					WriteGoogleAttribute(f, p_CookedFiles->m_Items[Index], LFAttrRoll);
					WriteGoogleAttribute(f, p_CookedFiles->m_Items[Index], LFAttrRecordingTime);
					WriteGoogleAttribute(f, p_CookedFiles->m_Items[Index], LFAttrComments);

					f.WriteString(_T("&lt;div&gt;</description>\n<styleUrl>#Location</styleUrl>\n"));

					CString tmpStr;
					tmpStr.Format(_T("<Point><coordinates>%.6lf,%.6lf,-5000</coordinates></Point>\n"), Location.Longitude, -Location.Latitude);

					f.WriteString(tmpStr+_T("</Placemark>\n"));
				}

				Index = GetNextSelectedItem(Index);
			}

			f.WriteString(_T("</Document>\n</kml>\n"));
			f.Close();

			ShellExecute(m_hWnd, _T("open"), szTempName, NULL, NULL, SW_SHOWNORMAL);
		}
		catch(CFileException ex)
		{
			LFErrorBox(this, LFDriveNotReady);
			f.Close();
		}
	}
}

void CGlobeView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = (m_RenderContext.hRC!=NULL);

	switch (pCmdUI->m_nID)
	{
	case IDM_GLOBE_ZOOMIN:
		b &= m_GlobeTarget.Zoom>0;
		break;

	case IDM_GLOBE_ZOOMOUT:
		b &= m_GlobeTarget.Zoom<1000;
		break;

	case IDM_GLOBE_AUTOSIZE:
		b &= m_GlobeTarget.Zoom!=600;
		break;

	case IDM_GLOBE_SETTINGS:
		b = TRUE;
		break;

	case IDM_GLOBE_GOOGLEEARTH:
		b &= (GetNextSelectedItem(-1)!=-1) && (!theApp.m_PathGoogleEarth.IsEmpty());
		break;
	}

	pCmdUI->Enable(b);
}
