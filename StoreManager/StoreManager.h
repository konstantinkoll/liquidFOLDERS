
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

class CStoreManagerApp : public LFApplication
{
public:
	CStoreManagerApp();
	virtual ~CStoreManagerApp();

	CString path_GoogleEarth;
	UINT m_nAppLook;
	UINT m_nTextureSize;
	UINT m_nMaxTextureSize;
	UINT m_NagCounter;
	BOOL m_GlobeHQModel;
	BOOL m_GlobeLighting;
	BOOL m_HideEmptyDrives;
	BOOL m_HideEmptyDomains;
	std::list<CMainFrame*> m_listMainFrames;
	std::list<CMainFrame*> m_listClipboardFrames;
	stdext::hash_map<std::string, std::wstring> m_Extensions;
	LFViewParameters m_Views[LFContextCount];
	LFBitArray* m_AllowedViews[LFContextCount];

	virtual BOOL InitInstance();
	virtual INT ExitInstance();
	virtual void SetApplicationLook(UINT nID);

	void AddFrame(CMainFrame* pFrame);
	void KillFrame(CMainFrame* pFrame);
	void ReplaceMainFrame(CMainFrame* pFrame);
	CMainFrame* GetClipboard(BOOL ForceNew);
	void CloseAllFrames(BOOL leaveOne=FALSE);

	BOOL SanitizeSortBy(LFViewParameters* vp, INT context);
	BOOL SanitizeViewMode(LFViewParameters* vp, INT context);
	void UpdateSortOptions(INT context);
	void UpdateViewOptions(INT context=-1);
	void Reload(INT context);

	void GetBinary(LPCTSTR lpszEntry, void* pData, UINT size);
	void ToggleAttribute(LFViewParameters* vp, UINT attr, INT ColumnCount=-1);
	HBITMAP GetGLTexture(UINT nID);
	void FreeGLTexture(UINT nID);
	static void GetRibbonColors(COLORREF* back, COLORREF* text=NULL, COLORREF* highlight=NULL);
	static CString GetCommandName(UINT nID, BOOL bInsertSpace=FALSE);
	static CMFCRibbonButton* CommandButton(UINT nID, INT nSmallImageIndex=-1, INT nLargeImageIndex=-1, BOOL bAlwaysShowDescription=FALSE, BOOL bInsertSpace=FALSE);
	static CMFCRibbonCheckBox* CommandCheckBox(UINT nID);

protected:
	virtual void OnClosingMainFrame(CFrameImpl* pFrameImpl);
	void LoadViewOptions(INT context);
	void SaveViewOptions(INT context);

	afx_msg void OnAppAbout();
	afx_msg void OnAppNewView();
	afx_msg void OnAppNewClipboard();
	afx_msg void OnAppExit();
	DECLARE_MESSAGE_MAP()

private:
	void GetMaxTextureSize();

	HBITMAP m_GLTextureCache[4];
	UINT m_GLTextureBinds[4];
};

extern CStoreManagerApp theApp;
