
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

class CLiquidFoldersApp : public LFApplication
{
public:
	CLiquidFoldersApp();

	virtual BOOL InitInstance();
	virtual CWnd* OpenCommandLine(LPWSTR pCmdLine=NULL);
	virtual INT ExitInstance();

	CMainWnd* GetClipboard();
	CWnd* GetFileDrop(const ABSOLUTESTOREID& StoreID);

	void Broadcast(INT Context, INT View, UINT cmdMsg);
	void SetContextSort(INT Context, UINT Attr, BOOL SortDescending, BOOL SetLastView=TRUE);
	void UpdateViewSettings(INT Context=-1, INT View=-1);
	void SetContextView(INT Context, INT View);

	CMainWnd* p_ClipboardWnd;
	WCHAR m_PathGoogleEarth[MAX_PATH];
	CIconFactory m_IconFactory;
	LFContextViewSettings m_ContextViewSettings[LFContextCount];
	LFGlobalViewSettings m_GlobalViewSettings;

	BOOL m_ShowInspectorPane;
	UINT m_InspectorPaneWidth;

	BOOL m_FileDropAlwaysOnTop;

protected:
	void SanitizeContextViewSettings(INT Context);
	void LoadContextViewSettings(UINT Context, BOOL Reset);
	void SaveContextViewSettings(UINT Context);
	BOOL LoadGlobalViewSettings();
	void SaveGlobalViewSettings();

	BOOL m_AppInitialized;
};

extern CLiquidFoldersApp theApp;
