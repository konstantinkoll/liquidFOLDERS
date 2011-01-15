
// CGlobeView.cpp: Implementierung der Klasse CGlobeView
//

#include "stdafx.h"
#include "CGlobeView.h"
#include "Resource.h"
#include "LFCore.h"
#include "GlobeOptionsDlg.h"
#include <math.h>

#define DISTANCE      39.0f
#define ARROWSIZE     9
#define PI            3.14159265358979323846
#define ANIMLENGTH    200
#define SPOT          2
#define CROSSHAIRS    3

void ColorRef2GLColor(GLfloat* dst, COLORREF src, GLfloat Alpha=1.0f)
{
	dst[0] = (src&0xFF)/255.0f;
	dst[1] = ((src>>8)&0xFF)/255.0f;
	dst[2] = ((src>>16)&0xFF)/255.0f;
	dst[3] = Alpha;
}

inline double decToRad(double dec)
{
	return dec*(PI/180.0);
}

inline void MatrixMul(GLdouble Result[4][4], GLdouble Left[4][4], GLdouble Right[4][4])
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

void CalculateWorldCoords(double lat, double lon, double result[])
{
	double lon_r = decToRad(lon);
	double lat_r = -decToRad(lat);

	result[0] = cos(lat_r)*cos(lon_r);
	result[1] = cos(lat_r)*sin(lon_r);
	result[2] = sin(lat_r);
}

