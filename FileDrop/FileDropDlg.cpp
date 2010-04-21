// FileDropDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FileDrop.h"
#include "FileDropDlg.h"
#include "Resource.h"
#include "LFCore.h"
#include <afxwin.h>
#include <io.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <dwmapi.h>


// Aero Glass
static HMODULE hModAero = NULL;

BOOL AeroLibLoaded = FALSE;
BOOL Glassed = FALSE;

typedef HRESULT(__stdcall *PFNDWMISCOMPOSITIONENABLED)(BOOL* pfEnabled);
typedef HRESULT(__stdcall *PFNDWMEXTENDFRAMEINTOCLIENTAREA)(HWND hWnd, const MARGINS* pMarInset);

PFNDWMISCOMPOSITIONENABLED zDwmIsCompositionEnabled = NULL;
PFNDWMEXTENDFRAMEINTOCLIENTAREA zDwmExtendFrameIntoClientArea = NULL;


void InitAero()
{
	hModAero = LoadLibrary(_T("DWMAPI.DLL"));
	if (hModAero)
	{
		zDwmIsCompositionEnabled = (PFNDWMISCOMPOSITIONENABLED)GetProcAddress(hModAero, "DwmIsCompositionEnabled");
		zDwmExtendFrameIntoClientArea = (PFNDWMEXTENDFRAMEINTOCLIENTAREA)GetProcAddress(hModAero, "DwmExtendFrameIntoClientArea");

		AeroLibLoaded = (zDwmIsCompositionEnabled && zDwmExtendFrameIntoClientArea);
		if (!AeroLibLoaded)
		{
			FreeLibrary(hModAero);
			hModAero = NULL;
		}
	}
}

void UseGlassBackground(HWND m_hWnd)
{
	Glassed = FALSE;
	zDwmIsCompositionEnabled(&Glassed);

	if (Glassed)
	{
		MARGINS margins = { -1 };
		zDwmExtendFrameIntoClientArea(m_hWnd, &margins);
	}
}

void DoneAero()
{
	FreeLibrary(hModAero);
	hModAero = NULL;
	AeroLibLoaded = FALSE;
}


// CFileDropDlg-Dialogfeld

CFileDropDlg::CFileDropDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_FILEDROP_DIALOG, pParent)
{
	m_hIcon = theApp.LoadIcon(IDR_APPLICATION);
	dropzoneL = NULL;
	dropzoneS = NULL;
	ready = NULL;
	warning = NULL;
	TimerID = 0;
	Themed = FALSE;
}

