
// CGlobeView.cpp: Implementierung der Klasse CGlobeView
//

#include "stdafx.h"
#include "CGlobeView.h"
#include "Resource.h"
#include "LFCore.h"
#include "GlobeOptionsDlg.h"
#include <math.h>

#define DISTANCE       39.0f
#define ARROWSIZE      9
#define PI             3.14159265358979323846
#define ANIMLENGTH     200
#define MOVEDELAY      10
#define MOVEDIVIDER    8.0f
#define SPOT           2
#define CROSSHAIRS     3
#define WHITE          100

__forceinline void ColorRef2GLColor(GLfloat* dst, COLORREF src, GLfloat Alpha=1.0f)
{
	dst[0] = (src & 0xFF)/255.0f;
	dst[1] = ((src>>8) & 0xFF)/255.0f;
	dst[2] = ((src>>16) & 0xFF)/255.0f;
	dst[3] = Alpha;
}

__forceinline double decToRad(double dec)
{
	return dec*(PI/180.0);
}

__forceinline void MatrixMul(GLfloat Result[4][4], GLfloat Left[4][4], GLfloat Right[4][4])
{
	Result[0][0] = Left[0][0]*Right[0][0] + Left[0][1]*Right[1][0] + Left[0][2]*Right[2][0] + Left[0][3]*Right[3][0];
	Result[0][1] = Left[0][0]*Right[0][1] + Left[0][1]*Right[1][1] + Left[0][2]*Right[2][1] + Left[0][3]*Right[3][1];
	Result[0][2] = Left[0][0]*Right[0][2] + Left[0][1]*Right[1][2] + Left[0][2]*Right[2][2] + Left[0][3]*Right[3][2];
	Result[0][3] = Left[0][0]*Right[0][3] + Left[0][1]*Right[1][3] + Left[0][2]*Right[2][3] + Left[0][3]*Right[3][3];
	Result[1][0] = Left[1][0]*Right[0][0] + Left[1][1]*Right[1][0] + Left[1][2]*Right[2][0] + Left[1][3]*Right[3][0];
	Result[1][1] = Left[1][0]*Right[0][1] + Left[1][1]*Right[1][1] + Left[1][2]*Right[2][1] + Left[1][3]*Right[3][1];
	Result[1][2] = Left[1][0]*Right[0][2] + Left[1][1]*Right[1][2] + Left[1][2]*Right[2][2] + Left[1][3]*Right[3][2];
	Result[1][3] = Left[1][0]*Right[0][3] + Left[1][1]*Right[1][3] + Left[1][2]*Right[2][3] + Left[1][3]*Right[3][3];
	Result[2][0] = Left[2][0]*Right[0][0] + Left[2][1]*Right[1][0] + Left[2][2]*Right[2][0] + Left[2][3]*Right[3][0];
	Result[2][1] = Left[2][0]*Right[0][1] + Left[2][1]*Right[1][1] + Left[2][2]*Right[2][1] + Left[2][3]*Right[3][1];
	Result[2][2] = Left[2][0]*Right[0][2] + Left[2][1]*Right[1][2] + Left[2][2]*Right[2][2] + Left[2][3]*Right[3][2];
	Result[2][3] = Left[2][0]*Right[0][3] + Left[2][1]*Right[1][3] + Left[2][2]*Right[2][3] + Left[2][3]*Right[3][3];
	Result[3][0] = Left[3][0]*Right[0][0] + Left[3][1]*Right[1][0] + Left[3][2]*Right[2][0] + Left[3][3]*Right[3][0];
	Result[3][1] = Left[3][0]*Right[0][1] + Left[3][1]*Right[1][1] + Left[3][2]*Right[2][1] + Left[3][3]*Right[3][1];
	Result[3][2] = Left[3][0]*Right[0][2] + Left[3][1]*Right[1][2] + Left[3][2]*Right[2][2] + Left[3][3]*Right[3][2];
	Result[3][3] = Left[3][0]*Right[0][3] + Left[3][1]*Right[1][3] + Left[3][2]*Right[2][3] + Left[3][3]*Right[3][3];
}

__forceinline void CalculateWorldCoords(double lat, double lon, GLfloat result[])
{
	double lon_r = decToRad(lon);
	double lat_r = -decToRad(lat);

	double c = cos(lat_r);

	result[0] = (GLfloat)(cos(lon_r)*c);
	result[1] = (GLfloat)(sin(lon_r)*c);
	result[2] = (GLfloat)(sin(lat_r));
}

CString CookAttributeString(WCHAR* attr)
{
	CString tmpStr(attr);
	tmpStr.Replace(_T("<"), _T("_"));
	tmpStr.Replace(_T(">"), _T("_"));
	tmpStr.Replace(_T("&"), _T("&amp;"));
	tmpStr.Replace(_T("–"), _T("&#8211;"));
	tmpStr.Replace(_T("—"), _T("&#8212;"));

	return tmpStr;
}

void WriteGoogleAttribute(CStdioFile* f, LFItemDescriptor* i, UINT attr)
{
	WCHAR tmpStr[256];
	LFAttributeToString(i, attr, tmpStr, 256);

	if (tmpStr[0]!='\0')
	{
		f->WriteString(_T("&lt;b&gt;"));
		f->WriteString(CookAttributeString(theApp.m_Attributes[attr]->Name));
		f->WriteString(_T("&lt;/b&gt;: "));
		f->WriteString(CookAttributeString(tmpStr));
		f->WriteString(_T("&lt;br&gt;"));
	}
}