CString CookAttributeString(WCHAR* attr)
{
	CString tmpStr(attr);
	tmpStr.Replace(_T("<"), _T("_"));
	tmpStr.Replace(_T(">"), _T("_"));
	tmpStr.Replace(_T("&"), _T("&amp;"));

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


// CGlobeView
//

#define GetItemData(idx)     ((GlobeItemData*)(m_ItemData+idx*m_DataSize))


CGlobeView::CGlobeView()
	: CFileView(sizeof(GlobeItemData), FALSE, FALSE, TRUE, FALSE, FALSE)
{
	m_pDC = NULL;
	m_hRC = NULL;
	lpszCursorName = IDC_WAIT;
	hCursor = theApp.LoadStandardCursor(IDC_WAIT);

	m_Width = m_Height = 0;
	m_GlobeModel = -1;
	m_TextureGlobe = m_TextureIcons = NULL;
	m_nTexture = -1;
	m_Latitude = m_Longitude = 0.0f;
	m_Zoom = -1.0f;
	m_Scale = 1.0f;
	m_Radius = 0.0f;
	m_Grabbed = FALSE;
	m_CursorPos.x = m_CursorPos.y = 0;
	m_AnimCounter = 0;

	ENSURE(YouLookAt.LoadString(IDS_YOULOOKAT));
	m_LockUpdate = FALSE;
}

BOOL CGlobeView::Create(CWnd* pParentWnd, UINT nID, LFSearchResult* Result, INT FocusItem)
{
	return CFileView::Create(pParentWnd, nID, Result, FocusItem, CS_DBLCLKS | CS_OWNDC);
}

void CGlobeView::SetViewOptions(BOOL Force)
{
	if (Force)
	{
		m_GlobeLatitude = p_ViewParameters->GlobeLatitude/1000.0f;
		m_GlobeLongitude = p_ViewParameters->GlobeLongitude/1000.0f;
		m_GlobeZoom = p_ViewParameters->GlobeZoom;
	}

	PrepareTexture();

	if (Force || (m_IsHQModel!=theApp.m_GlobeHQModel))
	{
		PrepareModel();
		m_IsHQModel = theApp.m_GlobeHQModel;
	}

	m_ViewParameters = *p_ViewParameters;
	UpdateScene(TRUE);
}

void CGlobeView::SetSearchResult(LFSearchResult* Result)
{
	p_Result = Result;

	if (Result)
		if (Result->m_ItemCount)
			for (UINT a=0; a<Result->m_ItemCount; a++)
			{
				LFGeoCoordinates coord = { 0.0, 0.0 };
				if (m_ViewParameters.SortBy==LFAttrLocationIATA)
				{
					LFAirport* airport;
					if (LFIATAGetAirportByCode((CHAR*)Result->m_Items[a]->AttributeValues[LFAttrLocationIATA], &airport))
						coord = airport->Location;
				}
				else
					if (Result->m_Items[a]->AttributeValues[m_ViewParameters.SortBy])
					{
						ASSERT(theApp.m_Attributes[m_ViewParameters.SortBy]->Type==LFTypeGeoCoordinates);
						coord = *((LFGeoCoordinates*)Result->m_Items[a]->AttributeValues[m_ViewParameters.SortBy]);
					}

				if ((coord.Latitude!=0.0) || (coord.Longitude!=0))
				{
					GlobeItemData* d = GetItemData(a);
					CalculateWorldCoords(coord.Latitude, coord.Longitude, d->World);
					LFGeoCoordinatesToString(coord, d->CoordString, 32, false);

					d->Valid = TRUE;
				}
			}

	UpdateScene(TRUE);
}

INT CGlobeView::ItemAtPosition(CPoint point)
{
	if (!p_Result)
		return -1;

	INT res = -1;
	float alpha = 0.0f;

	for (UINT a=0; a<p_Result->m_ItemCount; a++)
	{
		GlobeItemData* d = GetItemData(a);

		if (d->Valid)
			if ((d->Alpha>0.1f) && ((d->Alpha>alpha-0.05f) || (d->Alpha>0.75f)))
				if (PtInRect(&d->Hdr.Rect, point))
				{
					res = a;
					alpha = d->Alpha;
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

BOOL CGlobeView::CursorOnGlobe(CPoint point)
{
	double distX = point.x-(double)m_Width/2;
	double distY = point.y-(double)m_Height/2;

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

void CGlobeView::glDrawIcon(GLdouble x, GLdouble y, GLdouble Size, GLdouble Alpha, UINT ID)
{
	x -= 0.375;
	y -= 0.375;
	Size /= 2.0;

	GLdouble s = (ID%2) ? 0.5 : 0.0;
	GLdouble t = (ID/2) ? 0.5 : 0.0;

	glColor4d(1.0, 1.0, 1.0, Alpha);

	glTexCoord2d(s, t);
	glVertex2d(x-Size, y-Size);
	glTexCoord2d(s+0.5, t);
	glVertex2d(x+Size, y-Size);
	glTexCoord2d(s+0.5, t+0.5);
	glVertex2d(x+Size, y+Size);
	glTexCoord2d(s, t+0.5);
	glVertex2d(x-Size, y+Size);
}

void CGlobeView::CalcAndDrawSpots(GLdouble ModelView[4][4], GLdouble Projection[4][4])
{
	GLdouble SizeX = m_Width/2.0;
	GLdouble SizeY = m_Height/2.0;

	GLdouble MVP[4][4];
	MatrixMul(MVP, ModelView, Projection);

	for (UINT a=0; a<p_Result->m_ItemCount; a++)
	{
		GlobeItemData* d = GetItemData(a);
		if (d->Valid)
		{
			d->Alpha = 0.0f;

			GLdouble z = ModelView[0][2]*d->World[0] + ModelView[1][2]*d->World[1] + ModelView[2][2]*d->World[2];
			if ((z>m_FogEnd) && (m_Width) && (m_Height))
			{
				GLdouble w = MVP[0][3]*d->World[0] + MVP[1][3]*d->World[1] + MVP[2][3]*d->World[2] + MVP[3][3];
				GLdouble x = (MVP[0][0]*d->World[0] + MVP[1][0]*d->World[1] + MVP[2][0]*d->World[2] + MVP[3][0])*SizeX/w + SizeX + 0.5;
				GLdouble y = -(MVP[0][1]*d->World[0] + MVP[1][1]*d->World[1] + MVP[2][1]*d->World[2] + MVP[3][1])*SizeY/w + SizeY + 0.5;

				d->ScreenPoint[0] = (INT)x;
				d->ScreenPoint[1] = (INT)y;
				d->Alpha = 1.0f;
				if (z<m_FogStart)
					d->Alpha -= (GLfloat)((m_FogStart-z)/(m_FogStart-m_FogEnd));

				if (m_ViewParameters.GlobeShowSpots)
					glDrawIcon(x, y, 13.0*d->Alpha, d->Alpha, SPOT);
			}
		}
	}
}

void CGlobeView::DrawStatusBar(INT Height, GLfloat BackColor[], BOOL Themed)
{
	WCHAR Copyright[] = L"� NASA's Earth Observatory";
	INT CopyrightWidth = (INT)m_Fonts[0].GetTextWidth(Copyright);
	if (m_Width<CopyrightWidth)
		return;

	WCHAR Viewpoint[256] = L"";
	INT ViewpointWidth = -1;
	if (m_ViewParameters.GlobeShowViewport)
	{
		WCHAR Coord[256];
		LFGeoCoordinates c;
		c.Latitude = -m_Latitude;
		c.Longitude = (m_Longitude>180.0) ? 360-m_Longitude : -m_Longitude;
		LFGeoCoordinatesToString(c, Coord, 256, true);

		swprintf(Viewpoint, 256, YouLookAt, Coord);

		ViewpointWidth = (INT)m_Fonts[0].GetTextWidth(Viewpoint);
		if (m_Width<CopyrightWidth+ViewpointWidth+48)
			ViewpointWidth = -1;
	}

	// Kante
	glColor4f(BackColor[0], BackColor[1], BackColor[2], 0.8f);
	glBegin(GL_LINES);
	glVertex2i(0, m_Height-Height);
	glVertex2i(m_Width, m_Height-Height);
	glEnd();

	// F�llen
	glColor4d(BackColor[0], BackColor[1], BackColor[2], 0.65f);
	glRecti(0, m_Height-Height, m_Width, m_Height);

	// Text
	GLfloat TextColor[4];
	ColorRef2GLColor(TextColor, Themed ? 0xCC6600 : GetSysColor(COLOR_WINDOWTEXT));
	glColor4d(TextColor[0], TextColor[1], TextColor[2], 1.0f);

	INT Gutter = (ViewpointWidth>0) ? (m_Width-CopyrightWidth-ViewpointWidth)/3 : (m_Width-CopyrightWidth)/2;
	m_Fonts[0].Render(Copyright, Gutter, m_Height-Height);
	if (ViewpointWidth>0)
		m_Fonts[0].Render(Viewpoint, m_Width-ViewpointWidth-Gutter, m_Height-Height);
	}


BEGIN_MESSAGE_MAP(CGlobeView, CFileView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(IDM_GLOBE_ZOOMIN, OnZoomIn)
	ON_COMMAND(IDM_GLOBE_ZOOMOUT, OnZoomOut)
	ON_COMMAND(IDM_GLOBE_AUTOSIZE, OnAutosize)
	ON_COMMAND(IDM_GLOBE_OPTIONS, OnOptions)
	ON_COMMAND(IDM_GLOBE_JUMPTOLOCATION, OnJumpToLocation)
	ON_COMMAND(IDM_GLOBE_GOOGLEEARTH, OnGoogleEarth)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_GLOBE_ZOOMIN, IDM_GLOBE_GOOGLEEARTH, OnUpdateCommands)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

INT CGlobeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFileView::OnCreate(lpCreateStruct)==-1)
		return -1;

	Init();
	SetTimer(1, 10, NULL);

	return 0;
}

void CGlobeView::OnDestroy()
{
	KillTimer(1);

	if (m_pDC)
	{
		wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);

		if (m_TextureGlobe)
			delete m_TextureGlobe;
		if (m_TextureIcons)
			delete m_TextureIcons;
		if (m_GlobeModel!=-1)
			glDeleteLists(m_GlobeModel, 1);

		wglMakeCurrent(NULL, NULL);
		if (m_hRC)
			wglDeleteContext(m_hRC);
		delete m_pDC;
	}

	if (p_ViewParameters)
	{
		p_ViewParameters->GlobeLatitude = (INT)(m_GlobeLatitude*1000.0f);
		p_ViewParameters->GlobeLongitude = (INT)(m_GlobeLongitude*1000.0f);
		p_ViewParameters->GlobeZoom = m_GlobeZoom;
	}

	CWnd::OnDestroy();
}

void CGlobeView::OnZoomIn()
{
	if (m_GlobeZoom>0)
	{
		m_GlobeZoom -= 10;
		UpdateScene();
	}
}

void CGlobeView::OnZoomOut()
{
	if (m_GlobeZoom<100)
	{
		m_GlobeZoom += 10;
		UpdateScene();
	}
}

void CGlobeView::OnAutosize()
{
	m_GlobeZoom = 60;
	UpdateScene();
}

void CGlobeView::OnOptions()
{
	GlobeOptionsDlg dlg(this, p_ViewParameters, m_Context);
	if (dlg.DoModal()==IDOK)
		theApp.UpdateViewOptions();
}

void CGlobeView::OnJumpToLocation()
{
	LFSelectLocationIATADlg dlg(IDD_JUMPTOIATA, this);

	if (dlg.DoModal()==IDOK)
	{
		ASSERT(dlg.m_Airport);

		m_AnimCounter = ANIMLENGTH;
		m_AnimStartLatitude = m_Latitude;
		m_AnimStartLongitude = m_Longitude;
		m_GlobeLatitude = (GLfloat)-dlg.m_Airport->Location.Latitude;
		m_GlobeLongitude = (GLfloat)-dlg.m_Airport->Location.Longitude;

		UpdateScene();
	}
}

void CGlobeView::OnGoogleEarth()
{
	// Dateinamen finden
	TCHAR Pathname[MAX_PATH];
	if (!GetTempPath(MAX_PATH, Pathname))
		_tcscpy_s(Pathname, MAX_PATH, theApp.m_Path);

	CString szTempName;
	srand(rand());
	szTempName.Format(_T("%sliquidFOLDERS%.4X%.4X.KML"), Pathname, 32768+rand(), 32768+rand());

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
				LFGeoCoordinates c = p_Result->m_Items[i]->CoreAttributes.LocationGPS;
				if ((c.Latitude!=0) || (c.Longitude!=0))
				{
					f.WriteString(_T("<Placemark>\n<name>"));
					f.WriteString(CookAttributeString(p_Result->m_Items[i]->CoreAttributes.FileName));
					f.WriteString(_T("</name>\n<description>"));
					WriteGoogleAttribute(&f, p_Result->m_Items[i], LFAttrLocationName);
					WriteGoogleAttribute(&f, p_Result->m_Items[i], LFAttrLocationIATA);
					WriteGoogleAttribute(&f, p_Result->m_Items[i], LFAttrLocationGPS);
					WriteGoogleAttribute(&f, p_Result->m_Items[i], LFAttrArtist);
					WriteGoogleAttribute(&f, p_Result->m_Items[i], LFAttrRoll);
					WriteGoogleAttribute(&f, p_Result->m_Items[i], LFAttrRecordingTime);
					WriteGoogleAttribute(&f, p_Result->m_Items[i], LFAttrComment);
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

			ShellExecute(GetSafeHwnd(), _T("open"), szTempName, NULL, NULL, SW_SHOW);
		}
		catch(CFileException ex)
		{
			LFErrorBox(LFDriveNotReady);
		}
		f.Close();
	}
}

void CGlobeView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;
	switch (pCmdUI->m_nID)
	{
	case IDM_GLOBE_ZOOMIN:
		b = m_GlobeZoom>0;
		break;
	case IDM_GLOBE_ZOOMOUT:
		b = m_GlobeZoom<100;
		break;
	case IDM_GLOBE_AUTOSIZE:
		b = m_GlobeZoom!=60;
		break;
	case IDM_GLOBE_OPTIONS:
		break;
	case IDM_GLOBE_GOOGLEEARTH:
		b = (GetNextSelectedItem(-1)!=-1) && (!theApp.m_PathGoogleEarth.IsEmpty());
		break;
	}

	pCmdUI->Enable(b);
}