BEGIN_MESSAGE_MAP(CFileDropDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_ACTIVATE()
	ON_WM_QUERYDRAGICON()
	ON_WM_THEMECHANGED()
	ON_WM_DWMCOMPOSITIONCHANGED()
	ON_WM_NCRBUTTONDOWN()
	ON_WM_NCHITTEST()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_COMMAND(IDM_ALWAYSONTOP, OnAlwaysOnTop)
	ON_COMMAND(IDM_SMALLWINDOW, OnSmallWindow)
	ON_COMMAND(IDM_STOREMANAGER, theApp.OnAppNewStoreManager)
	ON_COMMAND(IDM_ABOUT, OnAbout)
	ON_COMMAND(IDM_CHOOSEDEFAULTSTORE, OnChooseDefaultStore)
END_MESSAGE_MAP()


// CFileDropDlg-Meldungshandler

BOOL CFileDropDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Hintergrundbilder laden
	dropzoneL = new CGdiPlusBitmapResource();
	dropzoneL->Load(IDB_DROPZONELARGE, _T("PNG"), AfxGetInstanceHandle());
	dropzoneS = new CGdiPlusBitmapResource();
	dropzoneS->Load(IDB_DROPZONESMALL, _T("PNG"), AfxGetInstanceHandle());

	// Badges laden
	ready = new CGdiPlusBitmapResource();
	ready->Load(IDB_BADGE_READY, _T("PNG"), AfxGetInstanceHandle());
	warning = new CGdiPlusBitmapResource();
	warning->Load(IDB_BADGE_WARNING, _T("PNG"), AfxGetInstanceHandle());

	// Einstellungen laden
	SetTopMost(theApp.GetProfileInt(_T(""), _T("AlwaysOnTop"), 1)==1);
	SetWindowSize(theApp.GetProfileInt(_T(""), _T("SmallWindow"), 0)==1);

	UpdateStatus();

	CRect r;
	GetWindowRect(&r);
	int x = theApp.GetProfileInt(_T(""), _T("X"), 10000);
	int y = theApp.GetProfileInt(_T(""), _T("Y"), 10000);

	CRect d;
	GetDesktopWindow()->GetWindowRect(&d);
	if (x<-1)
		x = -1;
	if (x+r.Width()>d.Width()+2)
		x = d.Width()-r.Width()+2;
	if (y<-1)
		y = -1;
	if (y+r.Height()>d.Height()+2)
		y = d.Height()-r.Height()+2;
	r.left = x;
	r.top = y;
	MoveWindow(x, y, x+r.Width(), y+r.Height());

	// IDM_xxx muss sich im Bereich der Systembefehle befinden.
	ASSERT((IDM_ALWAYSONTOP & 0xFFF0) == IDM_ALWAYSONTOP);
	ASSERT(IDM_ALWAYSONTOP < 0xF000);
	ASSERT((IDM_SMALLWINDOW & 0xFFF0) == IDM_SMALLWINDOW);
	ASSERT(IDM_SMALLWINDOW < 0xF000);
	ASSERT((IDM_ABOUT & 0xFFF0) == IDM_ABOUT);
	ASSERT(IDM_ABOUT < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		// Always on top
		CString strAlwaysOnTop;
		ENSURE(strAlwaysOnTop.LoadString(IDS_ALWAYSONTOP));
		if (!strAlwaysOnTop.IsEmpty())
			pSysMenu->InsertMenu(SC_CLOSE, MF_STRING | MF_BYCOMMAND | (AlwaysOnTop ? MF_CHECKED:0), IDM_ALWAYSONTOP, strAlwaysOnTop);

		// Small window
		CString strSmallWindow;
		ENSURE(strSmallWindow.LoadString(IDS_SMALLWINDOW));
		if (!strSmallWindow.IsEmpty())
			pSysMenu->InsertMenu(SC_CLOSE, MF_STRING | MF_BYCOMMAND | (SmallWindow ? MF_CHECKED:0), IDM_SMALLWINDOW, strSmallWindow);

		// About
		ENSURE(strAbout.LoadString(IDS_ABOUT));
		if (!strAbout.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUT, strAbout);
		}
	}

	// Hinweistext laden
	ENSURE(strHint.LoadString(IDS_HINT));

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	SetIcon(m_hIcon, TRUE);
	SetIcon((HICON)LoadImage(NULL, MAKEINTRESOURCE(IDR_APPLICATION), IMAGE_ICON, 16, 16, LR_LOADTRANSPARENT), FALSE);

	// XP Themes
	if (theApp.m_ThemeLibLoaded)
	{
		hTheme = theApp.zOpenThemeData(m_hWnd, VSCLASS_WINDOW);
		Themed = (hTheme!=NULL);
	}

	// Aero Glass
	InitAero();
	if (AeroLibLoaded)
		UseGlassBackground(m_hWnd);

	// Timer
	TimerID = SetTimer(IDT_UPDATESTATUS, 250, NULL);

	// Initialize Drop
	m_DropTarget.Register ( this );

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void CFileDropDlg::SetTopMost(BOOL TopMost)
{
	AlwaysOnTop = TopMost;
	SetWindowPos(TopMost ? &wndTopMost : &wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
		pSysMenu->CheckMenuItem(IDM_ALWAYSONTOP, MF_BYCOMMAND | (TopMost ? MF_CHECKED : MF_UNCHECKED));
}

void CFileDropDlg::SetWindowSize(BOOL Small)
{
	SmallWindow = Small;
	dropzone = Small ? dropzoneS : dropzoneL;

	CRect r;
	GetWindowRect(&r);
	int x = r.left;
	int y = r.top;
	int l = 130;
	int h = Small ? 160 : 270;

	CRect d;
	GetDesktopWindow()->GetWindowRect(&d);
	if (x+l>d.Width()+2)
		x = d.Width()-l+2;
	if (y+h>d.Height()+2)
		y = d.Height()-h+2;

	SetWindowPos(NULL, x, y, l, h, 0);
	Invalidate();
	UpdateWindow();

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
		pSysMenu->CheckMenuItem(IDM_SMALLWINDOW, MF_BYCOMMAND | (Small ? MF_CHECKED : MF_UNCHECKED));
}

void CFileDropDlg::UpdateStatus()
{
	liquidFOLDERSReady = LFDefaultStoreAvailable();
}

void CFileDropDlg::OnAlwaysOnTop()
{
	SetTopMost(!AlwaysOnTop);
}

void CFileDropDlg::OnSmallWindow()
{
	SetWindowSize(!SmallWindow);
}

void CFileDropDlg::OnAbout()
{
	LFAboutDlgParameters p;
	p.appname = "FileDrop";
	p.build = __TIMESTAMP__;
	p.caption = strAbout;
	p.icon = new CGdiPlusBitmapResource();
	p.icon->Load(IDB_ABOUTICON, _T("PNG"), AfxGetInstanceHandle());
	p.TextureSize = -1;
	p.RibbonColor = ID_VIEW_APPLOOK_OFF_2007_NONE;
	p.AllowEmptyDrives = -1;

	LFAboutDlg dlg(&p, this);
	dlg.DoModal();

	delete p.icon;
}

void CFileDropDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	switch (nID & 0xFFF0)
	{
	case IDM_ALWAYSONTOP:
		OnAlwaysOnTop();
		break;
	case IDM_SMALLWINDOW:
		OnSmallWindow();
		break;
	case IDM_ABOUT:
		OnAbout();
		break;
	default:
		CDialog::OnSysCommand(nID, lParam);
	}
}

