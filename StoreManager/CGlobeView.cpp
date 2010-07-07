
// CGlobeView.cpp: Implementierung der Klasse CGlobeView
//

#include "stdafx.h"
#include "CGlobeView.h"
#include "Resource.h"
#include "LFCore.h"
#include <cmath>

#define STATUSBAR_HEIGHT 16
#define DISTANCE 39.0f
#define ARROWSIZE 9
#define PI 3.14159265358979323846

void ColorRef2GLColor(GLfloat* dst, COLORREF src, GLfloat Alpha=1.0f)
{
	dst[0] = (src&0xFF)/255.0f;
	dst[1] = ((src>>8)&0xFF)/255.0f;
	dst[2] = ((src>>16)&0xFF)/255.0f;
	dst[3] = Alpha;
}

static inline double decToRad(double dec)
{
	return dec*(PI/180.0);
}

void CalculateWorldCoords(double lat, double lon, double result[])
{
	double lon_r = decToRad(lon);
	double lat_r = -decToRad(lat);

	result[0] = cos(lat_r)*cos(lon_r);
	result[1] = cos(lat_r)*sin(lon_r);
	result[2] = sin(lat_r);
}

CString CookAttributeString(wchar_t* attr)
{
	CString tmpStr(attr);
	tmpStr.Replace(_T("<"), _T("_"));
	tmpStr.Replace(_T(">"), _T("_"));
	tmpStr.Replace(_T("&"), _T("&amp;"));

	return tmpStr;
}

void WriteGoogleAttribute(CStdioFile* f, LFItemDescriptor* i, UINT attr)
{
	wchar_t tmpStr[256];
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

CGlobeView::CGlobeView()
{
	m_pDC = NULL;
	m_hrc = NULL;
	hCursor = theApp.LoadStandardCursor(IDC_WAIT);
	m_TextureGlobe = NULL;
	m_pSpecialFont = NULL;
	ZeroMemory(m_pFonts, sizeof(m_pFonts));
	m_Width = 0;
	m_Height = 0;
	m_GlobeList[FALSE] = -1;
	m_GlobeList[TRUE] = -1;
	m_AngleY = 0.0f;
	m_AngleZ = 0.0f;
	m_Zoom = -1.0f;
	m_Scale = 0.7f;
	m_Radius = 0.0f;
	m_Grabbed = FALSE;
	m_Locations = NULL;
	m_CameraChanged = FALSE;
	lpszCursorName = NULL;
	mPoint.x = 0;
	mPoint.y = 0;
	m_nTexture = -1;
	m_LockUpdate = FALSE;
}

CGlobeView::~CGlobeView()
{
	if (m_Locations)
		delete[] m_Locations;
}

void CGlobeView::Create(CWnd* _pParentWnd, LFSearchResult* _result)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC, hCursor, NULL, NULL);

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	CRect rect;
	rect.SetRectEmpty();
	CWnd::Create(className, _T(""), dwStyle, rect, _pParentWnd, AFX_IDW_PANE_FIRST);

	CFileView::Create(_result, LFViewGlobe);
}

void CGlobeView::SetViewOptions(UINT /*_ViewID*/, BOOL Force)
{
	if (Force)
	{
		m_LocalSettings.AngleY = pViewParameters->GlobeAngleY/1000.0f;
		m_LocalSettings.AngleZ = pViewParameters->GlobeAngleZ/1000.0f;
		m_LocalSettings.GlobeZoom = pViewParameters->GlobeZoom;
	}

	if (Force || (pViewParameters->Background!=m_ViewParameters.Background) || (theApp.m_nAppLook!=RibbonColor))
		theApp.GetBackgroundColors(pViewParameters->Background, &m_ColorBack, &m_ColorText, &m_ColorHighlight);

	PrepareFont(FALSE, pViewParameters->GrannyMode);
	SmallFont = m_pFonts[FALSE][pViewParameters->GrannyMode];
	PrepareFont(TRUE, pViewParameters->GrannyMode);
	LargeFont = m_pFonts[TRUE][pViewParameters->GrannyMode];

	PrepareTexture();
	PrepareModel(theApp.m_GlobeHQModel);

	m_ViewParameters = *pViewParameters;
	UpdateScene(TRUE);
}