void CGlobeView::OnLButtonDown(UINT nFlags, CPoint point)
{
	INT n = ItemAtPosition(point);
	if (n==-1)
	{
		if (GetFocus()!=this)
			SetFocus();

		if (CursorOnGlobe(point))
		{
			m_GrabPoint = point;
			m_Grabbed = TRUE;
			if (m_AnimCounter)
			{
				m_AnimCounter = 0;
				m_GlobeLatitude = m_Latitude;
				m_GlobeLongitude = m_Longitude;
				m_GlobeZoom = (INT)(m_Zoom*100.f);
			}

			SetCapture();
			UpdateCursor();
		}
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
		m_Grabbed = FALSE;
		ReleaseCapture();
		UpdateCursor();
	}
	else
	{
		CFileView::OnLButtonUp(nFlags, point);
	}
}

void CGlobeView::OnMouseMove(UINT nFlags, CPoint point)
{
	m_CursorPos = point;

	if (m_Grabbed)
	{
		CSize rotate = m_GrabPoint - point;
		m_GrabPoint = point;
		m_GlobeLongitude = m_Longitude -= rotate.cx/m_Scale*0.12f;
		m_GlobeLatitude = m_Latitude -= rotate.cy/m_Scale*0.12f;

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

BOOL CGlobeView::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	SetCursor(hCursor);
	return TRUE;
}

void CGlobeView::OnSize(UINT nType, INT cx, INT cy)
{
	if (cy>0)
	{
		m_Width = cx;
		m_Height = cy;

		wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);
		glViewport(0, 0, cx, cy);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(3.0f, (GLdouble)cx/cy, 0.1f, 500.0f);
	}

	CFileView::OnSize(nType, cx, cy);
}

