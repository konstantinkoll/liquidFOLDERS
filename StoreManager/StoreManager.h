
// StoreManager.h: Hauptheaderdatei für die StoreManager-Anwendung
//

#pragma once
#include "resource.h"
#include "MainFrm.h"
#include "liquidFOLDERS.h"
#include "LFCommDlg.h"
#include <list>
using std::list;

#define SaveMode_SettingsChanged  0
#define SaveMode_Force            1
#define SaveMode_FlagOnly         2


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
	BOOL m_ShowQueryDuration;
	BOOL m_GlobeHQModel;
	BOOL m_GlobeLighting;
	BOOL m_HideEmptyDrives;
	BOOL m_HideEmptyDomains;
	list<CMainFrame*> m_listMainFrames;
	list<CMainFrame*> m_listClipboardFrames;
	LFViewParameters m_Views[LFContextCount];
	CImageList m_Icons128;
	CImageList m_Icons64;
	CImageList m_Icons48;
	CImageList m_Icons24;
	CImageList m_Icons16;

	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual void SetApplicationLook(UINT nID);

	void AddFrame(CMainFrame* pFrame);
	void KillFrame(CMainFrame* pFrame);
	void ReplaceMainFrame(CMainFrame* pFrame);
	CMainFrame* GetClipboard(BOOL ForceNew);
	void CloseAllFrames(BOOL leaveOne=FALSE);
	void OpenChildViews(int context, BOOL UpdateViewOptions=FALSE);
	void UpdateViewOptions(int context=-1);
	void UpdateSortOptions(int context);
	void Reload(int context);
	void GetBinary(LPCTSTR lpszEntry, void* pData, UINT size);
	void LoadViewOptions(int context);
	void SaveViewOptions(int context, UINT SaveMode=SaveMode_SettingsChanged);
	void ToggleAttribute(LFViewParameters* vp, UINT attr, int ColumnCount=-1);
	HBITMAP GetGLTexture(UINT nID);
	void FreeGLTexture(UINT nID);
	static void GetRibbonColors(COLORREF* back, COLORREF* text=NULL, COLORREF* highlight=NULL);
	static CMFCRibbonButton* CommandButton(UINT nID, int nSmallImageIndex=-1, int nLargeImageIndex=-1, BOOL bAlwaysShowDescription=FALSE, BOOL bInsertSpace=FALSE);
	static CMFCRibbonCheckBox* CommandCheckBox(UINT nID);

protected:
	virtual void OnClosingMainFrame(CFrameImpl* pFrameImpl);

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