void CGlobeView::SetSearchResult(LFSearchResult* _result)
{
	if (m_Locations)
	{
		delete[] m_Locations;
		m_Locations = NULL;
	}

	result = _result;
	if (_result)
		if (_result->m_ItemCount)
		{
			m_Locations = new Location[_result->m_ItemCount];

			for (UINT a=0; a<_result->m_ItemCount; a++)
			{
				ZeroMemory(&m_Locations[a], sizeof(Location));

				LFGeoCoordinates coord = { 0, 0 };
				if ((theApp.m_Attributes[m_ViewParameters.SortBy]->Type==LFTypeGeoCoordinates) && (_result->m_Items[a]->AttributeValues[m_ViewParameters.SortBy]))
					coord = *((LFGeoCoordinates*)_result->m_Items[a]->AttributeValues[m_ViewParameters.SortBy]);

				if ((coord.Latitude==0) && (coord.Longitude==0))
					if (_result->m_Items[a]->AttributeValues[LFAttrLocationIATA])
					{
						LFAirport* airport;
						if (LFIATAGetAirportByCode((char*)_result->m_Items[a]->AttributeValues[LFAttrLocationIATA], &airport))
							coord = airport->Location;
					}

				if ((coord.Latitude!=0) || (coord.Longitude!=0))
				{
					CalculateWorldCoords(coord.Latitude, coord.Longitude, m_Locations[a].world);
					LFGeoCoordinatesToString(coord, m_Locations[a].coordstring, 32);
					m_Locations[a].valid = TRUE;
					m_Locations[a].selected = FALSE;
				}
			}
		}

	UpdateScene(TRUE);
}

void CGlobeView::SelectItem(int n, BOOL select, BOOL InternalCall)
{
	if (m_Locations)
	{
		m_Locations[n].selected = select;

		if (!InternalCall)
		{
			UpdateScene(TRUE);
			GetParentFrame()->SendMessage(WM_COMMAND, ID_APP_UPDATESELECTION);
		}
	}
}

int CGlobeView::GetSelectedItem()
{
	if ((m_Locations) && (FocusItem!=-1))
		if (m_Locations[FocusItem].selected)
			return FocusItem;

	return -1;
}

int CGlobeView::GetNextSelectedItem(int n)
{
	if (m_Locations)
	{
		if (n<-1)
			n = -1;

		for (UINT a=(UINT)(n+1); a<result->m_ItemCount; a++)
			if (m_Locations[a].selected)
				return a;
	}

	return -1;
}

BOOL CGlobeView::IsSelected(int n)
{
	return (m_Locations && (n>=0)? m_Locations[n].selected : FALSE);
}

int CGlobeView::ItemAtPosition(CPoint point)
{
	if ((!m_Locations) || (!result) || (!m_ViewParameters.GlobeShowBubbles))
		return -1;

	int res = -1;
	float alpha = 0.0f;
	for (UINT a=0; a<result->m_ItemCount; a++)
	{
		if (m_Locations[a].valid)
			if ((m_Locations[a].alpha>0.1f) && ((m_Locations[a].alpha>alpha-0.05f) || (m_Locations[a].alpha>0.75f)))
				if ((point.x>=m_Locations[a].screenlabel[0]) &&
					(point.x<m_Locations[a].screenlabel[2]) &&
					(point.y>=m_Locations[a].screenlabel[1]) &&
					(point.y<m_Locations[a].screenlabel[3]))
				{
					res = a;
					alpha = m_Locations[a].alpha;
				}
	}

	return res;
}

CMenu* CGlobeView::GetContextMenu()
{
	CMenu* menu = new CMenu();
	menu->LoadMenu(IDM_GLOBE);
	return menu;
}

BOOL CGlobeView::CursorOnGlobe(CPoint point)
{
	double distX = point.x-(double)m_Width/2;
	double distY = point.y-(double)m_Height/2;

	return distX*distX+distY*distY<m_Radius*m_Radius;
}


BEGIN_MESSAGE_MAP(CGlobeView, CFileView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_GLOBE_ZOOMIN, OnZoomIn)
	ON_COMMAND(ID_GLOBE_ZOOMOUT, OnZoomOut)
	ON_COMMAND(ID_GLOBE_SCALETOFIT, OnScaleToFit)
	ON_COMMAND(ID_GLOBE_SAVECAMERA, OnSaveCamera)
	ON_COMMAND(ID_GLOBE_JUMPTOLOCATION, OnJumpToLocation)
	ON_COMMAND(ID_GLOBE_GOOGLEEARTH, OnGoogleEarth)
	ON_COMMAND(ID_GLOBE_HQMODEL, OnHQModel)
	ON_COMMAND(ID_GLOBE_SHOWBUBBLES, OnShowBubbles)
	ON_COMMAND(ID_GLOBE_SHOWAIRPORTNAMES, OnShowAirportNames)
	ON_COMMAND(ID_GLOBE_SHOWGPS, OnShowGPS)
	ON_COMMAND(ID_GLOBE_SHOWHINTS, OnShowHints)
	ON_UPDATE_COMMAND_UI_RANGE(ID_GLOBE_ZOOMIN, ID_GLOBE_SHOWHINTS, OnUpdateCommands)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