void CGlobeView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
		if (UpdateScene())
			m_TooltipCtrl.Deactivate();

	CWnd::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

BOOL CGlobeView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CGlobeView::OnPaint()
{
	DrawScene();
	CFileView::OnPaint();
}


// OpenGL-Objekt
//

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

void CGlobeView::Init()
{
	m_pDC = new CClientDC(this);
	ASSERT(m_pDC);

	if (!SetupPixelFormat())
		return;

	PIXELFORMATDESCRIPTOR pfd;
	INT n = GetPixelFormat(m_pDC->GetSafeHdc());
	DescribePixelFormat(m_pDC->GetSafeHdc(), n, sizeof(pfd), &pfd);

	m_hRC = wglCreateContext(m_pDC->GetSafeHdc());
	wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_SMOOTH);
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_DENSITY, 1.0f);
	glHint(GL_FOG_HINT, GL_FASTEST);

	// Fonts
	m_Fonts[0].Create(&theApp.m_DefaultFont);
	m_Fonts[1].Create(&theApp.m_LargeFont);

	// Icons
	CGdiPlusBitmapResource tex0(IDB_GLOBEICONS_RGB, _T("PNG"));
	CGdiPlusBitmapResource tex1(IDB_GLOBEICONS_ALPHA, _T("PNG"));
	m_TextureIcons = new GLTextureCombine(&tex0, &tex1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

void CGlobeView::PrepareTexture()
{
	// Textur pr�fen
	UINT tex = theApp.m_nTextureSize;

	// Automatisch h�chstens 4096x4096 laden, da quadratisch und von den meisten Grafikkarten unterst�tzt
	if (tex==LFTextureAuto)
		tex = LFTexture4096;

	GLint texSize = 1024;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);

Smaller:
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, 4, texSize, texSize, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
	GLint proxySize = 0;
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &proxySize);

	if ((proxySize==0) && (texSize>1024))
	{
		texSize /= 2;
		goto Smaller;
	}

	if (texSize>=8192)
	{
		theApp.m_nMaxTextureSize = LFTexture8192;
	}
	else
		if (texSize>=4096)
		{
			theApp.m_nMaxTextureSize = LFTexture4096;
		}
		else
			if (texSize>=2048)
			{
				theApp.m_nMaxTextureSize = LFTexture2048;
			}
			else
			{
				theApp.m_nMaxTextureSize = LFTexture1024;
			}

	if (tex>theApp.m_nMaxTextureSize)
		tex = theApp.m_nMaxTextureSize;

	if ((INT)tex!=m_nTexture)
	{
		SetCursor(LoadCursor(NULL, IDC_WAIT));

		m_LockUpdate = TRUE;
		wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);

		if (m_TextureGlobe)
			delete m_TextureGlobe;
		m_TextureGlobe = new GLTextureBlueMarble(tex);

		m_LockUpdate = FALSE;
		SetCursor(hCursor);

		m_nTexture = tex;
		Invalidate();
	}
}