// Wenn Sie dem Dialogfeld eine Schaltfläche "Minimieren" hinzufügen, benötigen Sie 
//  den nachstehenden Code, um das Symbol zu zeichnen. Für MFC-Anwendungen, die das 
//  Dokument/Ansicht-Modell verwenden, wird dies automatisch ausgeführt.

void CFileDropDlg::OnPaint()
{
	CPaintDC pDC(this); // Gerätekontext zum Zeichnen

	if (IsIconic())
	{
		OnEraseBkgnd(&pDC);

		// Symbol in Clientrechteck zentrieren
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width()-cxIcon+1) / 2;
		int y = (rect.Height()-cyIcon+1) / 2;

		// Symbol zeichnen
		pDC.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

BOOL CFileDropDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	Graphics g(dc.m_hDC);
	g.SetCompositingMode(CompositingModeSourceOver);

	COLORREF cr;
	if (Glassed)
	{
		cr = 0;
	}
	else
	{
		if (Themed)
		{
			cr = GetSysColor(GetActiveWindow()==this ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION);
		}
		else
		{
			cr = GetSysColor(COLOR_3DFACE);
		}
	}
	g.Clear(Color(cr&0xFF, (cr>>8)&0xFF, (cr>>16)&0xFF));

	int hintHeight;

	if (!SmallWindow)
	{
		const UINT textflags = DT_VCENTER | DT_CENTER | DT_WORDBREAK;
		const UINT htextmargin = 6;
		int vtextmargin;
		CRect* rtext = new CRect(htextmargin, 0, rect.right-htextmargin, 0);

		if ((Glassed) || (Themed))
		{
			LOGFONTW lf;
			theApp.zGetThemeSysFont(hTheme, TMT_CAPTIONFONT, (LOGFONT*)&lf);
			if (lf.lfHeight<-15) // Wichtig für große und extragroße Schriftarten
				lf.lfHeight = -15;

			HFONT titleFont = CreateFontIndirectW(&lf);
			HGDIOBJ oldFontDc = dc.SelectObject(titleFont);

			dc.DrawText(strHint, -1, *rtext, textflags | DT_CALCRECT);
			hintHeight = rtext->bottom;
			vtextmargin = (Glassed ? 0: (rect.Height()-dropzone->m_pBitmap->GetHeight()-hintHeight)/3);
			delete rtext;
			rtext = new CRect(htextmargin, rect.bottom-hintHeight-vtextmargin, rect.right-htextmargin, rect.bottom-vtextmargin);		
			if (rtext->top<0)
			{
				vtextmargin = 0;
				rtext->top = rect.bottom-hintHeight;
				rtext->bottom = rect.bottom;
			}

			if (Glassed)
			{
				MARGINS margins = { 0 };
				margins.cyTopHeight = rect.Height()-hintHeight;
				zDwmExtendFrameIntoClientArea(m_hWnd, &margins);

				CRect rback(0, rect.bottom-hintHeight-vtextmargin, rect.right, rect.bottom-vtextmargin);
				pDC->FillSolidRect(&rback, GetSysColor(COLOR_WINDOW));
				pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));

				HGDIOBJ oldFontPdc = SelectObject(pDC->m_hDC, titleFont);
				pDC->DrawText(strHint, -1, rtext, textflags);
				pDC->SelectObject(oldFontPdc);
			}
			else
			{
				theApp.zDrawThemeText(hTheme, dc, WP_CAPTION, GetActiveWindow()==this ? CS_ACTIVE : CS_INACTIVE,
					strHint, -1, textflags, 0, rtext);
				dc.SetTextColor(GetSysColor(COLOR_CAPTIONTEXT));
				dc.DrawText(strHint, -1, rtext, textflags);
			}

			DeleteObject(titleFont);
			dc.SelectObject(oldFontDc);
			delete rtext;
		}
		else
		{
			HGDIOBJ oldFontDc = SelectObject(dc, GetStockObject(DEFAULT_GUI_FONT));
			dc.DrawText(strHint, -1, rtext, textflags | DT_CALCRECT);
			hintHeight = rtext->bottom;
			vtextmargin = (rect.Height()-dropzone->m_pBitmap->GetHeight()-hintHeight)/3;
			delete rtext;
			rtext = new CRect(htextmargin, rect.bottom-hintHeight-vtextmargin, rect.right-htextmargin, rect.bottom-vtextmargin);
			dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
			dc.DrawText(strHint, -1, rtext, textflags);
			dc.SelectObject(oldFontDc);
			delete rtext;
		}
	}
	else
	{
		if (Glassed)
		{
			MARGINS margins = { -1 };
			zDwmExtendFrameIntoClientArea(m_hWnd, &margins);
		}
		hintHeight = 0;
	}

	g.SetCompositingMode(CompositingModeSourceOver);
	int z = (int)(rect.Height()-dropzone->m_pBitmap->GetHeight()-hintHeight)>>1;
	z = (z<0) ? 0 : z;
	g.DrawImage(dropzone->m_pBitmap, (int)(rect.Width()-dropzone->m_pBitmap->GetWidth())>>1, z);

	CGdiPlusBitmapResource* badge = (liquidFOLDERSReady ? ready : warning);
	g.DrawImage(badge->m_pBitmap,rect.Width()-badge->m_pBitmap->GetWidth()-6,4);

	pDC->BitBlt(0, 0, rect.Width(), rect.Height()-(Glassed ? hintHeight : 0), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
	buffer.DeleteObject();

	return TRUE;
}