__forceinline BOOL SetupPixelFormat(HDC hDC)
{
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
		1,								// version number
		PFD_DRAW_TO_WINDOW |			// support window
		PFD_SUPPORT_OPENGL |			// support OpenGL
		PFD_DOUBLEBUFFER,				// double buffered
		PFD_TYPE_RGBA,					// RGBA type
		32,								// 32-bit color depth
		0, 0, 0, 0, 0, 0,				// color bits ignored
		0,								// no alpha buffer
		0,								// shift bit ignored
		0,								// no accumulation buffer
		0, 0, 0, 0,						// accum bits ignored
		0,								// no z-buffer
		0,								// no stencil buffer
		0,								// no auxiliary buffer
		PFD_MAIN_PLANE,					// main layer
		0,								// reserved
		0, 0, 0							// layer masks ignored
	};

	INT PixelFormat = ChoosePixelFormat(hDC, &pfd);
	return PixelFormat ? SetPixelFormat(hDC, PixelFormat, &pfd) : FALSE;
}

void glEnable2D()
{
	GLint iViewport[4];
	glGetIntegerv(GL_VIEWPORT, iViewport);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(iViewport[0], iViewport[0]+iViewport[2], iViewport[1]+iViewport[3], iViewport[1], -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.375, 0.375, 0);

	glPushAttrib(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
}

void glDisable2D()
{
	glPopAttrib();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void glDrawIcon(GLfloat x, GLfloat y, GLfloat Size, GLfloat Alpha, UINT ID)
{
	x -= 0.375;
	y -= 0.375;
	Size /= 2.0;

	GLfloat s = (ID%2) ? 0.5f : 0.0f;
	GLfloat t = (ID/2) ? 0.5f : 0.0f;

	glColor4f(1.0f, 1.0f, 1.0f, Alpha);

	glTexCoord2d(s, t);
	glVertex2d(x-Size, y-Size);
	glTexCoord2d(s+0.5, t);
	glVertex2d(x+Size, y-Size);
	glTexCoord2d(s+0.5, t+0.5);
	glVertex2d(x+Size, y+Size);
	glTexCoord2d(s, t+0.5);
	glVertex2d(x-Size, y+Size);
}


// CGlobeView
//

#define GetItemData(idx)     ((GlobeItemData*)(m_ItemData+idx*m_DataSize))


CGlobeView::CGlobeView()
	: CFileView(sizeof(GlobeItemData), FALSE, FALSE, TRUE, FALSE, FALSE)
{
	m_pDC = NULL;
	hRC = NULL;

	lpszCursorName = IDC_WAIT;
	hCursor = theApp.LoadStandardCursor(IDC_WAIT);
	m_CursorPos.x = m_CursorPos.y = 0;

	m_Width = m_Height = 0;
	m_GlobeModel = -1;
	m_pTextureGlobe = m_pTextureIcons = NULL;
	m_CurrentGlobeTexture = -1;
	m_Scale = 1.0f;
	m_Radius = m_Momentum = 0.0f;
	m_Grabbed = m_LockUpdate = FALSE;
	m_AnimCounter = m_MoveCounter = 0;

	ENSURE(m_YouLookAt.LoadString(IDS_YOULOOKAT));
}

BOOL CGlobeView::Create(CWnd* pParentWnd, UINT nID, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data)
{
	return CFileView::Create(pParentWnd, nID, pRawFiles, pCookedFiles, Data, CS_DBLCLKS | CS_OWNDC);
}

void CGlobeView::SetViewOptions(BOOL Force)
{
	if (Force)
	{
		m_GlobeCurrent.Latitude = m_GlobeTarget.Latitude = p_ViewParameters->GlobeLatitude/1000.0f;
		m_GlobeCurrent.Longitude = m_GlobeTarget.Longitude = p_ViewParameters->GlobeLongitude/1000.0f;
		m_GlobeCurrent.Zoom = m_GlobeTarget.Zoom = p_ViewParameters->GlobeZoom;
	}

	PrepareTexture();

	if (Force || (m_IsHQModel!=theApp.m_GlobeHQModel))
	{
		PrepareModel();
		m_IsHQModel = theApp.m_GlobeHQModel;
	}
}

void CGlobeView::SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data)
{
	CFileView::SetSearchResult(pRawFiles, pCookedFiles, Data);

	if (p_CookedFiles)
		if (p_CookedFiles->m_ItemCount)
			for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
			{
				LFGeoCoordinates coord = { 0.0, 0.0 };
				if (m_ViewParameters.SortBy==LFAttrLocationIATA)
				{
					LFAirport* airport;
					if (LFIATAGetAirportByCode((CHAR*)p_CookedFiles->m_Items[a]->AttributeValues[LFAttrLocationIATA], &airport))
						coord = airport->Location;
				}
				else
					if (p_CookedFiles->m_Items[a]->AttributeValues[m_ViewParameters.SortBy])
					{
						ASSERT(theApp.m_Attributes[m_ViewParameters.SortBy]->Type==LFTypeGeoCoordinates);
						coord = *((LFGeoCoordinates*)p_CookedFiles->m_Items[a]->AttributeValues[m_ViewParameters.SortBy]);
					}

				if ((coord.Latitude!=0.0) || (coord.Longitude!=0))
				{
					GlobeItemData* d = GetItemData(a);
					CalculateWorldCoords(coord.Latitude, coord.Longitude, d->World);
					LFGeoCoordinatesToString(coord, d->CoordString, 32, false);

					d->Hdr.Valid = TRUE;
				}
			}

	if (Data)
		if (Data->LocationValid)
			m_GlobeCurrent = Data->Location;
}

INT CGlobeView::ItemAtPosition(CPoint point)
{
	if (!p_CookedFiles)
		return -1;

	INT res = -1;
	GLfloat Alpha = 0.0f;

	for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
	{
		GlobeItemData* d = GetItemData(a);

		if (d->Hdr.Valid)
			if ((d->Alpha>0.75f) || ((d->Alpha>0.1f) && (d->Alpha>Alpha-0.05f)))
				if (PtInRect(&d->Hdr.Rect, point))
				{
					res = a;
					Alpha = d->Alpha;
				}
	}

	return res;
}

CMenu* CGlobeView::GetBackgroundContextMenu()
{
	CMenu* pMenu = new CMenu();
	pMenu->LoadMenu(IDM_GLOBE);
	return pMenu;
}

CMenu* CGlobeView::GetItemContextMenu(INT idx)
{
	CMenu* pMenu = CFileView::GetItemContextMenu(idx);
	
	CMenu* pPopup = pMenu->GetSubMenu(0);
	ASSERT_VALID(pPopup);

	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_OPENGOOGLEEARTH));
	pPopup->InsertMenu(1, MF_STRING | MF_BYPOSITION, IDM_GLOBE_GOOGLEEARTH, tmpStr);

	return pMenu;
}