void CGlobeView::PrepareModel()
{
	// 3D-Modelle einbinden
	#include "Globe_Low.h"
	#include "Globe_High.h"
	UINT count = (theApp.m_GlobeHQModel ? globe_high_node_count : globe_low_node_count);
	double* nodes = (theApp.m_GlobeHQModel ? &globe_high[0] : &globe_low[0]);

	// Display-Liste f�r das 3D-Modell erstellen
	m_LockUpdate = TRUE;
	wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);

	m_GlobeModel = glGenLists(1);
	glNewList(m_GlobeModel, GL_COMPILE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_TEXTURE_2D);

	glBegin(GL_TRIANGLES);

	UINT pos = 0;
	for (UINT a=0; a<count; a++)
	{
		double u = nodes[pos++];
		double v = nodes[pos++];
		glTexCoord2d(u, v);

		double x = nodes[pos++];
		double y = nodes[pos++];
		double z = nodes[pos++];
		glNormal3d(x, y, z);
		glVertex3d(x, y, z);
	}

	glEnd();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glEndList();

	m_LockUpdate = FALSE;
}

BOOL CGlobeView::SetupPixelFormat()
{
	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
		1,                              // version number
		PFD_DRAW_TO_WINDOW |            // support window
		PFD_SUPPORT_OPENGL |            // support OpenGL
		PFD_DOUBLEBUFFER,               // double buffered
		PFD_TYPE_RGBA,                  // RGBA type
		32,                             // 32-bit color depth
		0, 0, 0, 0, 0, 0,               // color bits ignored
		0,                              // no alpha buffer
		0,                              // shift bit ignored
		0,                              // no accumulation buffer
		0, 0, 0, 0,                     // accum bits ignored
		16,                             // 16-bit z-buffer
		0,                              // no stencil buffer
		0,                              // no auxiliary buffer
		PFD_MAIN_PLANE,                 // main layer
		0,                              // reserved
		0, 0, 0                         // layer masks ignored
	};

	INT pixelformat = ChoosePixelFormat(*m_pDC, &pfd);
	return pixelformat ? SetPixelFormat(*m_pDC, pixelformat, &pfd) : FALSE;
}

