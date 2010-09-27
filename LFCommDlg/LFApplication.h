
// LFApplication.h: Hauptheaderdatei für die Anwendung
//

#pragma once
#include "resource.h"
#include "liquidFOLDERS.h"
#include "CGdiPlusBitmap.h"
#include <uxtheme.h>

#define ChildBackground_Black     0
#define ChildBackground_Ribbon    1
#define ChildBackground_System    2
#define ChildBackground_White     3

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
typedef HRESULT(__stdcall* PFNDRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, int iPartId,
							int iStateId, const RECT* pRect, const RECT* pClipRect);
typedef HRESULT(__stdcall* PFNDRAWTHEMETEXT)(HTHEME hTheme, HDC hdc, int iPartId,
							int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags,
							DWORD dwTextFlags2, const RECT* pRect);
typedef HRESULT(__stdcall* PFNDRAWTHEMETEXTEX)(HTHEME hTheme, HDC hdc, int iPartId,
							int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags,
							const RECT* pRect, const DTTOPTS* pOptions);
typedef HRESULT(__stdcall* PFNGETTHEMESYSFONT)(HTHEME hTheme, int iFontID, LOGFONT* plf);
typedef HRESULT(__stdcall* PFNGETTHEMESYSCOLOR)(HTHEME hTheme, int iColorID);
typedef HRESULT (__stdcall* PFNGETTHEMEPARTSIZE)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
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

	CString path;
	UINT m_nAppLook;
	LFMessageIDs* p_MessageIDs;
	LFAttributeDescriptor* m_Attributes[LFAttributeCount];
	wchar_t* m_AttrCategories[LFAttrCategoryCount];
	LFContextDescriptor* m_Contexts[LFContextCount];
	LFItemCategoryDescriptor* m_ItemCategories[LFItemCategoryCount];
	CImageList m_SystemImageListSmall;
	CImageList m_SystemImageListLarge;
	CImageList m_CoreImageListSmall;
	CImageList m_CoreImageListLarge;
	HBITMAP m_RatingBitmaps[LFMaxRating+1];
	HBITMAP m_PriorityBitmaps[LFMaxRating+1];
	CFont m_DefaultFont;
	CFont m_ItalicFont;
	CFont m_SmallFont;
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
	virtual int ExitInstance();
	virtual void SetApplicationLook(UINT nID);

	CString GetDefaultFontFace();
	void SendMail(CString Subject=_T(""));
	int GetGlobalInt(LPCTSTR lpszEntry, int nDefault=0);
	CString GetGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszDefault=_T(""));
	BOOL WriteGlobalInt(LPCTSTR lpszEntry, int nValue);
	BOOL WriteGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszValue);
	static void ExtractCoreIcons(HINSTANCE hModIcons, int size, CImageList* li);
	static UINT DeleteStore(LFItemDescriptor* store, CWnd* pParentWnd=NULL, CWnd* pOwnerWnd=NULL);
	static UINT DeleteStore(LFStoreDescriptor* store, CWnd* pParentWnd=NULL, CWnd* pOwnerWnd=NULL);
	static void PlayNavigateSound();
	static void PlayWarningSound();
	static void PlayTrashSound();
	static BOOL HideFileExt();
	static CString GetCommandName(UINT nID, BOOL bInsertSpace=FALSE);

	afx_msg void OnAppNewFileDrop();
	afx_msg void OnAppNewMigrate();
	afx_msg void OnAppNewStoreManager();

protected:
	CString GetGlobalRegPath();

	afx_msg void OnAppHelp();
	afx_msg void OnAppPurchase();
	afx_msg void OnAppEnterLicenseKey();
	afx_msg void OnAppSupport();
	afx_msg void OnAppPrompt();
	afx_msg void OnUpdateAppCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	ULONG_PTR m_gdiplusToken;
	HMODULE hModThemes;
	HMODULE hModAero;
};
