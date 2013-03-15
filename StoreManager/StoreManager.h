
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

class CStoreManagerApp : public LFApplication
{
public:
	CStoreManagerApp();

	virtual BOOL InitInstance();
	virtual INT ExitInstance();

	void AddFrame(CMainWnd* pFrame);
	void KillFrame(CMainWnd* pVictim);
	CMainWnd* GetClipboard();

	void Broadcast(INT Context, INT View, UINT cmdMsg);
	void UpdateSortOptions(INT Context);
	void UpdateViewOptions(INT Context=-1, INT View=-1);
	void Reload(INT Context);
	void UpdateFooter(INT Context=-1, INT View=-1);

	CList<CMainWnd*> m_MainFrames;
	CMainWnd* p_Clipboard;
	UINT m_WakeupMsg;
	CString m_PathGoogleEarth;
	CFormatCache m_FileFormats;
	CThumbnailCache m_ThumbnailCache;
	LFViewParameters m_Views[LFContextCount];
	LFBitArray* m_AllowedViews[LFContextCount];

	UINT m_nTextureSize;
	UINT m_nMaxTextureSize;

	BOOL m_ShowFilterPane;
	BOOL m_ShowInspectorPane;
	UINT m_FilterWidth;
	UINT m_InspectorWidth;

	BOOL m_ShowEmptyVolumes;
	BOOL m_ShowEmptyDomains;
	BOOL m_ShowStatistics;
	BOOL m_CalendarShowStatistics;
	BOOL m_CalendarShowCaptions;
	BOOL m_CalendarShowEmptyDays;
	BOOL m_GlobeHQModel;
	BOOL m_GlobeLighting;
	BOOL m_GlobeAtmosphere;
	BOOL m_GlobeShowViewport;
	BOOL m_GlobeShowCrosshairs;
	BOOL m_TagcloudShowLegend;

protected:
	BOOL m_AppInitialized;

	BOOL SanitizeSortBy(LFViewParameters* vp, INT Context);
	BOOL SanitizeViewMode(LFViewParameters* vp, INT Context);
	void GetBinary(LPCTSTR lpszEntry, void* pData, UINT size);
	void LoadViewOptions(INT context);
	void SaveViewOptions(INT context);

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CStoreManagerApp theApp;