void CGlobeView::DisplayCursor(LPCTSTR _lpszCursorName)
{
	if (_lpszCursorName!=lpszCursorName)
	{
		hCursor = theApp.LoadStandardCursor(_lpszCursorName);
		lpszCursorName = _lpszCursorName;
	}
}

void CGlobeView::UpdateCursor()
{
	LPCTSTR csr;
	if (ItemAtPosition(mPoint)==-1)
	{
		csr = (CursorOnGlobe(mPoint) ? IDC_HAND : IDC_ARROW);
	}
	else
	{
		csr = IDC_ARROW;
	}

	if (csr!=lpszCursorName)
	{
		DisplayCursor(csr);
		SetCursor(hCursor);
	}
}

int CGlobeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFileView::OnCreate(lpCreateStruct)==-1)
		return -1;

	Init();
	SetTimer(1, 16, NULL);

	return 0;
}

void CGlobeView::OnDestroy()
{
	KillTimer(1);

	if (m_pDC)
	{
		wglMakeCurrent(m_pDC->GetSafeHdc(), m_hrc);

		if (m_TextureGlobe)
			delete m_TextureGlobe;
		if (m_GlobeList[FALSE]!=-1)
			glDeleteLists(m_GlobeList[FALSE], 1);
		if (m_GlobeList[TRUE]!=-1)
			glDeleteLists(m_GlobeList[TRUE], 1);
		if (m_pSpecialFont)
			delete m_pSpecialFont;

		for (UINT a=0; a<2; a++)
			for (UINT b=0; b<2; b++)
				if (m_pFonts[a][b])
					delete m_pFonts[a][b];

		wglMakeCurrent(NULL, NULL);
		if (m_hrc)
			wglDeleteContext(m_hrc);
		delete m_pDC;
	}

	CWnd::OnDestroy();
}

void CGlobeView::OnZoomIn()
{
	if (m_LocalSettings.GlobeZoom>0)
	{
		m_LocalSettings.GlobeZoom -= 10;
		m_CameraChanged = TRUE;
		UpdateScene();
	}
}

void CGlobeView::OnZoomOut()
{
	if (m_LocalSettings.GlobeZoom<100)
	{
		m_LocalSettings.GlobeZoom += 10;
		m_CameraChanged = TRUE;
		UpdateScene();
	}
}

void CGlobeView::OnScaleToFit()
{
	m_LocalSettings.GlobeZoom = 60;
	m_CameraChanged = TRUE;
	UpdateScene();
}

void CGlobeView::OnSaveCamera()
{
	pViewParameters->GlobeAngleY = (int)(m_LocalSettings.AngleY*1000.0f);
	pViewParameters->GlobeAngleZ = (int)(m_LocalSettings.AngleZ*1000.0f);
	pViewParameters->GlobeZoom = m_LocalSettings.GlobeZoom;
	OnViewOptionsChanged(TRUE);
	m_CameraChanged = FALSE;
}

void CGlobeView::OnJumpToLocation()
{
	LFSelectLocationIATADlg dlg(this, IDD_JUMPTOIATA);

	if (dlg.DoModal()==IDOK)
	{
		ASSERT(dlg.m_Airport);
		m_LocalSettings.AngleY = (GLfloat)-dlg.m_Airport->Location.Latitude;
		m_LocalSettings.AngleZ = (GLfloat)-dlg.m_Airport->Location.Longitude;
		m_CameraChanged = TRUE;
		UpdateScene();
	}
}