void CGlobeView::GetPersistentData(FVPersistentData& Data)
{
	CFileView::GetPersistentData(Data);

	Data.Location = m_GlobeCurrent;
	Data.LocationValid = TRUE;
}

BOOL CGlobeView::CursorOnGlobe(CPoint point)
{
	GLfloat distX = point.x-(GLfloat)m_Width/2;
	GLfloat distY = point.y-(GLfloat)m_Height/2;

	return distX*distX+distY*distY<m_Radius*m_Radius;
}

void CGlobeView::UpdateCursor()
{
	LPCTSTR csr;
	if (m_Grabbed)
	{
		csr = IDC_HAND;
	}
	else
	{
		csr = IDC_ARROW;

		if (CursorOnGlobe(m_CursorPos))
			if (ItemAtPosition(m_CursorPos)==-1)
				csr = IDC_HAND;
	}

	if (csr!=lpszCursorName)
	{
		hCursor = theApp.LoadStandardCursor(csr);

		SetCursor(hCursor);
		lpszCursorName = csr;
	}
}


// OpenGL
//

__forceinline void CGlobeView::PrepareModel()
{
	// 3D-Modelle einbinden
	#include "Globe_Low.h"
	#include "Globe_High.h"
	UINT Count = (theApp.m_GlobeHQModel ? GlobeHighCount : GlobeLowCount);
	GLfloat* Nodes = (theApp.m_GlobeHQModel ? &GlobeHighNodes[0] : &GlobeLowNodes[0]);

	// Display-Liste für das 3D-Modell erstellen
	m_LockUpdate = TRUE;
	wglMakeCurrent(*m_pDC, hRC);

	if (m_GlobeModel==-1)
		m_GlobeModel = glGenLists(1);

	glNewList(m_GlobeModel, GL_COMPILE);
	glEnable(GL_CULL_FACE);
	glBegin(GL_TRIANGLES);

	UINT Pos = 0;
	for (UINT a=0; a<Count; a++)
	{
		GLfloat s = Nodes[Pos++];
		GLfloat t = Nodes[Pos++];
		glTexCoord2f(s, t);

		GLfloat x = Nodes[Pos++];
		GLfloat y = Nodes[Pos++];
		GLfloat z = Nodes[Pos++];
		glNormal3f(x, y, z);
		glVertex3f(x, y, z);
	}

	glEnd();
	glDisable(GL_CULL_FACE);
	glEndList();

	m_LockUpdate = FALSE;
}

__forceinline void CGlobeView::PrepareTexture()
{
	// Automatisch höchstens 4096x4096 laden, da quadratisch und von den meisten Grafikkarten unterstützt
	UINT Tex = theApp.m_nTextureSize;
	if (Tex==LFTextureAuto)
		Tex = LFTexture4096;

	// Texture prüfen
	GLint TexSize = 1024;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &TexSize);

Smaller:
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, 4, TexSize, TexSize, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
	GLint ProxySize = 0;
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &ProxySize);

	if ((ProxySize==0) && (TexSize>1024))
	{
		TexSize /= 2;
		goto Smaller;
	}

	theApp.m_nMaxTextureSize = (TexSize>=8192) ? LFTexture8192 : (TexSize>=4096) ? LFTexture4096 : (TexSize>=2048) ? LFTexture2048 : LFTexture1024;
	if (Tex>theApp.m_nMaxTextureSize)
		Tex = theApp.m_nMaxTextureSize;

	if ((INT)Tex!=m_CurrentGlobeTexture)
	{
		SetCursor(theApp.LoadStandardCursor(IDC_WAIT));
		m_LockUpdate = TRUE;

		wglMakeCurrent(*m_pDC, hRC);

		if (m_pTextureGlobe)
			delete m_pTextureGlobe;
		m_pTextureGlobe = new GLTextureBlueMarble(Tex);
		m_CurrentGlobeTexture = Tex;

		m_LockUpdate = FALSE;
		SetCursor(hCursor);

		Invalidate();
	}
}

__forceinline void CGlobeView::Normalize()
{
	// Zoom
	if (m_GlobeTarget.Zoom<0)
		m_GlobeTarget.Zoom = 0;
	if (m_GlobeTarget.Zoom>1000)
		m_GlobeTarget.Zoom = 1000;

	// Nicht über die Pole rollen
	if (m_GlobeTarget.Latitude<-75.0f)
		m_GlobeTarget.Latitude = -75.0f;
	if (m_GlobeTarget.Latitude>75.0f)
		m_GlobeTarget.Latitude = 75.0f;

	// Rotation normieren
	if (m_GlobeTarget.Longitude<0.0f)
		m_GlobeTarget.Longitude += 360.0f;
	if (m_GlobeTarget.Longitude>360.0f)
		m_GlobeTarget.Longitude -= 360.0f;
}

