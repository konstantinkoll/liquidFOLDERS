
// CGlobeView.cpp: Implementierung der Klasse CGlobeView
//

#include "stdafx.h"
#include "CGlobeView.h"
#include "Resource.h"
#include "LFCore.h"
#include <cmath>

#define STATUSBAR_HEIGHT 12
#define DISTANCE 39.0f
#define ARROWSIZE 9
#define PI 3.14159265358979323846
#define ANIMLENGTH 200

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

static inline void MatrixMul(GLdouble Result[4][4], GLdouble Left[4][4], GLdouble Right[4][4])
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
	hCursor = LoadCursor(NULL, IDC_WAIT);
	m_TextureGlobe = NULL;
	m_Width = 0;
	m_Height = 0;
	m_GlobeList[FALSE] = -1;
	m_GlobeList[TRUE] = -1;
	m_Latitude = 0.0f;
	m_Longitude = 0.0f;
	m_Zoom = -1.0f;
	m_Scale = 0.7f;
	m_Radius = 0.0f;
	m_Grabbed = FALSE;
	m_Locations = NULL;
	m_CameraChanged = FALSE;
	lpszCursorName = NULL;
	m_CursorPos.x = 0;
	m_CursorPos.y = 0;
	m_AnimCounter = 0;
	m_nTexture = -1;
	ENSURE(YouLookAt.LoadString(IDS_YOULOOKAT));
	m_LockUpdate = FALSE;
}

CGlobeView::~CGlobeView()
{
	if (m_Locations)
		delete[] m_Locations;
}

void CGlobeView::Create(CWnd* _pParentWnd, LFSearchResult* _result, int _FocusItem)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC, hCursor);

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	CRect rect;
	rect.SetRectEmpty();
	CWnd::Create(className, _T(""), dwStyle, rect, _pParentWnd, AFX_IDW_PANE_FIRST);

	CFileView::Create(_result, LFViewGlobe, _FocusItem, FALSE, FALSE);
}

void CGlobeView::SetViewOptions(UINT /*_ViewID*/, BOOL Force)
{
	if (Force)
	{
		m_LocalSettings.Latitude = pViewParameters->GlobeLatitude/1000.0f;
		m_LocalSettings.Longitude = pViewParameters->GlobeLongitude/1000.0f;
		m_LocalSettings.GlobeZoom = pViewParameters->GlobeZoom;
	}

	if (Force || (theApp.m_nAppLook!=RibbonColor))
		theApp.GetRibbonColors(&m_ColorBack, &m_ColorText, &m_ColorHighlight);

	PrepareTexture();
	PrepareModel(theApp.m_GlobeHQModel);

	m_ViewParameters = *pViewParameters;
	UpdateScene(TRUE);
}

void CGlobeView::SetSearchResult(LFSearchResult* _result)
{
	UINT VictimCount = result ? result->m_ItemCount : 0;
	Location* Victim = m_Locations;
	m_Locations = NULL;

	result = _result;
	if (_result)
		if (_result->m_ItemCount)
		{
			m_Locations = new Location[_result->m_ItemCount];
			int FirstItem = -1;

			// Compute locations
			for (UINT a=0; a<_result->m_ItemCount; a++)
			{
				ZeroMemory(&m_Locations[a], sizeof(Location));
				if (a<VictimCount)
					m_Locations[a].selected = Victim[a].selected;

				LFGeoCoordinates coord = { 0.0, 0.0 };
				if (m_ViewParameters.SortBy==LFAttrLocationIATA)
				{
					LFAirport* airport;
					if (LFIATAGetAirportByCode((char*)_result->m_Items[a]->AttributeValues[LFAttrLocationIATA], &airport))
						coord = airport->Location;
				}
				else
					if (_result->m_Items[a]->AttributeValues[m_ViewParameters.SortBy])
						coord = *((LFGeoCoordinates*)_result->m_Items[a]->AttributeValues[m_ViewParameters.SortBy]);

				if ((coord.Latitude!=0.0) || (coord.Longitude!=0))
				{
					CalculateWorldCoords(coord.Latitude, coord.Longitude, m_Locations[a].world);
					LFGeoCoordinatesToString(coord, m_Locations[a].coordstring, 32, false);

					m_Locations[a].valid = TRUE;
					if (FirstItem==-1)
						FirstItem = a;
				}
			}

			// Set focus
			if (!m_Locations[FocusItem].valid)
				for (UINT a=(UINT)FocusItem; a<_result->m_ItemCount; a++)
					if (m_Locations[a].valid)
					{
						FocusItem = a;
						break;
					}
			if (!m_Locations[FocusItem].valid)
				FocusItem = FirstItem;
		}

	if (Victim)
		delete[] Victim;

	UpdateScene(TRUE);
}

