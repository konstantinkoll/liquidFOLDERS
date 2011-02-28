
// StoreManager.h: Hauptheaderdatei für die StoreManager-Anwendung
//

#pragma once
#include "LFCommDlg.h"
#include "CFormatCache.h"
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

	CString m_PathGoogleEarth;
	CList<CMainWnd*> m_MainFrames;
	CMainWnd* p_Clipboard;
	CFormatCache m_FileFormats;
	LFViewParameters m_Views[LFContextCount];
	LFBitArray* m_AllowedViews[LFContextCount];

	UINT m_nTextureSize;
	UINT m_nMaxTextureSize;
	UINT m_NagCounter;
	BOOL m_ShowEmptyDrives;
	BOOL m_ShowEmptyDomains;
	BOOL m_ShowStatistics;
	BOOL m_CalendarShowStatistics;
	BOOL m_CalendarShowCaptions;
	BOOL m_CalendarShowEmptyDays;
	BOOL m_GlobeHQModel;
	BOOL m_GlobeLighting;
	BOOL m_GlobeAtmosphere;
	BOOL m_GlobeShadows;
	BOOL m_GlobeBlackBackground;
	BOOL m_GlobeShowViewport;
	BOOL m_GlobeShowCrosshairs;
	BOOL m_TagcloudShowLegend;

	virtual BOOL InitInstance();
	virtual INT ExitInstance();

	void AddFrame(CMainWnd* pFrame);
	void KillFrame(CMainWnd* pVictim);
	CMainWnd* GetClipboard();
	void CloseAllFrames(BOOL LeaveOne=FALSE);

	HBITMAP SetContextMenuIcon(CMenu* pMenu, UINT CmdID, UINT ResID);

	void Broadcast(INT Context, INT View, UINT cmdMsg);
	void UpdateSortOptions(INT Context);
	void UpdateViewOptions(INT Context=-1, INT View=-1);
	void Reload(INT Context);
	void UpdateFooter(INT Context=-1, INT View=-1);

protected:
	BOOL SanitizeSortBy(LFViewParameters* vp, INT Context);
	BOOL SanitizeViewMode(LFViewParameters* vp, INT Context);
	void GetBinary(LPCTSTR lpszEntry, void* pData, UINT size);
	void LoadViewOptions(INT context);
	void SaveViewOptions(INT context);

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CStoreManagerApp theApp;