__forceinline void CGlobeView::CalcAndDrawSpots(GLfloat ModelView[4][4], GLfloat Projection[4][4])
{
	GLfloat SizeX = m_Width/2.0f;
	GLfloat SizeY = m_Height/2.0f;

	GLfloat MVP[4][4];
	MatrixMul(MVP, ModelView, Projection);

	for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
	{
		GlobeItemData* d = GetItemData(a);
		if (d->Hdr.Valid)
		{
			d->Alpha = 0.0f;

			GLfloat z = ModelView[0][2]*d->World[0] + ModelView[1][2]*d->World[1] + ModelView[2][2]*d->World[2];
			if ((z>m_FogEnd) && (m_Width) && (m_Height))
			{
				GLfloat w = MVP[0][3]*d->World[0] + MVP[1][3]*d->World[1] + MVP[2][3]*d->World[2] + MVP[3][3];
				GLfloat x = (MVP[0][0]*d->World[0] + MVP[1][0]*d->World[1] + MVP[2][0]*d->World[2] + MVP[3][0])*SizeX/w + SizeX + 0.5f;
				GLfloat y = -(MVP[0][1]*d->World[0] + MVP[1][1]*d->World[1] + MVP[2][1]*d->World[2] + MVP[3][1])*SizeY/w + SizeY + 0.5f;

				d->ScreenPoint[0] = (INT)x;
				d->ScreenPoint[1] = (INT)y;
				d->Alpha = 1.0f;
				if (z<m_FogStart)
					d->Alpha -= (GLfloat)((m_FogStart-z)/(m_FogStart-m_FogEnd));

				if (m_ViewParameters.GlobeShowSpots)
					glDrawIcon(x, y, 6.0f+8.0f*d->Alpha, d->Alpha, SPOT);
			}
		}
	}
}

__forceinline void CGlobeView::CalcAndDrawLabel(BOOL Themed)
{
	for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
	{
		GlobeItemData* d = GetItemData(a);

		if (d->Hdr.Valid)
			if (d->Alpha>0.0f)
			{
				// Beschriftung
				WCHAR* Caption = p_CookedFiles->m_Items[a]->CoreAttributes.FileName;
				UINT cCaption = (UINT)wcslen(Caption);
				WCHAR* Subcaption = NULL;
				WCHAR* Coordinates = (m_ViewParameters.GlobeShowGPS ? d->CoordString : NULL);
				WCHAR* Description = (m_ViewParameters.GlobeShowDescription ? p_CookedFiles->m_Items[a]->Description : NULL);
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
							Subcaption = &Caption[6];
						cCaption = 3;
					}
					break;
				case LFAttrLocationGPS:
					if ((wcscmp(Caption, d->CoordString)==0) && (m_ViewParameters.GlobeShowGPS))
						Coordinates = NULL;
					break;
				}

				DrawLabel(d, cCaption, Caption, Subcaption, Coordinates, Description, m_FocusItem==(INT)a, Themed);
			}
	}
}

