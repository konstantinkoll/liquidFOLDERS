
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

#define RATINGBITMAPWIDTH      88
#define RATINGBITMAPHEIGHT     15

#define OS_XP                   0
#define OS_Vista                1
#define OS_Seven                2

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
	virtual CWnd* OpenCommandLine(LPCWSTR CmdLine=NULL);
	virtual INT ExitInstance();

	void AddFrame(CWnd* pFrame);
	void KillFrame(CWnd* pVictim);
	void SendMail(const CString& Subject=_T("")) const;
	INT GetGlobalInt(LPCTSTR lpszEntry, INT nDefault=0) const;
	CString GetGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszDefault=_T("")) const;
	BOOL WriteGlobalInt(LPCTSTR lpszEntry, INT nValue) const;
	BOOL WriteGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszValue) const;
	Bitmap* GetResourceImage(UINT nID) const;
	Bitmap* GetCachedResourceImage(UINT nID);
	static HICON LoadDialogIcon(UINT nID);
	static HANDLE LoadFontFromResource(UINT nID);
	static void ExtractCoreIcons(HINSTANCE hModIcons, INT Size, CImageList* pImageList);
	void AttributeToString(CString& Name, CString& Value, LFItemDescriptor* pItemDescriptor, UINT Attr) const;
	CString GetHintForItem(LFItemDescriptor* pItemDescriptor, LPCWSTR pFormatName=NULL) const;
	CString GetHintForStore(LFStoreDescriptor* pStoreDescriptor) const;
	void ShowTooltip(CWnd* pCallerWnd, CPoint point, const CString& Caption, const CString& Hint, HICON hIcon=NULL, HBITMAP hBitmap=NULL);
	void ShowTooltip(CWnd* pCallerWnd, CPoint point, LFStoreDescriptor* pStoreDescriptor);
	BOOL IsTooltipVisible() const;
	void HideTooltip();
	INT LoadAttributeIconsSmall();
	INT LoadAttributeIconsLarge();
	void ExecuteExplorerContextMenu(CHAR Drive, LPCSTR Verb);
	static void PlayAsteriskSound();
	static void PlayDefaultSound();
	static void PlayErrorSound();
	static void PlayNavigateSound();
	static void PlayNotificationSound();
	static void PlayQuestionSound();
	static void PlayTrashSound();
	static void PlayWarningSound();
	void GetUpdateSettings(BOOL& EnableAutoUpdate, INT& Interval) const;
	void SetUpdateSettings(BOOL EnableAutoUpdate, INT Interval) const;
	BOOL IsUpdateCheckDue() const;
	void GetBinary(LPCTSTR lpszEntry, void* pData, UINT Size);

	const LFMessageIDs* p_MessageIDs;
	WCHAR m_AttrCategoryNames[LFAttrCategoryCount][256];
	LFAttributeDescriptor m_Attributes[LFAttributeCount];
	LFAttributeList m_SortedAttributeList;
	LFContextDescriptor m_Contexts[LFContextCount];
	LFItemCategoryDescriptor m_ItemCategories[LFItemCategoryCount];
	WCHAR m_SourceNames[LFSourceCount][2][256];
	INT m_ExtraLargeIconSize;
	CImageList m_SystemImageListSmall;
	CImageList m_SystemImageListExtraLarge;
	CImageList m_SystemImageListJumbo;
	CImageList m_CoreImageListSmall;
	CImageList m_CoreImageListExtraLarge;
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
	SmoothingMode m_SmoothingModeAntiAlias8x8;
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
	CIcons m_AttributeIconsLarge;
	CIcons m_AttributeIconsSmall;
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
	void AppendAttribute(CString& Str, LFItemDescriptor* pItemDescriptor, UINT Attr) const;
	static void PlayRegSound(const CString& Identifier);

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

inline INT LFApplication::LoadAttributeIconsSmall()
{
	return m_AttributeIconsSmall.LoadSmall(IDB_ATTRIBUTEICONS_16);
}

inline INT LFApplication::LoadAttributeIconsLarge()
{
	return m_AttributeIconsLarge.Load(IDB_ATTRIBUTEICONS_16, LI_FORTOOLTIPS);
}
