
// StoreManager.h: Hauptheaderdatei für die StoreManager-Anwendung
//

#pragma once
#include "LFCommDlg.h"
#include "CFormatCache.h"
#include "CThumbnailCache.h"
#include "MainWnd.h"
#include "resource.h"
#include <hash_map>
#include <string>


// CStoreManagerApp:
// Siehe StoreManager.cpp für die Implementierung dieser Klasse
//

#define ViewParametersVersion     1

class CStoreManagerApp : public LFApplication
{
public:
	CStoreManagerApp();

	virtual BOOL InitInstance();
	virtual CWnd* OpenCommandLine(WCHAR* CmdLine=NULL);
	virtual INT ExitInstance();

	void AddFrame(CMainWnd* pFrame);
	void KillFrame(CMainWnd* pVictim);
	CMainWnd* GetClipboard();

	void Broadcast(INT Context, INT View, UINT cmdMsg);
	void UpdateSortOptions(INT Context);
	void UpdateViewOptions(INT Context=-1, INT View=-1);
	void UpdateNumbers();
	void Reload(INT Context);

	CList<CMainWnd*> m_MainFrames;
	CMainWnd* p_Clipboard;
	CString m_PathGoogleEarth;
	CImageListTransparent m_SourceIcons;
	CFormatCache m_FileFormats;
	CThumbnailCache m_ThumbnailCache;
	LFViewParameters m_Views[LFContextCount];
	LFBitArray* m_AllowedViews[LFContextCount];

	UINT m_nTextureSize;
	UINT m_nMaxTextureSize;

	BOOL m_ShowInspectorPane;
	UINT m_InspectorWidth;

	BOOL m_CalendarShowCaptions;
	BOOL m_GlobeHQModel;
	BOOL m_GlobeLighting;
	BOOL m_GlobeAtmosphere;
	BOOL m_GlobeShowViewport;
	BOOL m_GlobeShowCrosshairs;

protected:
	BOOL m_AppInitialized;

	BOOL SanitizeSortBy(LFViewParameters* vp, INT Context);
	BOOL SanitizeViewMode(LFViewParameters* vp, INT Context);
	void GetBinary(LPCTSTR lpszEntry, void* pData, UINT size);
	void LoadViewOptions(UINT context);
	void SaveViewOptions(UINT context);

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CStoreManagerApp theApp;