__forceinline void CGlobeView::DrawLabel(GlobeItemData* d, UINT cCaption, WCHAR* Caption, WCHAR* Subcaption, WCHAR* Coordinates, WCHAR* Description, BOOL Focused, BOOL Themed)
{
	ASSERT(ARROWSIZE>3);

	// Breite
	UINT W1 = m_Fonts[1].GetTextWidth(Caption, cCaption);
	UINT W2 = m_Fonts[0].GetTextWidth(Subcaption);
	UINT W3 = m_Fonts[0].GetTextWidth(Coordinates);
	UINT W4 = m_Fonts[0].GetTextWidth(Description);
	UINT Width = max(W1, max(W2, max(W3, W4)))+8;

	// Höhe
	UINT Height = 3;
	Height += m_Fonts[1].GetTextHeight(Caption);
	Height += m_Fonts[0].GetTextHeight(Subcaption);
	Height += m_Fonts[0].GetTextHeight(Coordinates);
	Height += m_Fonts[0].GetTextHeight(Description);

	// Position
	INT top = (d->ScreenPoint[1]<m_Height/2) ? -1 : 1;

	INT x = d->Hdr.Rect.left = d->ScreenPoint[0]-ARROWSIZE-(((INT)Width-2*ARROWSIZE)*(m_Width-d->ScreenPoint[0])/m_Width);
	INT y = d->Hdr.Rect.top = d->ScreenPoint[1]+(ARROWSIZE-2)*top-(top<0 ? (INT)Height : 0);
	d->Hdr.Rect.right = x+Width;
	d->Hdr.Rect.bottom = y+Height;

	// Sichtbar?
	if ((x+Width+6<0) || (x-1>m_Width) || (y+Height+ARROWSIZE+6<0) || (y-ARROWSIZE-6>m_Height))
	{
		d->Alpha = 0.0f;
		return;
	}

	// Farben
	COLORREF BaseColorRef = GetSysColor(COLOR_WINDOW);
	COLORREF TextColorRef = GetSysColor(COLOR_WINDOWTEXT);
	COLORREF BorderColorRef = (!theApp.m_GlobeBlackBackground && Themed) ? 0xE0CDC4 : GetSysColor(COLOR_3DSHADOW);
	if (d->Hdr.Selected)
		if (this==GetFocus())
		{
			BaseColorRef = GetSysColor(COLOR_HIGHLIGHT);
			TextColorRef = GetSysColor(COLOR_HIGHLIGHTTEXT);
		}
		else
		{
			BaseColorRef = GetSysColor(COLOR_BTNFACE);
			TextColorRef = GetSysColor(COLOR_BTNTEXT);
		}

	GLfloat BaseColor[4];
	ColorRef2GLColor(&BaseColor[0], BaseColorRef);
	GLfloat TextColor[4];
	ColorRef2GLColor(&TextColor[0], TextColorRef);
	GLfloat BorderColor[4];
	ColorRef2GLColor(&BorderColor[0], BorderColorRef);

	// Schatten
	if (Themed)
	{
		glColor4f(0.0f, 0.0f, 0.0f, d->Alpha*(18.0f/256.0f));
		glBegin(GL_LINES);
		glVertex2i(x+1, y+Height+1);
		glVertex2i(x+Width, y+Height+1);
		glVertex2i(x+Width+1, y+1);
		glVertex2i(x+Width+1, y+Height);
		glVertex2i(x+Width, y+Height);
		glVertex2i(x+Width+1, y+Height);
		glEnd();
	}

	// Innen
	glColor4f(BaseColor[0], BaseColor[1], BaseColor[2], d->Alpha);
	glRecti(x, y, x+(INT)Width, y+(INT)Height);

	glBegin(GL_TRIANGLES);
	glVertex2i(d->ScreenPoint[0], d->ScreenPoint[1]);
	glVertex2i(d->ScreenPoint[0]+(ARROWSIZE-2), d->ScreenPoint[1]+(ARROWSIZE-2)*top);
	glVertex2i(d->ScreenPoint[0]-(ARROWSIZE-2), d->ScreenPoint[1]+(ARROWSIZE-2)*top);
	glEnd();

	// Rand
	glBegin(GL_LINES);
	glColor4f(BorderColor[0], BorderColor[1], BorderColor[2], d->Alpha);
	glVertex2i(x-1, y+1);					// Links
	glVertex2i(x-1, y+Height-1);
	glVertex2i(x+Width, y+1);				// Rechts
	glVertex2i(x+Width, y+Height-1);
	if (top>0)
	{
		glVertex2i(x+1, y+Height);			// Unten
		glVertex2i(x+Width-1, y+Height);

		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex2i(x+1, y-1);
		glVertex2i(d->ScreenPoint[0]-(ARROWSIZE-2), y-1);
		glVertex2i(d->ScreenPoint[0], d->ScreenPoint[1]-top);
		glVertex2i(d->ScreenPoint[0]+(ARROWSIZE-2), y-1);
		glVertex2i(x+Width-1, y-1);
	}
	else
	{
		glVertex2i(x+1, y-1);				// Oben
		glVertex2i(x+Width-1, y-1);

		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex2i(x+1, y+Height);
		glVertex2i(d->ScreenPoint[0]-(ARROWSIZE-2), y+Height);
		glVertex2i(d->ScreenPoint[0], d->ScreenPoint[1]);
		glVertex2i(d->ScreenPoint[0]+(ARROWSIZE-2), y+Height);
		glVertex2i(x+Width-1, y+Height);
	}
	glEnd();

	glColor4f(BorderColor[0], BorderColor[1], BorderColor[2], d->Alpha*0.5f);
	glBegin(GL_LINE_STRIP);
	glVertex2i(x-1, y);						// Oben links
	glVertex2i(x, y);
	glVertex2i(x, y-2);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex2i(x-1, y+Height-1);			// Unten links
	glVertex2i(x, y+Height-1);
	glVertex2i(x, y+Height+1);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex2i(x+Width, y);					// Oben rechts
	glVertex2i(x+Width-1, y);
	glVertex2i(x+Width-1, y-2);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex2i(x+Width, y+Height-1);		// Unten rechts
	glVertex2i(x+Width-1, y+Height-1);
	glVertex2i(x+Width-1, y+Height+1);
	glEnd();

	// Focus
	if ((Focused) && (GetFocus()==this))
	{
		glColor4f(1.0f-BaseColor[0], 1.0f-BaseColor[1], 1.0f-BaseColor[2], d->Alpha);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xAAAA);
		glBegin(GL_LINE_LOOP);
		glVertex2i(x, y);
		glVertex2i(x+Width-1, y);
		glVertex2i(x+Width-1, y+Height-1);
		glVertex2i(x, y+Height-1);
		glEnd();
		glDisable(GL_LINE_STIPPLE);
	}

	x += 3;

	glColor4f(TextColor[0], TextColor[1], TextColor[2], d->Alpha);
	y += m_Fonts[1].Render(Caption, x, y, cCaption);
	if (Subcaption)
		y += m_Fonts[0].Render(Subcaption, x, y);

	if ((!d->Hdr.Selected) && (BaseColorRef==0xFFFFFF) && (TextColorRef==0x000000))
		glColor4f(TextColor[0], TextColor[1], TextColor[2], d->Alpha/2);
	if (Coordinates)
		y += m_Fonts[0].Render(Coordinates, x, y);
	if (Description)
		y += m_Fonts[0].Render(Description, x, y);
}

__forceinline void CGlobeView::DrawStatusBar(INT Height, COLORREF BarColor, BOOL Themed)
{
	WCHAR Copyright[] = L"© NASA's Earth Observatory";
	INT CopyrightWidth = (INT)m_Fonts[0].GetTextWidth(Copyright);
	if (m_Width<CopyrightWidth)
		return;

	WCHAR Viewpoint[256] = L"";
	INT ViewpointWidth = -1;
	if (theApp.m_GlobeShowViewport)
	{
		WCHAR Coord[256];
		LFGeoCoordinates c;
		c.Latitude = -m_GlobeCurrent.Latitude;
		c.Longitude = (m_GlobeCurrent.Longitude>180.0) ? 360-m_GlobeCurrent.Longitude : -m_GlobeCurrent.Longitude;
		LFGeoCoordinatesToString(c, Coord, 256, true);

		swprintf(Viewpoint, 256, m_YouLookAt, Coord);

		ViewpointWidth = (INT)m_Fonts[0].GetTextWidth(Viewpoint);
		if (m_Width<CopyrightWidth+ViewpointWidth+48)
			ViewpointWidth = -1;
	}

	// Kante
	GLfloat BackColor[4];
	ColorRef2GLColor(BackColor, BarColor);
	glColor4f(BackColor[0], BackColor[1], BackColor[2], theApp.m_GlobeBlackBackground ? 0.6f : 0.9f);
	glBegin(GL_LINES);
	glVertex2i(0, m_Height-Height);
	glVertex2i(m_Width, m_Height-Height);
	glEnd();

	// Füllen
	glColor4f(BackColor[0], BackColor[1], BackColor[2], theApp.m_GlobeBlackBackground ? 0.55f : 0.8f);
	glRecti(0, m_Height-Height, m_Width, m_Height);

	// Text
	GLfloat TextColor[4];
	ColorRef2GLColor(TextColor, theApp.m_GlobeBlackBackground ? 0xFFFFFF : Themed ? 0xCC6600 : GetSysColor(COLOR_WINDOWTEXT));
	glColor4f(TextColor[0], TextColor[1], TextColor[2], 1.0f);

	INT Gutter = (ViewpointWidth>0) ? (m_Width-CopyrightWidth-ViewpointWidth)/3 : (m_Width-CopyrightWidth)/2;

	m_Fonts[0].Render(Copyright, Gutter, m_Height-Height);
	if (ViewpointWidth>0)
		m_Fonts[0].Render(Viewpoint, m_Width-ViewpointWidth-Gutter, m_Height-Height);
}