void CGlobeView::Normalize()
{
	// Zoom
	if (m_GlobeZoom<0)
		m_GlobeZoom = 0;
	if (m_GlobeZoom>100)
		m_GlobeZoom = 100;

	// Nicht �ber die Pole rollen
	if (m_GlobeLatitude<-75.0f)
		m_GlobeLatitude = -75.0f;
	if (m_GlobeLatitude>75.0f)
		m_GlobeLatitude = 75.0f;

	// Rotation normieren
	if (m_GlobeLongitude<0.0f)
		m_GlobeLongitude += 360.0f;
	if (m_GlobeLongitude>360.0f)
		m_GlobeLongitude -= 360.0f;
}

BOOL CGlobeView::UpdateScene(BOOL Redraw)
{
	if (m_LockUpdate)
		return FALSE;
	m_LockUpdate = TRUE;

	BOOL res = Redraw;
	Normalize();

	GLfloat TargetZoom = m_GlobeZoom/100.0f;

	if (m_Zoom<0)
	{
		res |= (m_Zoom!=TargetZoom);
		m_Zoom = TargetZoom;
	}
	else
	{
		if (m_Zoom<TargetZoom-0.001f)
		{
			res = TRUE;
			m_Zoom += 0.005f;
		}
		if (m_Zoom>TargetZoom+0.001f)
		{
			res = TRUE;
			m_Zoom -= 0.005f;
		}
	}

	if ((m_Latitude==0.0f) && (m_Longitude==0.0f))
	{
		res |= (m_Latitude!=m_GlobeLatitude) || (m_Longitude!=m_GlobeLongitude);
		m_Latitude = m_GlobeLatitude;
		m_Longitude = m_GlobeLongitude;
	}
	else
	{
		if (m_AnimCounter)
		{
			GLfloat f = (GLfloat)((cos(PI*m_AnimCounter/ANIMLENGTH)+1.0)/2.0);
			m_Latitude = m_AnimStartLatitude*(1.0f-f) + m_GlobeLatitude*f;
			m_Longitude = m_AnimStartLongitude*(1.0f-f) + m_GlobeLongitude*f;

			if (TargetZoom<0.6f)
			{
				GLfloat dist = 0.6f-TargetZoom;
				if (dist>(TargetZoom+0.1f)*1.2f)
					dist = (TargetZoom+0.1f)*1.2f;

				f = (GLfloat)sin(PI*m_AnimCounter/ANIMLENGTH);
				m_Zoom = TargetZoom*(1.0f-f)+(TargetZoom+dist)*f;
			}

			m_AnimCounter--;
			res = TRUE;
		}
		else
		{
			res |= (m_Latitude!=m_GlobeLatitude) || (m_Longitude!=m_GlobeLongitude);
			m_Latitude = m_GlobeLatitude;
			m_Longitude = m_GlobeLongitude;
		}
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

void CGlobeView::DrawScene(BOOL InternalCall)
{
	if (!InternalCall)
	{
		if (m_LockUpdate)
			return;
		m_LockUpdate = TRUE;
	}

	BOOL Themed = IsCtrlThemed();

	wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);
	glRenderMode(GL_RENDER);

	// Hintergrund
	GLfloat BackColor[4];
	ColorRef2GLColor(BackColor, Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));
	glFogfv(GL_FOG_COLOR, BackColor);

	glClearColor(BackColor[0], BackColor[1], BackColor[2], 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Globus berechnen
	m_Scale = 1.0f;
	if (m_Height>m_Width)
		m_Scale = 1-((GLfloat)(m_Height-m_Width))/m_Height;

	GLfloat zoomfactor = m_Zoom+0.4f;
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
	glRotatef(m_Latitude, 0.0f, 1.0f, 0.0f);
	glRotatef(m_Longitude, 0.0f, 0.0f, 1.0f);
	glScalef(m_Scale, m_Scale, m_Scale);

	// Atmosph�re/Nebel
	if (theApp.m_GlobeAtmosphere)
	{
		glEnable(GL_FOG);
		glFogf(GL_FOG_START, DISTANCE-m_FogStart);
		glFogf(GL_FOG_END, DISTANCE-m_FogEnd);
	}

	// Globus-Textur
	if (m_TextureGlobe)
	{
		glBindTexture(GL_TEXTURE_2D, m_TextureGlobe->GetID());
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, theApp.m_GlobeLighting ? GL_MODULATE : GL_REPLACE);
	}

	// Modell rendern
	glCallList(m_GlobeModel);

	// Atmosph�re aus
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
	GLdouble ModelView[4][4];
	GLdouble Projection[4][4];
	glGetDoublev(GL_MODELVIEW_MATRIX, &ModelView[0][0]);
	glGetDoublev(GL_PROJECTION_MATRIX, &Projection[0][0]);

	// F�r Icons vorbereiten
	glEnable2D();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_TextureIcons->GetID());
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBegin(GL_QUADS);

	// Koordinaten bestimmen und Spots zeichnen
	if (p_Result)
		if (p_Result->m_ItemCount)
			CalcAndDrawSpots(ModelView, Projection);

	// Fadenkreuz zeichnen
	if (m_ViewParameters.GlobeShowCrosshair)
		glDrawIcon(m_Width/2.0, m_Height/2.0, 64.0, 1.0, CROSSHAIRS);

	// Icons beenden
	glEnd();
	glDisable(GL_TEXTURE_2D);

	// Label zeichnen
	if (p_Result)
		if (p_Result->m_ItemCount)
			CalcAndDrawLabel();

	// Statuszeile
	const INT Height = m_FontHeight[0]+1;
	if (m_Height>=Height)
		DrawStatusBar(Height, BackColor, Themed);

	// Beenden
	glDisable2D();

	SwapBuffers(*m_pDC);
	m_LockUpdate = FALSE;
}


