
// LFApplication.h: Hauptheaderdatei für die Anwendung
//

#pragma once
#include "resource.h"
#include "CIcons.h"
#include "GLRenderer.h"
#include "LFCore.h"
#include "LFFont.h"
#include "LFTooltip.h"
#include <uxtheme.h>

#define RatingBitmapWidth      88
#define RatingBitmapHeight     15

#define OS_XP                   0
#define OS_Vista                1
#define OS_Seven                2
#define OS_Eight                3

typedef HRESULT(__stdcall* PFNSETWINDOWTHEME)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
typedef HRESULT(__stdcall* PFNCLOSETHEMEDATA)(HTHEME hTheme);
typedef HTHEME(__stdcall* PFNOPENTHEMEDATA)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT(__stdcall* PFNDRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, INT iPartId,
							INT iStateId, const RECT* pRect, const RECT* pClipRect);
typedef HRESULT (__stdcall* PFNGETTHEMEPARTSIZE)(HTHEME hTheme, HDC hdc, INT iPartId, INT iStateId,
							LPCRECT prc, THEMESIZE eSize, SIZE *psz);
typedef BOOL (__stdcall* PFNISTHEMEACTIVE)();
typedef BOOL (__stdcall* PFNISAPPTHEMED)();

typedef HRESULT(__stdcall* PFNDWMISCOMPOSITIONENABLED)(BOOL* pfEnabled);
typedef HRESULT(__stdcall* PFNDWMSETWINDOWATTRIBUTE)(HWND hWnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);

typedef HRESULT(__stdcall* PFNSETCURRENTPROCESSEXPLICITAPPUSERMODELID)(PCWSTR AppID);

typedef HRESULT(__stdcall* PFNREGISTERAPPLICATIONRESTART)(PCWSTR CommandLine, DWORD Flags);

typedef HRESULT(__stdcall* PFNCHANGEWINDOWMESSAGEFILTER)(HWND hWnd, UINT message, DWORD action);

struct CDS_Wakeup
{
	GUID AppID;
	WCHAR Command[MAX_PATH];
};

struct ResourceCacheItem
{
	Bitmap* pImage;
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

	virtual BOOL InitInstance();
	virtual CWnd* OpenCommandLine(WCHAR* CmdLine=NULL);
	virtual INT ExitInstance();

	void AddFrame(CWnd* pFrame);
	void KillFrame(CWnd* pVictim);
	void SendMail(const CString& Subject=_T("")) const;
	BOOL IsAttributeAllowed(INT Context, INT Attr) const;
	INT GetGlobalInt(LPCTSTR lpszEntry, INT nDefault=0) const;
	CString GetGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszDefault=_T("")) const;
	BOOL WriteGlobalInt(LPCTSTR lpszEntry, INT nValue) const;
	BOOL WriteGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszValue) const;
	Bitmap* GetResourceImage(UINT nID) const;
	Bitmap* GetCachedResourceImage(UINT nID);
	static HICON LoadDialogIcon(UINT nID);
	static HANDLE LoadFontFromResource(UINT nID);
	static void ExtractCoreIcons(HINSTANCE hModIcons, INT Size, CImageList* pImageList);
	void ShowTooltip(CWnd* pCallerWnd, CPoint point, const CString& Caption, const CString& Hint, HICON hIcon=NULL, HBITMAP hBitmap=NULL);
	BOOL IsTooltipVisible() const;
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
	void GetUpdateSettings(BOOL* EnableAutoUpdate, INT* Interval) const;
	void SetUpdateSettings(BOOL EnableAutoUpdate, INT Interval) const;
	BOOL IsUpdateCheckDue() const;
	void GetBinary(LPCTSTR lpszEntry, void* pData, UINT Size);

	const LFMessageIDs* p_MessageIDs;
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
	HBITMAP hRatingBitmaps[LFMaxRating+1];
	HBITMAP hPriorityBitmaps[LFMaxRating+1];
	LFFont m_DefaultFont;
	LFFont m_ItalicFont;
	LFFont m_SmallFont;
	LFFont m_SmallBoldFont;
	LFFont m_LargeFont;
	LFFont m_CaptionFont;
	LFFont m_UACFont;
	LFFont m_DialogFont;
	LFFont m_DialogBoldFont;
	LFFont m_DialogItalicFont;
	UINT OSVersion;
	UINT m_TaskbarButtonCreated;
	UINT m_LicenseActivatedMsg;
	UINT m_SetProgressMsg;
	UINT m_WakeupMsg;
	GUID m_AppID;
	CLIPFORMAT CF_FILEDESCRIPTOR;
	CLIPFORMAT CF_FILECONTENTS;
	CLIPFORMAT CF_HLIQUID;
	CList<CWnd*> m_pMainFrames;
	LFUpdateDlg* m_pUpdateNotification;
	CIcons m_LargeAttributeIcons;
	CIcons m_SmallAttributeIcons;
	GLModelQuality m_ModelQuality;
	GLTextureQuality m_TextureQuality;
	BOOL m_TextureCompress;

	PFNSETWINDOWTHEME zSetWindowTheme;
	PFNOPENTHEMEDATA zOpenThemeData;
	PFNCLOSETHEMEDATA zCloseThemeData;
	PFNDRAWTHEMEBACKGROUND zDrawThemeBackground;
	PFNGETTHEMEPARTSIZE zGetThemePartSize;
	PFNISAPPTHEMED zIsAppThemed;
	BOOL m_ThemeLibLoaded;

	PFNDWMISCOMPOSITIONENABLED zDwmIsCompositionEnabled;
	PFNDWMSETWINDOWATTRIBUTE zDwmSetWindowAttribute;
	BOOL m_DwmLibLoaded;

	PFNSETCURRENTPROCESSEXPLICITAPPUSERMODELID zSetCurrentProcessExplicitAppUserModelID;
	BOOL m_ShellLibLoaded;

	PFNREGISTERAPPLICATIONRESTART zRegisterApplicationRestart;
	BOOL m_KernelLibLoaded;

	PFNCHANGEWINDOWMESSAGEFILTER zChangeWindowMessageFilter;
	BOOL m_UserLibLoaded;

	afx_msg void OnBackstagePurchase();
protected:
	afx_msg void OnBackstageEnterLicenseKey();
	afx_msg void OnBackstageSupport();
	afx_msg void OnBackstageAbout();
	afx_msg void OnUpdateBackstageCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	LFTooltip m_wndTooltip;
	LFDynArray<ResourceCacheItem, 16, 4> m_ResourceCache;

private:
	ULONG_PTR m_GdiPlusToken;
	HMODULE hModThemes;
	HMODULE hModDwm;
	HMODULE hModShell;
	HMODULE hModKernel;
	HMODULE hModUser;
	HANDLE hFontLetterGothic;
};

inline BOOL LFApplication::IsTooltipVisible() const
{
	ASSERT(IsWindow(m_wndTooltip));

	return m_wndTooltip.IsWindowVisible();
}

inline void LFApplication::HideTooltip()
{
	ASSERT(IsWindow(m_wndTooltip));

	m_wndTooltip.HideTooltip();
}