void CGlobeView::DrawScene(BOOL InternalCall)
{
	if (!InternalCall)
	{
		if (m_LockUpdate)
			return;
		m_LockUpdate = TRUE;
	}

	BOOL Themed = IsCtrlThemed();

	wglMakeCurrent(*m_pDC, hRC);
	glRenderMode(GL_RENDER);

	// Hintergrund
	GLfloat BackColor[4];
	ColorRef2GLColor(BackColor, theApp.m_GlobeBlackBackground ? 0x000000 : Themed ? 0xFDF7F4 : GetSysColor(COLOR_WINDOW));
	glFogfv(GL_FOG_COLOR, BackColor);

	glClearColor(BackColor[0], BackColor[1], BackColor[2], 1.0f);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Weißer Farbverlauf
	if (!theApp.m_GlobeBlackBackground && Themed)
	{
		glEnable2D();
		glBegin(GL_QUADS);

		glColor3f(BackColor[0], BackColor[1], BackColor[2]);
		glVertex2i(0, WHITE);
		glVertex2i(m_Width, WHITE);

		glColor3f(1.0, 1.0, 1.0);
		glVertex2i(m_Width, 0);
		glVertex2i(0, 0);

		glEnd();
		glDisable2D();
	}

	// Globus berechnen
	m_Scale = 1.0f;
	if (m_Height>m_Width)
		m_Scale = 1-((GLfloat)(m_Height-m_Width))/m_Height;

	GLfloat zoomfactor = ((m_GlobeCurrent.Zoom+400)/1000.0f);
	m_Scale /= zoomfactor*zoomfactor;
	m_Radius = 0.49f*m_Height*m_Scale;
	m_FogStart = 0.40f*m_Scale;
	m_FogEnd = 0.025f*m_Scale;

	// Globus zeichnen
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(DISTANCE, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);

	// Beleuchtung mit FESTER Lichtquelle
	if (theApp.m_GlobeLighting)
	{
		GLfloat lAmbient[] = { 0.9f, 0.9f, 0.9f, 1.0f };
		GLfloat lDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat lSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glLightfv(GL_LIGHT0, GL_AMBIENT, lAmbient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lDiffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, lSpecular);

		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHTING);
		glEnable(GL_NORMALIZE);

		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lAmbient);
	}

	// Rotationsmatrix (erst NACH Lichtquelle)
	glRotatef(m_GlobeCurrent.Latitude, 0.0f, 1.0f, 0.0f);
	glRotatef(m_GlobeCurrent.Longitude, 0.0f, 0.0f, 1.0f);
	glScalef(m_Scale, m_Scale, m_Scale);

	// Atmosphäre/Nebel
	if (theApp.m_GlobeAtmosphere)
	{
		glEnable(GL_FOG);
		glFogf(GL_FOG_START, DISTANCE-m_FogStart);
		glFogf(GL_FOG_END, DISTANCE-m_FogEnd);
	}

	// Globus-Textur
	if (m_pTextureGlobe)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_pTextureGlobe->GetID());
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, theApp.m_GlobeLighting ? GL_MODULATE : GL_REPLACE);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}

	// Modell rendern
	glCallList(m_GlobeModel);

	// Atmosphäre aus
	if (theApp.m_GlobeAtmosphere)
		glDisable(GL_FOG);

	// Licht aus
	if (theApp.m_GlobeLighting)
	{
		glDisable(GL_NORMALIZE);
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	}

	// Matritzen speichern
	GLfloat ModelView[4][4];
	GLfloat Projection[4][4];
	glGetFloatv(GL_MODELVIEW_MATRIX, &ModelView[0][0]);
	glGetFloatv(GL_PROJECTION_MATRIX, &Projection[0][0]);

	// Für Icons vorbereiten
	if (m_pTextureIcons)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_pTextureIcons->GetID());
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}
	glEnable2D();
	glBegin(GL_QUADS);

	// Koordinaten bestimmen und Spots zeichnen
	if (p_CookedFiles && !m_Nothing)
		if (p_CookedFiles->m_ItemCount)
			CalcAndDrawSpots(ModelView, Projection);

	// Fadenkreuz zeichnen
	if (theApp.m_GlobeShowViewport && theApp.m_GlobeShowCrosshairs)
		glDrawIcon((GLfloat)(m_Width/2), (GLfloat)(m_Height/2), 64.0f, 1.0f, CROSSHAIRS);

	// Icons beenden
	glEnd();
	glDisable(GL_TEXTURE_2D);

	// Label zeichnen
	if (p_CookedFiles && !m_Nothing)
		if (p_CookedFiles->m_ItemCount)
			CalcAndDrawLabel(Themed);

	// Statuszeile
	const INT Height = m_FontHeight[0]+1;
	if (m_Height>=Height)
		DrawStatusBar(Height, theApp.m_GlobeBlackBackground ? 0x000000 : 0xFFFFFF, Themed);

	// Beenden
	glDisable2D();

	SwapBuffers(*m_pDC);
	m_LockUpdate = FALSE;
}