void CGlobeView::SelectItem(int n, BOOL select, BOOL InternalCall)
{
	if (m_Locations)
	{
		m_Locations[n].selected = select;

		if (!InternalCall)
		{
			DrawScene(TRUE);
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
	ON_COMMAND(ID_GLOBE_LIGHTING, OnLighting)
	ON_COMMAND(ID_GLOBE_SHOWBUBBLES, OnShowBubbles)
	ON_COMMAND(ID_GLOBE_SHOWAIRPORTNAMES, OnShowAirportNames)
	ON_COMMAND(ID_GLOBE_SHOWGPS, OnShowGPS)
	ON_COMMAND(ID_GLOBE_SHOWHINTS, OnShowHints)
	ON_COMMAND(ID_GLOBE_SHOWSPOTS, OnShowSpots)
	ON_COMMAND(ID_GLOBE_SHOWVIEWPOINT, OnShowViewpoint)
	ON_UPDATE_COMMAND_UI_RANGE(ID_GLOBE_ZOOMIN, ID_GLOBE_SHOWVIEWPOINT, OnUpdateCommands)
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
		hCursor = LoadCursor(NULL, csr);
		lpszCursorName = csr;
	}

	SetCursor(hCursor);
}

int CGlobeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
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
		wglMakeCurrent(m_pDC->GetSafeHdc(), m_hrc);

		if (m_TextureGlobe)
			delete m_TextureGlobe;
		if (m_GlobeList[FALSE]!=-1)
			glDeleteLists(m_GlobeList[FALSE], 1);
		if (m_GlobeList[TRUE]!=-1)
			glDeleteLists(m_GlobeList[TRUE], 1);

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
	pViewParameters->GlobeLatitude = (int)(m_LocalSettings.Latitude*1000.0f);
	pViewParameters->GlobeLongitude = (int)(m_LocalSettings.Longitude*1000.0f);
	pViewParameters->GlobeZoom = m_LocalSettings.GlobeZoom;
	m_CameraChanged = FALSE;

	// Kein Broadcase an andere Fenster
}

