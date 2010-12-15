
// LFApplication.h: Hauptheaderdatei für die Anwendung
//

#pragma once
#include "resource.h"
#include "liquidFOLDERS.h"
#include "CGdiPlusBitmap.h"
#include <uxtheme.h>

#define RatingBitmapWidth        88
#define RatingBitmapHeight       15

#define HasGUI_None               0
#define HasGUI_Standard           1
#define HasGUI_Ribbon             2

#define OS_XP                     0
#define OS_Vista                  1
#define OS_Seven                  2

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

typedef HRESULT(__stdcall* PFNDWMISCOMPOSITIONENABLED)(BOOL* pfEnabled);
typedef HRESULT(__stdcall* PFNDWMEXTENDFRAMEINTOCLIENTAREA)(HWND hWnd, const MARGINS* pMarInset);
typedef BOOL(__stdcall* PFNDWMDEFWINDOWPROC)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* plResult);


// LFApplication:
// Siehe LFApplication.cpp für die Implementierung dieser Klasse
//

class AFX_EXT_CLASS LFApplication : public CWinAppEx
{
public:
	LFApplication(UINT _HasGUI);
	virtual ~LFApplication();

	CString m_Path;
	LFMessageIDs* p_MessageIDs;
	LFAttributeDescriptor* m_Attributes[LFAttributeCount];
	WCHAR* m_AttrCategories[LFAttrCategoryCount];
	LFContextDescriptor* m_Contexts[LFContextCount];
	LFItemCategoryDescriptor* m_ItemCategories[LFItemCategoryCount];
	CImageList m_SystemImageListSmall;
	CImageList m_SystemImageListLarge;
	CImageList m_SystemImageListExtraLarge;
	CImageList m_SystemImageListJumbo;
	CImageList m_CoreImageListSmall;
	CImageList m_CoreImageListLarge;
	CImageList m_CoreImageListExtraLarge;
	CImageList m_CoreImageListJumbo;
	HBITMAP m_RatingBitmaps[LFMaxRating+1];
	HBITMAP m_PriorityBitmaps[LFMaxRating+1];
	CFont m_DefaultFont;
	CFont m_ItalicFont;
	CFont m_SmallFont;
	CFont m_LargeFont;
	CFont m_CaptionFont;
	LFMessageIDs* MessageIDs;
	BOOL IsLicensed;
	UINT HasGUI;
	UINT OSVersion;

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
	PFNISTHEMEACTIVE zIsThemeActive;
	BOOL m_ThemeLibLoaded;

	PFNDWMISCOMPOSITIONENABLED zDwmIsCompositionEnabled;
	PFNDWMEXTENDFRAMEINTOCLIENTAREA zDwmExtendFrameIntoClientArea;
	PFNDWMDEFWINDOWPROC zDwmDefWindowProc;
	BOOL m_AeroLibLoaded;

	virtual BOOL InitInstance();
	virtual INT ExitInstance();

	CString GetDefaultFontFace();
	void SendMail(CString Subject=_T(""));
	INT GetGlobalInt(LPCTSTR lpszEntry, INT nDefault=0);
	CString GetGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszDefault=_T(""));
	BOOL WriteGlobalInt(LPCTSTR lpszEntry, INT nValue);
	BOOL WriteGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszValue);
	static void ExtractCoreIcons(HINSTANCE hModIcons, INT size, CImageList* li);
	static UINT DeleteStore(LFItemDescriptor* store, CWnd* pParentWnd=NULL, CWnd* pOwnerWnd=NULL);
	static UINT DeleteStore(LFStoreDescriptor* store, CWnd* pParentWnd=NULL, CWnd* pOwnerWnd=NULL);
	static void PlayNavigateSound();
	static void PlayWarningSound();
	static void PlayTrashSound();
	static BOOL HideFileExt();

	afx_msg void OnAppNewFileDrop();
	afx_msg void OnAppNewMigrate();
	afx_msg void OnAppNewStoreManager();

protected:
	CString GetGlobalRegPath();

	afx_msg void OnAppHelp();
	afx_msg void OnAppPurchase();
	afx_msg void OnAppEnterLicenseKey();
	afx_msg void OnAppSupport();
	afx_msg void OnUpdateAppCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	ULONG_PTR m_gdiplusToken;
	HMODULE hModThemes;
	HMODULE hModAero;
};