HBRUSH CFileDropDlg::OnCtlColor(CDC* pDC, CWnd* /*pWnd*/, UINT /*nCtlColor*/)
{
	pDC->SetBkMode(TRANSPARENT);
	return (HBRUSH)GetStockObject(NULL_BRUSH);
}

void CFileDropDlg::OnActivate(UINT /*nState*/, CWnd* /*pWndOther*/, BOOL /*bMinimized*/)
{
	UpdateStatus();
	Invalidate();
}

// Die System ruft diese Funktion auf, um den Cursor abzufragen, der angezeigt wird, während der Benutzer
//  das minimierte Fenster mit der Maus zieht.
HCURSOR CFileDropDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CFileDropDlg::OnThemeChanged()
{
	if (theApp.m_ThemeLibLoaded)
	{
		if (hTheme)
			theApp.zCloseThemeData(hTheme);

		hTheme = theApp.zOpenThemeData(m_hWnd, VSCLASS_WINDOW);
		Themed = (hTheme!=NULL);
	}

	OnCompositionChanged();
	return TRUE;
}

void CFileDropDlg::OnCompositionChanged()
{
	if (AeroLibLoaded)
	{
		UseGlassBackground(m_hWnd);
	}
	else
	{
		Glassed = FALSE;
	}
}

