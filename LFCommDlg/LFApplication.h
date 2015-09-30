
// LFApplication.h: Hauptheaderdatei für die Anwendung
//

#pragma once
#include "resource.h"
#include "CGdiPlusBitmap.h"
#include "LFCore.h"
#include "LFTooltip.h"
#include <uxtheme.h>

#define RatingBitmapWidth      88
#define RatingBitmapHeight     15

#define OS_XP                   0
#define OS_Vista                1
#define OS_Seven                2
#define OS_Eight                3

#define NAG_NOTLICENSED         0
#define NAG_EXPIRED             1
#define NAG_COUNTER             0
#define NAG_FORCE               2

typedef HRESULT(__stdcall* PFNSETWINDOWTHEME)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
typedef HRESULT(__stdcall* PFNCLOSETHEMEDATA)(HTHEME hTheme);
typedef HTHEME(__stdcall* PFNOPENTHEMEDATA)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT(__stdcall* PFNDRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, INT iPartId,
							INT iStateId, const RECT* pRect, const RECT* pClipRect);
typedef HRESULT(__stdcall* PFNDRAWTHEMETEXT)(HTHEME hTheme, HDC hdc, INT iPartId,
							INT iStateId, LPCWSTR pszText, INT iCharCount, DWORD dwTextFlags,
							DWORD dwTextFlags2, const RECT* pRect);
typedef HRESULT(__stdcall* PFNDRAWTHEMETEXTEX)(HTHEME hTheme, HDC hdc, INT iPartId,
							INT iStateId, LPCWSTR pszText, INT iCharCount, DWORD dwTextFlags,
							const RECT* pRect, const DTTOPTS* pOptions);
typedef HRESULT(__stdcall* PFNGETTHEMESYSFONT)(HTHEME hTheme, INT iFontID, LOGFONT* plf);
typedef HRESULT(__stdcall* PFNGETTHEMESYSCOLOR)(HTHEME hTheme, INT iColorID);
typedef HRESULT (__stdcall* PFNGETTHEMEPARTSIZE)(HTHEME hTheme, HDC hdc, INT iPartId, INT iStateId,
							LPRECT prc, THEMESIZE eSize, SIZE *psz);
typedef HRESULT (__stdcall* PFNSETWINDOWTHEMEATTRIBUTE)(HWND hWnd, WINDOWTHEMEATTRIBUTETYPE eAttribute,
							void* pAttribute, DWORD cdAttribute);
typedef BOOL (__stdcall* PFNISTHEMEACTIVE)();
typedef BOOL (__stdcall* PFNISAPPTHEMED)();

typedef HRESULT(__stdcall* PFNDWMISCOMPOSITIONENABLED)(BOOL* pfEnabled);
typedef HRESULT(__stdcall* PFNDWMEXTENDFRAMEINTOCLIENTAREA)(HWND hWnd, const MARGINS* pMarInset);
typedef BOOL(__stdcall* PFNDWMDEFWINDOWPROC)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* plResult);

typedef HRESULT(__stdcall* PFNSETCURRENTPROCESSEXPLICITAPPUSERMODELID)(PCWSTR AppID);

typedef HRESULT(__stdcall* PFNREGISTERAPPLICATIONRESTART)(PCWSTR CommandLine, DWORD Flags);

struct CDS_Wakeup
{
	GUID AppID;
	WCHAR Command[MAX_PATH];
};

struct ResourceCacheItem
{
	CGdiPlusBitmapResource* pImage;
	UINT nID;
};


// LFApplication:
// Siehe LFApplication.cpp für die Implementierung dieser Klasse
//

class LFUpdateDlg;

class LFApplication : public CWinAppEx
{
public:
	LFApplication(GUID& AppID);
	virtual ~LFApplication();

	LFMessageIDs* p_MessageIDs;
	WCHAR m_AttrCategoryNames[LFAttrCategoryCount][256];
	WCHAR m_SourceNames[LFSourceCount][2][256];
	LFAttributeDescriptor m_Attributes[LFAttributeCount];
	LFContextDescriptor m_Contexts[LFContextCount];
	LFItemCategoryDescriptor m_ItemCategories[LFItemCategoryCount];
	CImageList m_SystemImageListSmall;
	CImageList m_SystemImageListLarge;
	CImageList m_SystemImageListExtraLarge;
	CImageList m_SystemImageListJumbo;
	CImageList m_CoreImageListSmall;
	CImageList m_CoreImageListLarge;
	CImageList m_CoreImageListExtraLarge;
	CImageList m_CoreImageListHuge;
	CImageList m_CoreImageListJumbo;
	HBITMAP m_RatingBitmaps[LFMaxRating+1];
	HBITMAP m_PriorityBitmaps[LFMaxRating+1];
	CFont m_DefaultFont;
	CFont m_BoldFont;
	CFont m_ItalicFont;
	CFont m_SmallFont;
	CFont m_LargeFont;
	CFont m_CaptionFont;
	CFont m_UACFont;
	BOOL IsLicensed;
	UINT OSVersion;
	UINT m_WakeupMsg;
	GUID m_AppID;
	CLIPFORMAT CF_FILEDESCRIPTOR;
	CLIPFORMAT CF_FILECONTENTS;
	CLIPFORMAT CF_HLIQUID;
	CList<CWnd*> m_pMainFrames;
	LFUpdateDlg* m_pUpdateNotification;