BOOL CGlobeView::UpdateScene(BOOL Redraw)
{
	if (m_LockUpdate)
		return FALSE;
	m_LockUpdate = TRUE;

	BOOL res = Redraw;
	Normalize();

	// Zoom
	if (m_GlobeCurrent.Zoom<=m_GlobeTarget.Zoom-5)
	{
		res = TRUE;
		m_GlobeCurrent.Zoom += 5;
		m_HotItem = -1;
	}
	else
		if (m_GlobeCurrent.Zoom>=m_GlobeTarget.Zoom+5)
		{
			res = TRUE;
			m_GlobeCurrent.Zoom -= 5;
			m_HotItem = -1;
		}
		else
		{
			res |= (m_GlobeCurrent.Zoom!=m_GlobeTarget.Zoom);
			m_GlobeCurrent.Zoom = m_GlobeTarget.Zoom;
		}

	// Animation
	if (m_AnimCounter)
	{
		GLfloat f = (GLfloat)((cos(PI*m_AnimCounter/ANIMLENGTH)+1.0)/2.0);
		m_GlobeCurrent.Latitude = m_AnimStartLatitude*(1.0f-f) + m_GlobeTarget.Latitude*f;
		m_GlobeCurrent.Longitude = m_AnimStartLongitude*(1.0f-f) + m_GlobeTarget.Longitude*f;

		if (m_GlobeTarget.Zoom<600)
		{
			INT Dist = 600-m_GlobeTarget.Zoom;
			INT MaxDist = (INT)((m_GlobeTarget.Zoom+100)*1.2f);
			if (Dist>MaxDist)
				Dist = MaxDist;

			GLfloat f = (GLfloat)sin(PI*m_AnimCounter/ANIMLENGTH);
			m_GlobeCurrent.Zoom = (INT)(m_GlobeTarget.Zoom*(1.0f-f)+(m_GlobeTarget.Zoom+Dist)*f);
		}

		res = TRUE;
		m_AnimCounter--;
	}
	else
	{
		if (m_Momentum!=0.0f)
			m_GlobeTarget.Longitude += m_Momentum;

		res |= (m_GlobeCurrent.Latitude!=m_GlobeTarget.Latitude) || (m_GlobeCurrent.Longitude!=m_GlobeTarget.Longitude);
		m_GlobeCurrent.Latitude = m_GlobeTarget.Latitude;
		m_GlobeCurrent.Longitude = m_GlobeTarget.Longitude;
	}

	if (res)
	{
		DrawScene(TRUE);
		UpdateCursor();
	}
	else
	{
		m_LockUpdate = FALSE;
	}

	return res;
}


BEGIN_MESSAGE_MAP(CGlobeView, CFileView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SIZE()
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

	m_pDC = new CClientDC(this);
	if (!m_pDC)
		return -1;

	if (!SetupPixelFormat(*m_pDC))
		return -1;

	hRC = wglCreateContext(*m_pDC);
	wglMakeCurrent(*m_pDC, hRC);

	// 3D-Einstellungen
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_SMOOTH);
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glFogi(GL_FOG_MODE, GL_LINEAR);
	glHint(GL_FOG_HINT, GL_NICEST);

	// Fonts
	m_Fonts[0].Create(&theApp.m_DefaultFont);
	m_Fonts[1].Create(&theApp.m_LargeFont);

	// Icons
	CGdiPlusBitmapResource Tex0(IDB_GLOBEICONS_RGB, _T("PNG"));
	CGdiPlusBitmapResource Tex1(IDB_GLOBEICONS_ALPHA, _T("PNG"));
	m_pTextureIcons = new GLTextureCombine(&Tex0, &Tex1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// Animations-Timer
	SetTimer(1, 10, NULL);

	return 0;
}

void CGlobeView::OnDestroy()
{
	KillTimer(1);

	if (m_pDC)
	{
		wglMakeCurrent(*m_pDC, hRC);

		if (m_pTextureGlobe)
			delete m_pTextureGlobe;
		if (m_pTextureIcons)
			delete m_pTextureIcons;
		if (m_GlobeModel!=-1)
			glDeleteLists(m_GlobeModel, 1);

		wglMakeCurrent(NULL, NULL);
		if (hRC)
			wglDeleteContext(hRC);
		delete m_pDC;
	}

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
	DrawScene();
}

void CGlobeView::OnSize(UINT nType, INT cx, INT cy)
{
	if (cy>0)
	{
		m_Width = cx;
		m_Height = cy;

		wglMakeCurrent(*m_pDC, hRC);
		glViewport(0, 0, cx, cy);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(3.0f, (GLfloat)cx/cy, 0.1f, 500.0f);
	}

	CFileView::OnSize(nType, cx, cy);
}

BOOL CGlobeView::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
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

		CSize rotate = m_GrabPoint - point;
		m_GrabPoint = point;

		m_LastMove = -rotate.cx/m_Scale*0.12f;
		m_GlobeTarget.Longitude = m_GlobeCurrent.Longitude += m_LastMove;
		m_GlobeTarget.Latitude = m_GlobeCurrent.Latitude -= rotate.cy/m_Scale*0.12f;

		UpdateScene(TRUE);
	}
	else
	{
		UpdateCursor();
	}

	CFileView::OnMouseMove(nFlags, point);
}

BOOL CGlobeView::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
{
	m_TooltipCtrl.Deactivate();

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
		tme.dwHoverTime = LFHOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
}