void CFileDropDlg::OnNcRButtonDown(UINT /*nFlags*/, CPoint point)
{
	CMenu menu;
	if (menu.CreatePopupMenu())
	{
		// Always on top
		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDS_ALWAYSONTOP));
		menu.AppendMenu(MF_STRING | (AlwaysOnTop ? MF_CHECKED:0), IDM_ALWAYSONTOP, tmpStr);

		// Small window
		ENSURE(tmpStr.LoadString(IDS_SMALLWINDOW));
		menu.AppendMenu(MF_STRING | (SmallWindow ? MF_CHECKED:0), IDM_SMALLWINDOW, tmpStr);

		// Choose default store
		ENSURE(tmpStr.LoadString(IDS_CHOOSEDEFAULTSTORE));
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, IDM_CHOOSEDEFAULTSTORE, tmpStr);

		// Launch StoreManager
		ENSURE(tmpStr.LoadString(IDS_STOREMANAGER));
		menu.AppendMenu(MF_STRING | (_access(theApp.path + "StoreManager.exe", 0)==0 ? 0 : MF_GRAYED), IDM_STOREMANAGER, tmpStr);

		// About
		ENSURE(tmpStr.LoadString(IDS_ABOUT));
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, IDM_ABOUT, tmpStr);

		menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
}

LRESULT CFileDropDlg::OnNcHitTest(CPoint point)
{
	LRESULT uHitTest = DefWindowProc(WM_NCHITTEST, 0, (point.y<<16) | point.x);
	return (uHitTest==HTCLIENT) ? HTCAPTION : uHitTest;
}

void CFileDropDlg::OnClose()
{
	CRect r;
	GetWindowRect(&r);
	theApp.WriteProfileInt(_T(""), _T("AlwaysOnTop"), AlwaysOnTop);
	theApp.WriteProfileInt(_T(""), _T("SmallWindow"), SmallWindow);
	theApp.WriteProfileInt(_T(""), _T("X"), r.left);
	theApp.WriteProfileInt(_T(""), _T("Y"), r.top);
	CDialog::OnClose();
}

void CFileDropDlg::OnDestroy()
{
	if (TimerID)
		KillTimer(TimerID);

	DoneAero();

	if (dropzoneL)
		delete dropzoneL;
	if (dropzoneS)
		delete dropzoneS;
	if (ready)
		delete ready;
	if (warning)
		delete warning;

	CDialog::OnDestroy();
}

void CFileDropDlg::OnTimer(UINT_PTR _TimerID)
{
	if (_TimerID==TimerID)
	{
		BOOL old = liquidFOLDERSReady;
		UpdateStatus();
		if (old!=liquidFOLDERSReady)
		{
			Invalidate();
			UpdateWindow();
		}
	}
}

void CFileDropDlg::OnChooseDefaultStore()
{
	LFChooseDefaultStoreDlg dlg(this);
	dlg.DoModal();
}