	PFNSETWINDOWTHEME zSetWindowTheme;
	PFNOPENTHEMEDATA zOpenThemeData;
	PFNCLOSETHEMEDATA zCloseThemeData;
	PFNDRAWTHEMEBACKGROUND zDrawThemeBackground;
	PFNDRAWTHEMETEXT zDrawThemeText;
	PFNDRAWTHEMETEXTEX zDrawThemeTextEx;
	PFNGETTHEMESYSFONT zGetThemeSysFont;
	PFNGETTHEMESYSCOLOR zGetThemeSysColor;
	PFNGETTHEMEPARTSIZE zGetThemePartSize;
	PFNSETWINDOWTHEMEATTRIBUTE zSetWindowThemeAttribute;
	PFNISAPPTHEMED zIsAppThemed;
	BOOL m_ThemeLibLoaded;

	PFNDWMISCOMPOSITIONENABLED zDwmIsCompositionEnabled;
	PFNDWMEXTENDFRAMEINTOCLIENTAREA zDwmExtendFrameIntoClientArea;
	PFNDWMDEFWINDOWPROC zDwmDefWindowProc;
	BOOL m_AeroLibLoaded;

	PFNSETCURRENTPROCESSEXPLICITAPPUSERMODELID zSetCurrentProcessExplicitAppUserModelID;
	BOOL m_ShellLibLoaded;

	PFNREGISTERAPPLICATIONRESTART zRegisterApplicationRestart;
	BOOL m_KernelLibLoaded;

	virtual BOOL InitInstance();
	virtual CWnd* OpenCommandLine(WCHAR* CmdLine=NULL);
	virtual INT ExitInstance();

	void AddFrame(CWnd* pFrame);
	void KillFrame(CWnd* pVictim);
	BOOL ShowNagScreen(UINT Level, CWnd* pWndParent=NULL, BOOL Abort=FALSE);
	CString GetDefaultFontFace();
	void SendMail(CString Subject=_T(""));
	BOOL IsAttributeAllowed(INT Context, INT Attr);
	INT GetGlobalInt(LPCTSTR lpszEntry, INT nDefault=0);
	CString GetGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszDefault=_T(""));
	BOOL WriteGlobalInt(LPCTSTR lpszEntry, INT nValue);
	BOOL WriteGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszValue);
	CGdiPlusBitmap* GetCachedResourceImage(UINT nID, LPCTSTR pType=RT_RCDATA);
	static HICON LoadDialogIcon(UINT nID);
	static HANDLE LoadFontFromResource(UINT nID);
	static void ExtractCoreIcons(HINSTANCE hModIcons, INT Size, CImageList* pImageList);
	void ShowTooltip(CWnd* pCallerWnd, CPoint point, const CString& strCaption, const CString& strText, HICON hIcon=NULL, HBITMAP hBitmap=NULL);
	BOOL IsTooltipVisible();
	void HideTooltip();
	void ExecuteExplorerContextMenu(CHAR Drive, LPCSTR Verb);
	static void PlayAsteriskSound();
	static void PlayDefaultSound();
	static void PlayErrorSound();
	static void PlayNavigateSound();
	static void PlayNotificationSound();
	static void PlayQuestionSound();
	static void PlayTrashSound();
	static void PlayWarningSound();
	void GetUpdateSettings(BOOL* EnableAutoUpdate, INT* Interval);
	void SetUpdateSettings(BOOL EnableAutoUpdate, INT Interval);
	BOOL IsUpdateCheckDue();
	void GetBinary(LPCTSTR lpszEntry, void* pData, UINT size);

protected:
	LFTooltip m_wndTooltip;
	CList<ResourceCacheItem> m_ResourceCache;
	UINT m_NagCounter;

	afx_msg void OnAppPurchase();
	afx_msg void OnAppEnterLicenseKey();
	afx_msg void OnAppSupport();
	afx_msg void OnUpdateAppCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	ULONG_PTR m_gdiplusToken;
	HMODULE hModThemes;
	HMODULE hModAero;
	HMODULE hModShell;
	HMODULE hModKernel;
	HANDLE hFontLetterGothic;
};