void CGlobeView::OnGoogleEarth()
{
	// Dateinamen finden
	TCHAR Pathname[MAX_PATH];
	if (!GetTempPath(MAX_PATH, Pathname))
		_tcscpy_s(Pathname, MAX_PATH, theApp.path);

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

			int i = GetNextSelectedItem(-1);
			while (i>-1)
			{
				LFGeoCoordinates c = result->m_Items[i]->CoreAttributes.LocationGPS;
				if ((c.Latitude!=0) || (c.Longitude!=0))
				{
					f.WriteString(_T("<Placemark>\n<name>"));
					f.WriteString(CookAttributeString(result->m_Items[i]->CoreAttributes.FileName));
					f.WriteString(_T("</name>\n<description>"));
					WriteGoogleAttribute(&f, result->m_Items[i], LFAttrLocationName);
					WriteGoogleAttribute(&f, result->m_Items[i], LFAttrLocationIATA);
					WriteGoogleAttribute(&f, result->m_Items[i], LFAttrLocationGPS);
					WriteGoogleAttribute(&f, result->m_Items[i], LFAttrArtist);
					WriteGoogleAttribute(&f, result->m_Items[i], LFAttrRoll);
					WriteGoogleAttribute(&f, result->m_Items[i], LFAttrRecordingTime);
					WriteGoogleAttribute(&f, result->m_Items[i], LFAttrComment);
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

void CGlobeView::OnHQModel()
{
	theApp.m_GlobeHQModel = !theApp.m_GlobeHQModel;
	OnViewOptionsChanged();
}

void CGlobeView::OnShowBubbles()
{
	pViewParameters->GlobeShowBubbles = !pViewParameters->GlobeShowBubbles;
	OnViewOptionsChanged();
}

void CGlobeView::OnShowAirportNames()
{
	pViewParameters->GlobeShowAirportNames = !pViewParameters->GlobeShowAirportNames;
	OnViewOptionsChanged();
}

void CGlobeView::OnShowGPS()
{
	pViewParameters->GlobeShowGPS = !pViewParameters->GlobeShowGPS;
	OnViewOptionsChanged();
}

void CGlobeView::OnShowHints()
{
	pViewParameters->GlobeShowHints = !pViewParameters->GlobeShowHints;
	OnViewOptionsChanged();
}

void CGlobeView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;
	switch (pCmdUI->m_nID)
	{
	case ID_GLOBE_ZOOMIN:
		b = m_LocalSettings.GlobeZoom>0;
		break;
	case ID_GLOBE_ZOOMOUT:
		b = m_LocalSettings.GlobeZoom<100;
		break;
	case ID_GLOBE_SCALETOFIT:
		b = m_LocalSettings.GlobeZoom!=60;
		break;
	case ID_GLOBE_SAVECAMERA:
		b = m_CameraChanged;
		break;
	case ID_GLOBE_GOOGLEEARTH:
		b = (GetNextSelectedItem(-1)!=-1) && (theApp.path_GoogleEarth!="");
		break;
	case ID_GLOBE_HQMODEL:
		pCmdUI->SetCheck(theApp.m_GlobeHQModel);
		break;
	case ID_GLOBE_SHOWBUBBLES:
		pCmdUI->SetCheck(m_ViewParameters.GlobeShowBubbles);
		break;
	case ID_GLOBE_SHOWAIRPORTNAMES:
		pCmdUI->SetCheck(m_ViewParameters.GlobeShowAirportNames);
		b = m_ViewParameters.GlobeShowBubbles && (m_ViewParameters.SortBy==LFAttrLocationIATA);
		break;
	case ID_GLOBE_SHOWGPS:
		pCmdUI->SetCheck(m_ViewParameters.GlobeShowGPS);
		b = m_ViewParameters.GlobeShowBubbles;
		break;
	case ID_GLOBE_SHOWHINTS:
		pCmdUI->SetCheck(m_ViewParameters.GlobeShowHints);
		b = m_ViewParameters.GlobeShowBubbles;
	}

	pCmdUI->Enable(b);
}

void CGlobeView::OnLButtonDown(UINT nFlags, CPoint point)
{
	int n = ItemAtPosition(point);
	if (n==-1)
	{
		if (GetFocus()!=this)
			SetFocus();

		if (CursorOnGlobe(point))
		{
			m_GrabPoint = point;
			m_Grabbed = TRUE;
			SetCapture();
			DisplayCursor(IDC_HAND);
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
	mPoint = point;

	if (m_Grabbed)
	{
		CSize rotate = m_GrabPoint - point;
		m_GrabPoint = point;
		m_LocalSettings.AngleZ = m_AngleZ -= rotate.cx/m_Scale*0.12f;
		m_LocalSettings.AngleY = m_AngleY -= rotate.cy/m_Scale*0.12f;
		m_CameraChanged = TRUE;

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

void CGlobeView::OnSize(UINT nType, int cx, int cy)
{
	CFileView::OnSize(nType, cx, cy);

	if (cy>0)
	{
		m_Width = cx;
		m_Height = cy;

		wglMakeCurrent(m_pDC->GetSafeHdc(), m_hrc);
		glViewport(0, 0, cx, cy);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(3.0f, (GLdouble)cx/cy, 0.1f, 500.0f);
		glMatrixMode(GL_MODELVIEW);
	}
}

void CGlobeView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
		if (UpdateScene())
		{
			// Schatten aktualisieren
			CRect rect;
			GetClientRect(rect);
			ClientToScreen(rect);
			CMFCPopupMenu::UpdateAllShadows(rect);
		}

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
	CWnd::OnPaint();
}

void CGlobeView::OnSetFocus(CWnd* /*pOldWnd*/)
{
	DrawScene();
}

void CGlobeView::OnKillFocus(CWnd* /*pNewWnd*/)
{
	DrawScene();
}

void CGlobeView::OnSysColorChange()
{
	DrawScene();
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
	int n = GetPixelFormat(m_pDC->GetSafeHdc());
	DescribePixelFormat(m_pDC->GetSafeHdc(), n, sizeof(pfd), &pfd);

	m_hrc = wglCreateContext(m_pDC->GetSafeHdc());
	wglMakeCurrent(m_pDC->GetSafeHdc(), m_hrc);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_SMOOTH);
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_DENSITY, 1.0f);
	glHint(GL_FOG_HINT, GL_NICEST);

	// Spezial-Font
	if (!m_pSpecialFont)
	{
		m_pSpecialFont = new CGLFont();
		m_pSpecialFont->Create(theApp.GetDefaultFontFace(), 12, FALSE, FALSE);
	}
}

void CGlobeView::PrepareFont(BOOL large, BOOL granny)
{
	if (!m_pFonts[large][granny])
	{
		m_pFonts[large][granny] = new CGLFont();
		m_pFonts[large][granny]->Create(&theApp.m_Fonts[large][granny]);
	}
}

void CGlobeView::PrepareTexture()
{
	// Textur prüfen
	UINT tex = theApp.m_nTextureSize;

	// Automatisch höchstens 4096x4096 laden, da quadratisch und von den meisten Grafikkarten unterstützt
	if (tex==LFTextureAuto)
		tex = LFTexture4096;

	GLint texSize = 1024;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
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

	if ((int)tex!=m_nTexture)
	{
		SetCursor(theApp.LoadStandardCursor(IDC_WAIT));

		m_LockUpdate = TRUE;
		wglMakeCurrent(m_pDC->GetSafeHdc(), m_hrc);

		if (m_TextureGlobe)
			delete m_TextureGlobe;
		m_TextureGlobe = new CTextureBlueMarble(tex);

		m_LockUpdate = FALSE;
		SetCursor(hCursor);

		m_nTexture = tex;
		Invalidate();
	}
}

void CGlobeView::PrepareModel(BOOL HQ)
{
	if (m_GlobeList[HQ]==-1)
	{
		// 3D-Modelle einbinden
		#include "Globe_Low.h"
		#include "Globe_High.h"
		UINT count = (HQ ? globe_high_node_count : globe_low_node_count);
		double* nodes = (HQ ? &globe_high[0] : &globe_low[0]);

		// Display-Liste für das 3D-Modell erstellen
		m_LockUpdate = TRUE;
		wglMakeCurrent(m_pDC->GetSafeHdc(), m_hrc);

		m_GlobeList[HQ] = glGenLists(1);
		glNewList(m_GlobeList[HQ], GL_COMPILE);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

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
			glVertex3d(x, y, z);
		}

		glEnd();
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glEndList();

		m_LockUpdate = FALSE;
	}
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

	int pixelformat = ChoosePixelFormat(m_pDC->GetSafeHdc(), &pfd);
	return pixelformat ? SetPixelFormat(m_pDC->GetSafeHdc(), pixelformat, &pfd) : FALSE;
}

BOOL CGlobeView::UpdateScene(BOOL Redraw)
{
	if (m_LockUpdate)
		return FALSE;
	m_LockUpdate = TRUE;

	BOOL res = Redraw;

	// Zoom
	if (m_LocalSettings.GlobeZoom<0)
		m_LocalSettings.GlobeZoom = 0;
	if (m_LocalSettings.GlobeZoom>100)
		m_LocalSettings.GlobeZoom = 100;

	GLfloat TargetZoom = m_LocalSettings.GlobeZoom/100.0f;

	if (m_Zoom<0)
	{
		res |= (m_Zoom!=TargetZoom);
		m_Zoom = TargetZoom;
	}
	else
	{
		if (m_Zoom<TargetZoom-0.009f)
		{
			res = TRUE;
			m_Zoom += 0.01f;
		}
		if (m_Zoom>TargetZoom+0.009f)
		{
			res = TRUE;
			m_Zoom -= 0.01f;
		}
	}

	// Nicht über die Pole rollen
	if (m_LocalSettings.AngleY<-75.0f)
		m_LocalSettings.AngleY = -75.0f;
	if (m_LocalSettings.AngleY>75.0f)
		m_LocalSettings.AngleY = 75.0f;

	// Rotation normieren
	if (m_LocalSettings.AngleZ<0.0f)
		m_LocalSettings.AngleZ += 360.0f;
	if (m_LocalSettings.AngleZ>360.0f)
		m_LocalSettings.AngleZ -= 360.0f;

	if ((m_AngleY==0.0f) && (m_AngleZ==0.0f))
	{
		res |= (m_AngleY!=m_LocalSettings.AngleY) || (m_AngleZ!=m_LocalSettings.AngleZ);
		m_AngleY = m_LocalSettings.AngleY;
		m_AngleZ = m_LocalSettings.AngleZ;
	}
	else
	{
		// Nicht über die Pole rollen
		if (m_AngleY<-75.0f)
			m_AngleY = -75.0f;
		if (m_AngleY>75.0f)
			m_AngleY = 75.0f;

		// Rotation normieren
		if (m_AngleZ-m_LocalSettings.AngleZ<-180.0f)
			m_AngleZ += 360.0f;
		if (m_AngleZ-m_LocalSettings.AngleZ>180.0f)
			m_AngleZ -= 360.0f;

		if ((abs(m_AngleY-m_LocalSettings.AngleY)>0.1f) || (abs(m_AngleZ-m_LocalSettings.AngleZ)>0.1f))
		{
			res = TRUE;
			m_AngleY = (m_AngleY*19+m_LocalSettings.AngleY)/20;
			m_AngleZ = (m_AngleZ*19+m_LocalSettings.AngleZ)/20;
		}
		else
		{
			res |= (m_AngleY!=m_LocalSettings.AngleY) || (m_AngleZ!=m_LocalSettings.AngleZ);
			m_AngleY = m_LocalSettings.AngleY;
			m_AngleZ = m_LocalSettings.AngleZ;
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

	wglMakeCurrent(m_pDC->GetSafeHdc(), m_hrc);
	glRenderMode(GL_RENDER);

	// Hintergrund
	GLfloat backcol[4];
	ColorRef2GLColor(backcol, m_ColorBack);
	glFogfv(GL_FOG_COLOR, backcol);

	glClearColor(backcol[0], backcol[1], backcol[2], 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Globus berechnen
	m_Scale = 1.0f;
	if (m_Height>m_Width)
		m_Scale = 1-((GLfloat)(m_Height-m_Width))/m_Height;

	GLfloat zoomfactor = m_Zoom+0.4f;
	m_Scale /= zoomfactor*zoomfactor;
	m_Radius = 0.49f*m_Height*m_Scale;
	m_FogStart = 0.35f*m_Scale;
	m_FogEnd = 0.1f*m_Scale;

	// Globus zeichnen
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(DISTANCE, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);

	glRotatef(m_AngleY, 0.0f, 1.0f, 0.0f);
	glRotatef(m_AngleZ, 0.0f, 0.0f, 1.0f);
	glScalef(m_Scale, m_Scale, m_Scale);

	glEnable(GL_FOG);
	glFogf(GL_FOG_START, DISTANCE-m_FogStart);
	glFogf(GL_FOG_END, DISTANCE-m_FogEnd);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	if (m_TextureGlobe)
		glBindTexture(GL_TEXTURE_2D, m_TextureGlobe->GetID());
	glCallList(m_GlobeList[theApp.m_GlobeHQModel]);

	if (m_Locations)
	{
		// Punkte und Schilder zeichnen
		CalcAndDrawPoints();
		glDisable(GL_FOG);

		// Label
		if (m_ViewParameters.GlobeShowBubbles)
			CalcAndDrawLabel();
	}

	// Statuszeile
	if (m_Height>=STATUSBAR_HEIGHT)
	{
		glEnable2D();

		// Kante
		glColor4d(backcol[0], backcol[1], backcol[2], 0.8f);
		glBegin(GL_LINES);
		glVertex2i(0, m_Height-STATUSBAR_HEIGHT);
		glVertex2i(m_Width, m_Height-STATUSBAR_HEIGHT);
		glEnd();

		// Füllen
		glColor4d(backcol[0], backcol[1], backcol[2], 0.65f);
		glRecti(0, m_Height-STATUSBAR_HEIGHT, m_Width, m_Height);

		// Text
		GLfloat highlightcol[4];
		ColorRef2GLColor(highlightcol, m_ColorHighlight);
		glColor4d(highlightcol[0], highlightcol[1], highlightcol[2], 1.0f);

		wchar_t Copyright[] = L"© NASA's Earth Observatory";
		m_pSpecialFont->Render(&Copyright[0],
			(float)((m_Width-m_pSpecialFont->GetTextWidth(&Copyright[0]))>>1),
			m_Height-16.0f);

		glDisable2D();
	}

	glFinish();
	SwapBuffers(m_pDC->GetSafeHdc());

	m_LockUpdate = FALSE;
}

void CGlobeView::CalcAndDrawPoints()
{
	GLdouble modelview[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

	for (UINT a=0; a<result->m_ItemCount; a++)
		if (m_Locations[a].valid)
		{
			m_Locations[a].alpha = 0.0f;

			// double x = modelview[0] * m_Locations[a].world[0] + modelview[4] * m_Locations[a].world[1] + modelview[8] * m_Locations[a].world[2];
			// double y = modelview[0+1] * m_Locations[a].world[0] + modelview[4+1] * m_Locations[a].world[1] + modelview[8+1] * m_Locations[a].world[2];
			double z = modelview[0+2] * m_Locations[a].world[0] + modelview[4+2] * m_Locations[a].world[1] + modelview[8+2] * m_Locations[a].world[2];

			if (z>m_FogEnd)
			{
				// Feedback
				GLfloat buffer[4];
				glFeedbackBuffer(4, GL_3D,buffer);

				glRenderMode(GL_FEEDBACK);
				glBegin(GL_POINTS);
				glVertex3dv(m_Locations[a].world);
				glEnd();
				if (glRenderMode(GL_RENDER)>0)
				{
					m_Locations[a].screenpoint[0] = (int)(buffer[1]+0.5f);
					m_Locations[a].screenpoint[1] = m_Height-(int)(buffer[2]+0.5f);

					m_Locations[a].alpha = 1.0f;
					GLfloat psize = 9.5f;
					if (z<m_FogStart)
					{
						m_Locations[a].alpha -= (GLfloat)((m_FogStart-z)/(m_FogStart-m_FogEnd));
						psize *= max(m_Locations[a].alpha, 0.25f);
					}

					// Weiß
					glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
					glPointSize(psize);
					glBegin(GL_POINTS);
					glVertex3dv(m_Locations[a].world);
					glEnd();

					// Rot
					glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
					glPointSize(psize*0.66f);
					glBegin(GL_POINTS);
					glVertex3dv(m_Locations[a].world);
					glEnd();
				}
			}
		}
}

void CGlobeView::CalcAndDrawLabel()
{
	GLfloat labelcol[4];
	ColorRef2GLColor(labelcol, GetSysColor(COLOR_3DFACE));
	GLfloat textcol[4];
	ColorRef2GLColor(textcol, GetSysColor(COLOR_WINDOWTEXT));

	glEnable2D();

	for (UINT a=0; a<result->m_ItemCount; a++)
		if (m_Locations[a].valid)
			if (m_Locations[a].alpha>0.0f)
			{
				// Beschriftung
				wchar_t* caption = result->m_Items[a]->CoreAttributes.FileName;
				UINT cCaption = (UINT)wcslen(caption);
				wchar_t* subcaption = NULL;
				wchar_t* coordinates = (m_ViewParameters.GlobeShowGPS ? m_Locations[a].coordstring : NULL);
				wchar_t* description = (m_ViewParameters.GlobeShowHints ? result->m_Items[a]->Description : NULL);
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
					if ((wcscmp(caption, m_Locations[a].coordstring)==0) && (m_ViewParameters.GlobeShowGPS))
						coordinates = NULL;
					break;
				}

				DrawLabel(&m_Locations[a], cCaption, caption, subcaption, coordinates, description, FocusItem==(int)a);
			}

	glDisable2D();
}

void CGlobeView::DrawLabel(Location* loc, UINT cCaption, wchar_t* caption, wchar_t* subcaption, wchar_t* coordinates, wchar_t* description, BOOL focused)
{
	ASSERT(ARROWSIZE>3);

	COLORREF BaseColorRef = GetSysColor(COLOR_WINDOW);
	COLORREF TextColorRef = GetSysColor(COLOR_WINDOWTEXT);
	if (loc->selected)
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
	UINT width = LargeFont->GetTextWidth(caption, cCaption);
	width = max(width, SmallFont->GetTextWidth(subcaption));
	width = max(width, SmallFont->GetTextWidth(coordinates));
	width = max(width, SmallFont->GetTextWidth(description));
	width += 8;

	// Höhe
	UINT height = LargeFont->GetTextHeight(caption);
	height += SmallFont->GetTextHeight(subcaption);
	height += SmallFont->GetTextHeight(coordinates);
	height += SmallFont->GetTextHeight(description);
	height += 3;

	// Position
	int top = (loc->screenpoint[1]<m_Height/2 ? -1 : 1);

	loc->screenlabel[0] = loc->screenpoint[0]-ARROWSIZE-((width-2*ARROWSIZE)*(m_Width-loc->screenpoint[0])/m_Width);
	loc->screenlabel[1] = loc->screenpoint[1]+(ARROWSIZE-2)*top-(top<0 ? height : 0);
	loc->screenlabel[2] = loc->screenlabel[0]+width;
	loc->screenlabel[3] = loc->screenlabel[1]+height;

	GLfloat x = (GLfloat)loc->screenlabel[0];
	GLfloat y = (GLfloat)loc->screenlabel[1];

	// Schatten
	for (int s=3; s>0; s--)
	{
		glColor4f(0.0f, 0.0f, 0.0f, loc->alpha/(s+2.5f));
		glBegin(GL_LINE_STRIP);
		glVertex2f(x+2.5f, y+height+s+0.5f);
		glVertex2f(x+width+s+0.5f, y+height+s+0.5f);
		glVertex2f(x+width+s+0.5f, y+2.5f);
		glEnd();
	}

	glColor4f(0.0f, 0.0f, 0.0f, loc->alpha/2.5f);
	glBegin(GL_LINES);
	glVertex2f(x+width+0.5f, y+height+0.5f);
	glVertex2f(x+width+1.5f, y+height+0.5f);

	// Grauer Rand
	glColor4f(BaseColor[0]/2, BaseColor[1]/2, BaseColor[2]/2, loc->alpha);
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0xFFFF);
	glVertex2f(x+0.5f, y-0.5f);				// Oben
	glVertex2f(x+width+0.25f, y-0.5f);
	glVertex2f(x+0.5f, y+height+0.5f);		// Unten
	glVertex2f(x+width+0.25f, y+height+0.5f);
	glVertex2f(x-0.5f, y);					// Links
	glVertex2f(x-0.5f, y+height);
	glVertex2f(x+width+0.5f, y);			// Rechts
	glVertex2f(x+width+0.5f, y+height);
	glDisable(GL_LINE_STIPPLE);
	glEnd();

	// Pfeil
	glBegin(GL_TRIANGLES);
	GLfloat alpha = pow(loc->alpha, 3);
	for (int a=0; a<=3; a++)
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
			glColor4f(BaseColor[0], BaseColor[1], BaseColor[2], loc->alpha);
		}
		glVertex2f(loc->screenpoint[0]+0.5f, (GLfloat)(loc->screenpoint[1]+(a-2)*top));
		glVertex2f(loc->screenpoint[0]+0.5f+(ARROWSIZE-a)*top, (GLfloat)(loc->screenpoint[1]+(ARROWSIZE-2)*top));
		glVertex2f(loc->screenpoint[0]+0.5f-(ARROWSIZE-a)*top, (GLfloat)(loc->screenpoint[1]+(ARROWSIZE-2)*top));
	}
	glEnd();

	// Innen
	glRectf(x, y, x+width, y+height);
	if (focused)
		if (this==GetFocus())
		{
			glColor4f(1.0f-BaseColor[0], 1.0f-BaseColor[1], 1.0f-BaseColor[2], loc->alpha);
			glEnable(GL_LINE_STIPPLE);
			glLineStipple(1, 0xAAAA);
			glBegin(GL_LINE_LOOP);
			glVertex2f(x+0.5f, y+0.5f);
			glVertex2f(x+width-0.5f, y+0.5f);
			glVertex2f(x+width-0.5f, y+height-0.5f);
			glVertex2f(x+0.5f, y+height-0.5f);
			glEnd();
			glDisable(GL_LINE_STIPPLE);
		}

	x += 3.0f;

	glColor4f(TextColor[0], TextColor[1], TextColor[2], loc->alpha);
	y += LargeFont->Render(caption, x, y, cCaption);
	if (subcaption)
		y += SmallFont->Render(subcaption, x, y);

	if ((!loc->selected) && (BaseColorRef==0xFFFFFF) && (TextColorRef==0x000000))
		glColor4f(TextColor[0], TextColor[1], TextColor[2], loc->alpha/2);
	if (coordinates)
		y += SmallFont->Render(coordinates, x, y);
	if (description)
		y += SmallFont->Render(description, x, y);
}