void CGlobeView::OnJumpToLocation()
{
	LFSelectLocationIATADlg dlg(this, IDD_JUMPTOIATA);

	if (dlg.DoModal()==IDOK)
	{
		ASSERT(dlg.m_Airport);
		m_AnimCounter = ANIMLENGTH;
		m_AnimStartLatitude = m_Latitude;
		m_AnimStartLongitude = m_Longitude;
		m_LocalSettings.Latitude = (GLfloat)-dlg.m_Airport->Location.Latitude;
		m_LocalSettings.Longitude = (GLfloat)-dlg.m_Airport->Location.Longitude;
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
	theApp.UpdateViewOptions(ActiveContextID);
}

void CGlobeView::OnLighting()
{
	theApp.m_GlobeLighting = !theApp.m_GlobeLighting;
	theApp.UpdateViewOptions(ActiveContextID);
}

void CGlobeView::OnShowBubbles()
{
	pViewParameters->GlobeShowBubbles = !pViewParameters->GlobeShowBubbles;
	if (!pViewParameters->GlobeShowBubbles)
		pViewParameters->GlobeShowSpots = TRUE;

	theApp.UpdateViewOptions(ActiveContextID);
}

void CGlobeView::OnShowAirportNames()
{
	pViewParameters->GlobeShowAirportNames = !pViewParameters->GlobeShowAirportNames;
	theApp.UpdateViewOptions(ActiveContextID);
}

void CGlobeView::OnShowGPS()
{
	pViewParameters->GlobeShowGPS = !pViewParameters->GlobeShowGPS;
	theApp.UpdateViewOptions(ActiveContextID);
}

void CGlobeView::OnShowHints()
{
	pViewParameters->GlobeShowHints = !pViewParameters->GlobeShowHints;
	theApp.UpdateViewOptions(ActiveContextID);
}

void CGlobeView::OnShowSpots()
{
	pViewParameters->GlobeShowSpots = !pViewParameters->GlobeShowSpots;
	if (!pViewParameters->GlobeShowSpots)
		pViewParameters->GlobeShowBubbles = TRUE;

	theApp.UpdateViewOptions(ActiveContextID);
}

void CGlobeView::OnShowViewpoint()
{
	pViewParameters->GlobeShowViewpoint = !pViewParameters->GlobeShowViewpoint;
	theApp.UpdateViewOptions(ActiveContextID);
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
	case ID_GLOBE_LIGHTING:
		pCmdUI->SetCheck(theApp.m_GlobeLighting);
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
		break;
	case ID_GLOBE_SHOWSPOTS:
		pCmdUI->SetCheck(m_ViewParameters.GlobeShowSpots);
		break;
	case ID_GLOBE_SHOWVIEWPOINT:
		pCmdUI->SetCheck(m_ViewParameters.GlobeShowViewpoint);
		break;
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
			if (m_AnimCounter)
			{
				m_AnimCounter = 0;
				m_LocalSettings.Latitude = m_Latitude;
				m_LocalSettings.Longitude = m_Longitude;
				m_LocalSettings.GlobeZoom = (int)(m_Zoom*100.f);
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
		m_LocalSettings.Longitude = m_Longitude -= rotate.cx/m_Scale*0.12f;
		m_LocalSettings.Latitude = m_Latitude -= rotate.cy/m_Scale*0.12f;
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
	glTranslatef (0.375, 0.375, 0);

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

	// Fonts
	m_Fonts[FALSE].Create(&theApp.m_DefaultFont);
	m_Fonts[TRUE].Create(&theApp.m_CaptionFont);
	m_SpecialFont.Create(&theApp.m_SmallFont);
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

	if ((int)tex!=m_nTexture)
	{
		SetCursor(LoadCursor(NULL, IDC_WAIT));

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

void CGlobeView::Normalize()
{
	// Zoom
	if (m_LocalSettings.GlobeZoom<0)
		m_LocalSettings.GlobeZoom = 0;
	if (m_LocalSettings.GlobeZoom>100)
		m_LocalSettings.GlobeZoom = 100;

	// Nicht über die Pole rollen
	if (m_LocalSettings.Latitude<-75.0f)
		m_LocalSettings.Latitude = -75.0f;
	if (m_LocalSettings.Latitude>75.0f)
		m_LocalSettings.Latitude = 75.0f;

	// Rotation normieren
	if (m_LocalSettings.Longitude<0.0f)
		m_LocalSettings.Longitude += 360.0f;
	if (m_LocalSettings.Longitude>360.0f)
		m_LocalSettings.Longitude -= 360.0f;
}

BOOL CGlobeView::UpdateScene(BOOL Redraw)
{
	if (m_LockUpdate)
		return FALSE;
	m_LockUpdate = TRUE;

	BOOL res = Redraw;
	Normalize();

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
			m_Zoom += 0.005f;
		}
		if (m_Zoom>TargetZoom+0.009f)
		{
			res = TRUE;
			m_Zoom -= 0.005f;
		}
	}

	if ((m_Latitude==0.0f) && (m_Longitude==0.0f))
	{
		res |= (m_Latitude!=m_LocalSettings.Latitude) || (m_Longitude!=m_LocalSettings.Longitude);
		m_Latitude = m_LocalSettings.Latitude;
		m_Longitude = m_LocalSettings.Longitude;
	}
	else
	{
		if (m_AnimCounter)
		{
			GLfloat f = (GLfloat)((cos(PI*m_AnimCounter/ANIMLENGTH)+1.0)/2.0);
			m_Latitude = m_AnimStartLatitude*(1.0f-f) + m_LocalSettings.Latitude*f;
			m_Longitude = m_AnimStartLongitude*(1.0f-f) + m_LocalSettings.Longitude*f;

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
			res |= (m_Latitude!=m_LocalSettings.Latitude) || (m_Longitude!=m_LocalSettings.Longitude);
			m_Latitude = m_LocalSettings.Latitude;
			m_Longitude = m_LocalSettings.Longitude;
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

	if (theApp.m_GlobeLighting)
	{
		GLfloat lAmbient[] = { 0.9f, 0.9f, 0.9f, 1.0f };
		GLfloat lDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat lSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glLightfv(GL_LIGHT0, GL_AMBIENT, lAmbient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lDiffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, lSpecular);

		GLfloat LightPosition[3];
		LightPosition[0] = 100.0f;
		LightPosition[1] = 0.0f;
		LightPosition[2] = 0.0f;
		glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);

		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHTING);
		glEnable(GL_NORMALIZE);

		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lAmbient);
	}

	glRotatef(m_Latitude, 0.0f, 1.0f, 0.0f);
	glRotatef(m_Longitude, 0.0f, 0.0f, 1.0f);
	glScalef(m_Scale, m_Scale, m_Scale);

	glEnable(GL_FOG);
	glFogf(GL_FOG_START, DISTANCE-m_FogStart);
	glFogf(GL_FOG_END, DISTANCE-m_FogEnd);

	if (m_TextureGlobe)
	{
		glBindTexture(GL_TEXTURE_2D, m_TextureGlobe->GetID());
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, theApp.m_GlobeLighting ? GL_MODULATE : GL_REPLACE);
	}

	glCallList(m_GlobeList[theApp.m_GlobeHQModel]);

	if (theApp.m_GlobeLighting)
	{
		glDisable(GL_NORMALIZE);
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	}

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
		wchar_t Copyright[] = L"© NASA's Earth Observatory";
		int CopyrightX = -1;
		UINT CopyrightWidth = m_SpecialFont.GetTextWidth(Copyright);

		if (m_Width>=(int)CopyrightWidth)
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

			wchar_t Viewpoint[256];
			int ViewpointX = -1;
			if (m_ViewParameters.GlobeShowViewpoint)
			{
				wchar_t Coord[256];
				LFGeoCoordinates c;
				c.Latitude = -m_Latitude;
				c.Longitude = (m_Longitude>180.0) ? 360-m_Longitude : -m_Longitude;
				LFGeoCoordinatesToString(c, Coord, 256, true);

				swprintf(Viewpoint, 256, YouLookAt, Coord);
				UINT ViewpointWidth = m_SpecialFont.GetTextWidth(Viewpoint);

				if (m_Width>=(int)(CopyrightWidth+ViewpointWidth+60))
				{
					UINT Spare = m_Width-CopyrightWidth-ViewpointWidth;
					CopyrightX = Spare/3;
					ViewpointX = m_Width-ViewpointWidth-Spare/3;
				}
			}

			if (CopyrightX==-1)
				CopyrightX = (m_Width-m_SpecialFont.GetTextWidth(&Copyright[0]))>>1;

			// Text
			GLfloat highlightcol[4];
			ColorRef2GLColor(highlightcol, m_ColorHighlight);
			glColor4d(highlightcol[0], highlightcol[1], highlightcol[2], 1.0f);

			m_SpecialFont.Render(Copyright, CopyrightX, m_Height-STATUSBAR_HEIGHT);
			if (ViewpointX!=-1)
				m_SpecialFont.Render(Viewpoint, ViewpointX, m_Height-STATUSBAR_HEIGHT);

			glDisable2D();
		}
	}

	glFinish();
	SwapBuffers(m_pDC->GetSafeHdc());

	m_LockUpdate = FALSE;
}