void CGlobeView::CalcAndDrawLabel()
{
	GLfloat labelcol[4];
	ColorRef2GLColor(labelcol, GetSysColor(COLOR_3DFACE));
	GLfloat textcol[4];
	ColorRef2GLColor(textcol, GetSysColor(COLOR_WINDOWTEXT));

	for (UINT a=0; a<p_Result->m_ItemCount; a++)
	{
		GlobeItemData* d = GetItemData(a);

		if (d->Valid)
			if (d->Alpha>0.0f)
			{
				// Beschriftung
				WCHAR* caption = p_Result->m_Items[a]->CoreAttributes.FileName;
				UINT cCaption = (UINT)wcslen(caption);
				WCHAR* subcaption = NULL;
				WCHAR* coordinates = (m_ViewParameters.GlobeShowGPS ? d->CoordString : NULL);
				WCHAR* description = (m_ViewParameters.GlobeShowDescription ? p_Result->m_Items[a]->Description : NULL);
				if (description)
					if (*description==L'\0')
						description = NULL;

				// Beschriftung aufbereiten
				switch (m_ViewParameters.SortBy)
				{
				case LFAttrLocationIATA:
					if (cCaption>6)
					{
						if (m_ViewParameters.GlobeShowAirportNames)
							subcaption = &caption[6];
						cCaption = 3;
					}
					break;
				case LFAttrLocationGPS:
					if ((wcscmp(caption, d->CoordString)==0) && (m_ViewParameters.GlobeShowGPS))
						coordinates = NULL;
					break;
				}

				DrawLabel(d, cCaption, caption, subcaption, coordinates, description, m_FocusItem==(INT)a);
			}
	}
}

