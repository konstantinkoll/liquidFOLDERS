
// StoreManager.h: Hauptheaderdatei für die StoreManager-Anwendung
//

#pragma once
#include "LFCommDlg.h"
#include "CFormatCache.h"
#include "MainFrm.h"
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
	CList<CMainFrame*> m_MainFrames;
	CMainFrame* p_Clipboard;
	CFormatCache m_FileFormats;
	LFViewParameters m_Views[LFContextCount];
	LFBitArray* m_AllowedViews[LFContextCount];

	UINT m_nTextureSize;
	UINT m_nMaxTextureSize;
	UINT m_NagCounter;
	BOOL m_ShowEmptyDrives;
	BOOL m_ShowEmptyDomains;
	BOOL m_ShowStatistics;
	BOOL m_ShowCaptions;
	BOOL m_ShowEmptyDays;
	BOOL m_GlobeHQModel;
	BOOL m_GlobeLighting;
	BOOL m_GlobeAtmosphere;
	BOOL m_GlobeShadows;
	BOOL m_GlobeBlackBackground;
	BOOL m_GlobeShowViewport;
	BOOL m_GlobeShowCrosshairs;

	virtual BOOL InitInstance();
	virtual INT ExitInstance();
		virtual void OnClosingMainFrame(CFrameImpl* pFrameImpl);	// Axe

	void AddFrame(CMainFrame* pFrame);
	void KillFrame(CMainFrame* pVictim);
	void ReplaceMainFrame(CMainFrame* pVictim);
	CMainFrame* GetClipboard();
	void CloseAllFrames(BOOL LeaveOne=FALSE);

	HBITMAP SetContextMenuIcon(CMenu* pMenu, UINT CmdID, UINT ResID);

	BOOL SanitizeSortBy(LFViewParameters* vp, INT Context);
	BOOL SanitizeViewMode(LFViewParameters* vp, INT Context);
	void Broadcast(INT Context, INT View, UINT cmdMsg);
	void UpdateSortOptions(INT Context);
	void UpdateViewOptions(INT Context=-1, INT View=-1);
	void Reload(INT Context);
	void UpdateFooter(INT Context);

		static CString GetCommandName(UINT nID, BOOL bInsertSpace=FALSE);							// Axe
		static CMFCRibbonButton* CommandButton(UINT nID, INT nSmallImageIndex=-1, INT nLargeImageIndex=-1, BOOL bAlwaysShowDescription=FALSE, BOOL bInsertSpace=FALSE);	// Axe

protected:
	void GetBinary(LPCTSTR lpszEntry, void* pData, UINT size);
	void LoadViewOptions(INT context);
	void SaveViewOptions(INT context);

	afx_msg void OnAppAbout();
	afx_msg void OnAppNewView();
	afx_msg void OnAppExit();
	DECLARE_MESSAGE_MAP()
};

extern CStoreManagerApp theApp;
