
// StoreManager.h: Hauptheaderdatei für die StoreManager-Anwendung
//

#pragma once
#include "resource.h"
#include "MainFrm.h"
#include "liquidFOLDERS.h"
#include "LFCommDlg.h"
#include <hash_map>
#include <list>
#include <string>

// CStoreManagerApp:
// Siehe StoreManager.cpp für die Implementierung dieser Klasse
//

struct FormatData
{
	INT SysIconIndex;
	WCHAR FormatName[80];
};

class CStoreManagerApp : public LFApplication
{
public:
	CStoreManagerApp();

	CString m_PathGoogleEarth;
	UINT m_nTextureSize;
	UINT m_nMaxTextureSize;
	UINT m_NagCounter;
	BOOL m_HideEmptyDrives;
	BOOL m_HideEmptyDomains;
	BOOL m_HideStatistics;
	BOOL m_HideCaptions;
	BOOL m_HideEmptyDays;
	BOOL m_GlobeHQModel;
	BOOL m_GlobeLighting;
	BOOL m_GlobeAtmosphere;
	BOOL m_GlobeShadows;
	BOOL m_GlobeBlackBackground;
	BOOL m_GlobeShowViewport;
	BOOL m_GlobeShowCrosshairs;
	stdext::hash_map<std::string, FormatData> m_FileFormats;
	std::list<CMainFrame*> m_listMainFrames;
	CMainFrame* p_Clipboard;
	LFViewParameters m_Views[LFContextCount];
	LFBitArray* m_AllowedViews[LFContextCount];

	virtual BOOL InitInstance();
	virtual INT ExitInstance();
		virtual void OnClosingMainFrame(CFrameImpl* pFrameImpl);	// Axe

	void AddFrame(CMainFrame* pFrame);
	void KillFrame(CMainFrame* pFrame);
	void ReplaceMainFrame(CMainFrame* pFrame);
	CMainFrame* GetClipboard();
	void CloseAllFrames(BOOL leaveOne=FALSE);

	HBITMAP SetContextMenuIcon(CMenu* pMenu, UINT CmdID, UINT ResID);

	BOOL SanitizeSortBy(LFViewParameters* vp, INT context);
	BOOL SanitizeViewMode(LFViewParameters* vp, INT context);
	void Broadcast(INT context, UINT cmdMsg);
	void UpdateSortOptions(INT context);
	void UpdateViewOptions(INT context=-1);
	void Reload(INT context);
	void UpdateSearchResult(INT context);
	void PrepareFormatData(CHAR* FileFormat);

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