void CGlobeView::CalcAndDrawPoints()
{
	GLdouble modelview[4][4];
	glGetDoublev(GL_MODELVIEW_MATRIX, &modelview[0][0]);

	GLdouble projection[4][4];
	glGetDoublev(GL_PROJECTION_MATRIX, &projection[0][0]);

	GLdouble mvp[4][4];
	MatrixMul(mvp, modelview, projection);

	GLdouble szx = m_Width/2.0;
	GLdouble szy = m_Height/2.0;

	for (UINT a=0; a<result->m_ItemCount; a++)
		if (m_Locations[a].valid)
		{
			m_Locations[a].alpha = 0.0f;

			GLdouble z = modelview[0][2]*m_Locations[a].world[0] + modelview[1][2]*m_Locations[a].world[1] + modelview[2][2]*m_Locations[a].world[2];
			if ((z>m_FogEnd) && (m_Width) && (m_Height))
			{
				GLdouble w = mvp[0][3]*m_Locations[a].world[0] + mvp[1][3]*m_Locations[a].world[1] + mvp[2][3]*m_Locations[a].world[2] + mvp[3][3];
				GLdouble x = (mvp[0][0]*m_Locations[a].world[0] + mvp[1][0]*m_Locations[a].world[1] + mvp[2][0]*m_Locations[a].world[2] + mvp[3][0])*szx/w;
				GLdouble y = -(mvp[0][1]*m_Locations[a].world[0] + mvp[1][1]*m_Locations[a].world[1] + mvp[2][1]*m_Locations[a].world[2] + mvp[3][1])*szy/w;

				m_Locations[a].screenpoint[0] = (int)(x+szx+0.5f);
				m_Locations[a].screenpoint[1] = (int)(y+szy+0.5f);

				m_Locations[a].alpha = 1.0f;
				GLfloat psize = 9.5f;
				if (z<m_FogStart)
				{
					m_Locations[a].alpha -= (GLfloat)((m_FogStart-z)/(m_FogStart-m_FogEnd));
					psize *= max(m_Locations[a].alpha, 0.25f);
				}

				if (m_ViewParameters.GlobeShowSpots)
				{
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
	UINT width = m_Fonts[TRUE].GetTextWidth(caption, cCaption);
	width = max(width, m_Fonts[FALSE].GetTextWidth(subcaption));
	width = max(width, m_Fonts[FALSE].GetTextWidth(coordinates));
	width = max(width, m_Fonts[FALSE].GetTextWidth(description));
	width += 8;

	// Höhe
	UINT height = m_Fonts[TRUE].GetTextHeight(caption);
	height += m_Fonts[FALSE].GetTextHeight(subcaption);
	height += m_Fonts[FALSE].GetTextHeight(coordinates);
	height += m_Fonts[FALSE].GetTextHeight(description);
	height += 3;

	// Position
	int top = (loc->screenpoint[1]<m_Height/2 ? -1 : 1);

	loc->screenlabel[0] = loc->screenpoint[0]-ARROWSIZE-((width-2*ARROWSIZE)*(m_Width-loc->screenpoint[0])/m_Width);
	loc->screenlabel[1] = loc->screenpoint[1]+(ARROWSIZE-2)*top-(top<0 ? height : 0);
	loc->screenlabel[2] = loc->screenlabel[0]+width;
	loc->screenlabel[3] = loc->screenlabel[1]+height;

	int x = loc->screenlabel[0];
	int y = loc->screenlabel[1];

	// Schatten
	for (int s=3; s>0; s--)
	{
		glColor4f(0.0f, 0.0f, 0.0f, loc->alpha/(s+2.5f));
		glBegin(GL_LINES);
		glVertex2i(x+2, y+height+s);
		glVertex2i(x+width+s, y+height+s);
		glVertex2i(x+width+s, y+2);
		glVertex2i(x+width+s, y+height+s+1);
		glEnd();
	}

	glColor4f(0.0f, 0.0f, 0.0f, loc->alpha/2.5f);
	glBegin(GL_LINES);
	glVertex2i(x+width, y+height);
	glVertex2i(x+width+1, y+height);

	// Grauer Rand
	glColor4f(BaseColor[0]/2, BaseColor[1]/2, BaseColor[2]/2, loc->alpha);
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

		glVertex2i(loc->screenpoint[0], loc->screenpoint[1]+(a-2)*top);
		glVertex2i(loc->screenpoint[0]+(ARROWSIZE-a)*top, loc->screenpoint[1]+(ARROWSIZE-2)*top);
		glVertex2i(loc->screenpoint[0]-(ARROWSIZE-a)*top, loc->screenpoint[1]+(ARROWSIZE-2)*top);
	}
	glEnd();

	// Innen
	glRecti(x, y, x+width, y+height);
	if ((focused) && (GetFocus()==this))
	{
		glColor4f(1.0f-BaseColor[0], 1.0f-BaseColor[1], 1.0f-BaseColor[2], loc->alpha);
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

	glColor4f(TextColor[0], TextColor[1], TextColor[2], loc->alpha);
	y += m_Fonts[TRUE].Render(caption, x, y, cCaption);
	if (subcaption)
		y += m_Fonts[FALSE].Render(subcaption, x, y);

	if ((!loc->selected) && (BaseColorRef==0xFFFFFF) && (TextColorRef==0x000000))
		glColor4f(TextColor[0], TextColor[1], TextColor[2], loc->alpha/2);
	if (coordinates)
		y += m_Fonts[FALSE].Render(coordinates, x, y);
	if (description)
		y += m_Fonts[FALSE].Render(description, x, y);
}
