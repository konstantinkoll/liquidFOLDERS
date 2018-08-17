
// liquidFOLDERS.h: Hauptheaderdatei für die liquidFOLDERS-Anwendung
//

#pragma once
#include "LFCommDlg.h"
#include "CIconFactory.h"
#include "CMainWnd.h"
#include "resource.h"


// CLiquidFoldersApp:
// Siehe liquidFOLDERS.cpp für die Implementierung dieser Klasse
//

#define STARTWITH_STORES        0
#define STARTWITH_ALLFILES      1
#define STARTWITH_FAVORITES     2
#define STARTWITH_TASKS         3
#define STARTWITH_NEW           4

class CLiquidFoldersApp : public LFApplication
{
public:
	CLiquidFoldersApp();

	virtual BOOL InitInstance();
	virtual BOOL OpenCommandLine(LPWSTR pCmdLine=NULL);
	virtual INT ExitInstance();

	CMainWnd* GetClipboard();
	void OpenFileDrop(const ABSOLUTESTOREID& StoreID);

	void Broadcast(INT Context, INT View, UINT cmdMsg);
	void SetContextSort(INT Context, UINT Attr, BOOL SortDescending, BOOL SetLastView=TRUE);
	void UpdateViewSettings(INT Context=-1, INT View=-1);
	void SetContextView(INT Context, INT View);

	CMainWnd* p_ClipboardWnd;
	WCHAR m_PathGoogleEarth[MAX_PATH];
	CIconFactory m_IconFactory;
	LFContextViewSettings m_ContextViewSettings[LFContextCount];
	LFGlobalViewSettings m_GlobalViewSettings;

	INT m_StartWith;

	BOOL m_ShowInspectorPane;
	UINT m_InspectorPaneWidth;

	BOOL m_FileDropAlwaysOnTop;

protected:
	afx_msg void OnBackstageAbout();
	DECLARE_MESSAGE_MAP()

	void SanitizeContextViewSettings(INT Context);
	void LoadContextViewSettings(UINT Context, BOOL Reset);
	void SaveContextViewSettings(UINT Context);
	BOOL LoadGlobalViewSettings();
	void SaveGlobalViewSettings();

	BOOL m_AppInitialized;
};

extern CLiquidFoldersApp theApp;
