
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
	CWnd* GetFileDrop(LPCSTR StoreID);

	BOOL IsViewAllowed(INT Context, INT View) const;
	BOOL IsAttributeAvailable(INT Context, UINT Attr) const;
	BOOL IsAttributeAdvertised(INT Context, UINT Attr) const;
	BOOL IsAttributeSortable(INT Context, UINT Attr) const;
	void Broadcast(INT Context, INT View, UINT cmdMsg);
	void SetContextSort(INT Context, UINT Attr, BOOL Descending, BOOL SetLastView=TRUE);
	void UpdateViewSettings(INT Context=-1, INT View=-1);
	void SetContextView(INT Context, INT View);
	void Reload(INT Context);

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

inline BOOL CLiquidFoldersApp::IsViewAllowed(INT Context, INT View) const
{
	ASSERT(Context>=0);
	ASSERT(Context<LFContextCount);
	ASSERT(View>=0);
	ASSERT(View<=31);

	return (m_Contexts[Context].CtxProperties.AvailableViews>>View) & 1;
}

inline BOOL CLiquidFoldersApp::IsAttributeAvailable(INT Context, UINT Attr) const
{
	ASSERT(Context>=0);
	ASSERT(Context<LFContextCount);
	ASSERT(Attr<LFAttributeCount);

	return (m_Contexts[Context].CtxProperties.AvailableAttributes>>Attr) & 1;
}

inline BOOL CLiquidFoldersApp::IsAttributeAdvertised(INT Context, UINT Attr) const
{
	ASSERT(Context>=0);
	ASSERT(Context<LFContextCount);
	ASSERT(Attr<LFAttributeCount);

	return ((m_Contexts[Context].CtxProperties.AdvertisedAttributes>>Attr) & 1);
}

inline BOOL CLiquidFoldersApp::IsAttributeSortable(INT Context, UINT Attr) const
{
	ASSERT(Context>=0);
	ASSERT(Context<LFContextCount);
	ASSERT(Attr<LFAttributeCount);

	return (Context<=LFLastPersistentContext) || m_Attributes[Attr].TypeProperties.SortableSubfolder;
}

inline void CLiquidFoldersApp::Reload(INT Context)
{
	Broadcast(Context, -1, WM_RELOAD);
}

extern CLiquidFoldersApp theApp;