void CGlobeView::DrawLabel(GlobeItemData* d, UINT cCaption, WCHAR* caption, WCHAR* subcaption, WCHAR* coordinates, WCHAR* description, BOOL focused)
{
	ASSERT(ARROWSIZE>3);

	COLORREF BaseColorRef = GetSysColor(COLOR_WINDOW);
	COLORREF TextColorRef = GetSysColor(COLOR_WINDOWTEXT);
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

	// Breite
	UINT width = m_Fonts[1].GetTextWidth(caption, cCaption);
	width = max(width, m_Fonts[0].GetTextWidth(subcaption));
	width = max(width, m_Fonts[0].GetTextWidth(coordinates));
	width = max(width, m_Fonts[0].GetTextWidth(description));
	width += 8;

	// H�he
	UINT height = m_Fonts[1].GetTextHeight(caption);
	height += m_Fonts[0].GetTextHeight(subcaption);
	height += m_Fonts[0].GetTextHeight(coordinates);
	height += m_Fonts[0].GetTextHeight(description);
	height += 3;

	// Position
	INT top = (d->ScreenPoint[1]<m_Height/2) ? -1 : 1;

	INT x = d->Hdr.Rect.left = d->ScreenPoint[0]-ARROWSIZE-((width-2*ARROWSIZE)*(m_Width-d->ScreenPoint[0])/m_Width);
	INT y = d->Hdr.Rect.top = d->ScreenPoint[1]+(ARROWSIZE-2)*top-(top<0 ? height : 0);
	d->Hdr.Rect.right = x+width;
	d->Hdr.Rect.bottom = y+height;

	// Schatten
	if (theApp.m_GlobeShadows)
	{
		for (INT s=3; s>0; s--)
		{
			glColor4f(0.0f, 0.0f, 0.0f, d->Alpha/(s+2.5f));
			glBegin(GL_LINES);
			glVertex2i(x+2, y+height+s);
			glVertex2i(x+width+s, y+height+s);
			glVertex2i(x+width+s, y+2);
			glVertex2i(x+width+s, y+height+s+1);
			glEnd();
		}

		glColor4f(0.0f, 0.0f, 0.0f, d->Alpha/2.5f);
		glBegin(GL_LINES);
		glVertex2i(x+width, y+height);
		glVertex2i(x+width+1, y+height);
	}
	else
	{
		glBegin(GL_LINES);
	}

	// Grauer Rand
	glColor4f(BaseColor[0]/2, BaseColor[1]/2, BaseColor[2]/2, d->Alpha);
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0xFFFF);
	glVertex2i(x, y-1);						// Oben
	glVertex2i(x+width, y-1);
	glVertex2i(x, y+height);				// Unten
	glVertex2i(x+width, y+height);
	glVertex2i(x-1, y);						// Links
	glVertex2i(x-1, y+height);
	glVertex2i(x+width, y);					// Rechts
	glVertex2i(x+width, y+height);
	glDisable(GL_LINE_STIPPLE);
	glEnd();

	// Pfeil
	glBegin(GL_TRIANGLES);
	GLfloat alpha = pow(d->Alpha, 3);
	for (INT a=0; a<=3; a++)
	{
		switch (a)
		{
		case 0:
			glColor4f(BaseColor[0]/2, BaseColor[1]/2, BaseColor[2]/2, alpha/2);
			break;
		case 1:
			glColor4f(BaseColor[0]/2, BaseColor[1]/2, BaseColor[2]/2, alpha);
			break;
		case 2:
			glColor4f(BaseColor[0], BaseColor[1], BaseColor[2], alpha/2);
			break;
		default:
			glColor4f(BaseColor[0], BaseColor[1], BaseColor[2], d->Alpha);
		}

		glVertex2i(d->ScreenPoint[0], d->ScreenPoint[1]+(a-2)*top);
		glVertex2i(d->ScreenPoint[0]+(ARROWSIZE-a)*top, d->ScreenPoint[1]+(ARROWSIZE-2)*top);
		glVertex2i(d->ScreenPoint[0]-(ARROWSIZE-a)*top, d->ScreenPoint[1]+(ARROWSIZE-2)*top);
	}
	glEnd();

	// Innen
	glRecti(x, y, x+width, y+height);
	if ((focused) && (GetFocus()==this))
	{
		glColor4f(1.0f-BaseColor[0], 1.0f-BaseColor[1], 1.0f-BaseColor[2], d->Alpha);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xAAAA);
		glBegin(GL_LINE_LOOP);
		glVertex2i(x, y);
		glVertex2i(x+width-1, y);
		glVertex2i(x+width-1, y+height-1);
		glVertex2i(x, y+height-1);
		glEnd();
		glDisable(GL_LINE_STIPPLE);
	}

	x += 3;

	glColor4f(TextColor[0], TextColor[1], TextColor[2], d->Alpha);
	y += m_Fonts[1].Render(caption, x, y, cCaption);
	if (subcaption)
		y += m_Fonts[0].Render(subcaption, x, y);

	if ((!d->Hdr.Selected) && (BaseColorRef==0xFFFFFF) && (TextColorRef==0x000000))
		glColor4f(TextColor[0], TextColor[1], TextColor[2], d->Alpha/2);
	if (coordinates)
		y += m_Fonts[0].Render(coordinates, x, y);
	if (description)
		y += m_Fonts[0].Render(description, x, y);
}