void CGlobeView::OnLButtonDown(UINT nFlags, CPoint point)
{
	INT idx = ItemAtPosition(point);
	if (idx==-1)
	{
		if (CursorOnGlobe(point))
		{
			m_GrabPoint = point;
			m_Grabbed = TRUE;
			m_Momentum = m_LastMove = 0.0f;

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
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			OnZoomIn();
		break;
	case VK_SUBTRACT:
	case VK_OEM_MINUS:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			OnZoomOut();
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
	LFSelectLocationIATADlg dlg(IDD_JUMPTOIATA, this);

	if (dlg.DoModal()==IDOK)
	{
		ASSERT(dlg.p_Airport);

		m_AnimCounter = ANIMLENGTH;
		m_AnimStartLatitude = m_GlobeCurrent.Latitude;
		m_AnimStartLongitude = m_GlobeCurrent.Longitude;
		m_GlobeTarget.Latitude = (GLfloat)-dlg.p_Airport->Location.Latitude;
		m_GlobeTarget.Longitude = (GLfloat)-dlg.p_Airport->Location.Longitude;
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
	GlobeOptionsDlg dlg(this, p_ViewParameters, m_Context);
	if (dlg.DoModal()==IDOK)
		theApp.UpdateViewOptions(-1, LFViewGlobe);
}

void CGlobeView::OnGoogleEarth()
{
	// Dateinamen finden
	TCHAR Pathname[MAX_PATH];
	if (!GetTempPath(MAX_PATH, Pathname))
		_tcscpy_s(Pathname, MAX_PATH, theApp.m_Path);

	CString szTempName;
	srand(rand());
	szTempName.Format(_T("%sliquidFOLDERS%.4X%.4X.kml"), Pathname, 32768+rand(), 32768+rand());

	// Datei erzeugen
	CStdioFile f;
	if (!f.Open(szTempName, CFile::modeCreate | CFile::modeWrite))
	{
		LFErrorBox(LFDriveNotReady);
	}
	else
	{
		try
		{
			f.WriteString(_T("<?xml version=\"1.0\"?>\n<kml xmlns=\"http://earth.google.com/kml/2.0\">\n<Document>\n"));
			f.WriteString(_T("<Style id=\"A\"><IconStyle><scale>0.8</scale><Icon><href>http://maps.google.com/mapfiles/kml/pal4/icon57.png</href></Icon></IconStyle><LabelStyle><scale>0</scale></LabelStyle></Style>\n"));
			f.WriteString(_T("<Style id=\"B\"><IconStyle><scale>1.0</scale><Icon><href>http://maps.google.com/mapfiles/kml/pal4/icon57.png</href></Icon></IconStyle><LabelStyle><scale>1</scale></LabelStyle></Style>\n"));
			f.WriteString(_T("<StyleMap id=\"C\"><Pair><key>normal</key><styleUrl>#A</styleUrl></Pair><Pair><key>highlight</key><styleUrl>#B</styleUrl></Pair></StyleMap>\n"));

			INT i = GetNextSelectedItem(-1);
			while (i>-1)
			{
				LFGeoCoordinates c = p_CookedFiles->m_Items[i]->CoreAttributes.LocationGPS;
				if ((c.Latitude!=0) || (c.Longitude!=0))
				{
					f.WriteString(_T("<Placemark>\n<name>"));
					f.WriteString(CookAttributeString(p_CookedFiles->m_Items[i]->CoreAttributes.FileName));
					f.WriteString(_T("</name>\n<description>"));
					WriteGoogleAttribute(&f, p_CookedFiles->m_Items[i], LFAttrLocationName);
					WriteGoogleAttribute(&f, p_CookedFiles->m_Items[i], LFAttrLocationIATA);
					WriteGoogleAttribute(&f, p_CookedFiles->m_Items[i], LFAttrLocationGPS);
					WriteGoogleAttribute(&f, p_CookedFiles->m_Items[i], LFAttrArtist);
					WriteGoogleAttribute(&f, p_CookedFiles->m_Items[i], LFAttrRoll);
					WriteGoogleAttribute(&f, p_CookedFiles->m_Items[i], LFAttrRecordingTime);
					WriteGoogleAttribute(&f, p_CookedFiles->m_Items[i], LFAttrComments);
					f.WriteString(_T("&lt;div&gt;</description>\n"));

					f.WriteString(_T("<styleUrl>#C</styleUrl>\n"));
					CString tmpStr;
					tmpStr.Format(_T("<Point><coordinates>%.6lf,%.6lf,-5000</coordinates></Point>\n"), c.Longitude, -c.Latitude);
					f.WriteString(tmpStr);
					f.WriteString(_T("</Placemark>\n"));
				}

				i = GetNextSelectedItem(i);
			}

			f.WriteString(_T("</Document>\n</kml>\n"));
			f.Close();

			ShellExecute(m_hWnd, _T("open"), szTempName, NULL, NULL, SW_SHOW);
		}
		catch(CFileException ex)
		{
			LFErrorBox(LFDriveNotReady);
			f.Close();
		}
	}
}

void CGlobeView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;
	switch (pCmdUI->m_nID)
	{
	case IDM_GLOBE_ZOOMIN:
		b = m_GlobeTarget.Zoom>0;
		break;
	case IDM_GLOBE_ZOOMOUT:
		b = m_GlobeTarget.Zoom<1000;
		break;
	case IDM_GLOBE_AUTOSIZE:
		b = m_GlobeTarget.Zoom!=600;
		break;
	case IDM_GLOBE_GOOGLEEARTH:
		b = (GetNextSelectedItem(-1)!=-1) && (!theApp.m_PathGoogleEarth.IsEmpty());
		break;
	}

	pCmdUI->Enable(b);
}
