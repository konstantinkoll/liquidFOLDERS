
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
#define OS_Eight                3
#define OS_Ten                  4

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


// LFApplication
//

class LFUpdateDlg;

class LFApplication : public CWinAppEx
{
public:
	LFApplication(GUID& AppID);
	virtual ~LFApplication();

	virtual BOOL InitInstance();
	virtual CWnd* OpenCommandLine(LPWSTR CmdLine=NULL);
	virtual INT ExitInstance();

	void AddFrame(CWnd* pFrame);
	void KillFrame(CWnd* pVictim);

	void SendMail(const CString& Subject=_T("")) const;

	void GetBinary(LPCTSTR lpszEntry, LPVOID pData, UINT Size);
	INT GetGlobalInt(LPCTSTR lpszEntry, INT nDefault=0) const;
	CString GetGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszDefault=_T("")) const;
	BOOL WriteGlobalInt(LPCTSTR lpszEntry, INT nValue) const;
	BOOL WriteGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszValue) const;

	Bitmap* GetResourceImage(UINT nID) const;
	Bitmap* GetCachedResourceImage(UINT nID);
	static HICON LoadDialogIcon(UINT nID);
	static HANDLE LoadFontFromResource(UINT nID);
	static void LoadColorDots(CIcons& Icons, INT Size);
	static void LoadColorDots(CIcons& Icons, const LFFont& Font);
	static void ExtractCoreIcons(HINSTANCE hModIcons, INT Size, CImageList* pImageList);

	void AttributeToString(CString& Name, CString& Value, const LFItemDescriptor* pItemDescriptor, UINT Attr) const;
	static CString GetFreeBytesAvailable(INT64 FreeBytesAvailable);
	static CString GetFreeBytesAvailable(const LFStoreDescriptor& StoreDescriptor);
	CString GetHintForItem(const LFItemDescriptor* pItemDescriptor, LPCWSTR pFormatName=NULL) const;
	CString GetHintForStore(const LFStoreDescriptor& StoreDescriptor) const;
	void ShowTooltip(const CWnd* pWndOwner, CPoint point, const CString& Caption, const CString& Hint, HICON hIcon=NULL, HBITMAP hBitmap=NULL);
	void ShowTooltip(const CWnd* pWndOwner, const CPoint& point, const LFStoreDescriptor& StoreDescriptor);
	void HideTooltip(const CWnd* pWndOwner=NULL);

	LPCWSTR GetAttributeName(UINT Attr, UINT Context=LFContextAllFiles) const;
	INT GetAttributeIcon(UINT Attr, UINT Context=LFContextAllFiles) const;
	BOOL IsAttributeAlwaysVisible(UINT Attr) const;
	BOOL IsAttributeAdvertised(INT Context, UINT Attr) const;
	BOOL IsAttributeAvailable(INT Context, UINT Attr) const;
	BOOL IsAttributeBucket(UINT Attr) const;
	BOOL IsAttributeEditable(UINT Attr) const;
	BOOL IsAttributeFormatRight(UINT Attr) const;
	BOOL IsAttributeSortable(INT Context, UINT Attr, INT SubfolderAttribute=-1) const;
	BOOL IsAttributeSortableInSubfolder(UINT Attr) const;
	BOOL IsAttributeSortDescending(UINT Context, UINT Attr) const;
	static BOOL IsPlaceholderIcon(UINT IconID);
	BOOL IsViewAllowed(INT Context, INT View) const;
	BOOL ShowRepresentativeThumbnail(UINT Attr, UINT Context) const;
	BOOL ShowRepresentativeThumbnail(INT SubfolderAttribute, UINT Context) const;

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
	void WriteUpdateSettings(BOOL EnableAutoUpdate, INT Interval) const;
	void CheckForUpdate(BOOL Force=FALSE, CWnd* pParentWnd=NULL);

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
	CLIPFORMAT CF_LIQUIDFILES;
	CList<CWnd*> m_pMainFrames;
	LFUpdateDlg* m_pUpdateNotification;
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
	const CWnd* p_WndTooltipOwner;
	LFDynArray<ResourceCacheItem, 16, 4> m_ResourceCache;

private:
	void AppendAttribute(CString& Str, const LFItemDescriptor* pItemDescriptor, UINT Attr) const;
	static void PlayRegSound(const CString& Identifier);

	BOOL IsUpdateCheckDue() const;
	void WriteUpdateCheckTime() const;
	static CString GetLatestVersion(CString CurrentVersion);
	static CString GetIniValue(CString Ini, const CString& Name);
	static void ParseVersion(const CString& tmpStr, LFVersion* pVersion);
	static BOOL IsVersionLater(const LFVersion& LatestVersion, const LFVersion& CurrentVersion);
	static BOOL IsUpdateFeatureLater(const CString& VersionIni, const CString& Name, LFVersion& CurrentVersion);
	static DWORD GetUpdateFeatures(const CString& VersionIni, LFVersion& CurrentVersion);

	ULONG_PTR m_GdiPlusToken;
	HMODULE hModThemes;
	HMODULE hModDwm;
	HMODULE hModShell;
	HMODULE hModKernel;
	HMODULE hModUser;
	HANDLE hFontDinMittelschrift;
	static const INT m_ColorDotSizes[4];
};

inline void LFApplication::LoadColorDots(CIcons& Icons, const LFFont& Font)
{
	LoadColorDots(Icons, Font.GetFontHeight());
}

inline CString LFApplication::GetFreeBytesAvailable(const LFStoreDescriptor& StoreDescriptor)
{
	return GetFreeBytesAvailable(StoreDescriptor.FreeBytesAvailable.QuadPart);
}

inline void LFApplication::ShowTooltip(const CWnd* pWndOwner, const CPoint& point, const LFStoreDescriptor& StoreDescriptor)
{
	ShowTooltip(pWndOwner, point, StoreDescriptor.StoreName, GetHintForStore(StoreDescriptor));
}

inline BOOL LFApplication::IsAttributeAlwaysVisible(UINT Attr) const
{
	ASSERT(Attr<LFAttributeCount);

	return (m_Attributes[Attr].AttrProperties.DataFlags & LFDataAlwaysVisible);
}

inline BOOL LFApplication::IsAttributeAdvertised(INT Context, UINT Attr) const
{
	ASSERT(Context>=0);
	ASSERT(Context<LFContextCount);
	ASSERT(Attr<LFAttributeCount);

	return (m_Contexts[Context].CtxProperties.AdvertisedAttributes>>Attr) & 1;
}

inline BOOL LFApplication::IsAttributeAvailable(INT Context, UINT Attr) const
{
	ASSERT(Context>=0);
	ASSERT(Context<LFContextCount);
	ASSERT(Attr<LFAttributeCount);

	return (m_Contexts[Context].CtxProperties.AvailableAttributes>>Attr) & 1;
}

inline BOOL LFApplication::IsAttributeBucket(UINT Attr) const
{
	ASSERT(Attr<LFAttributeCount);

	return (m_Attributes[Attr].TypeProperties.DataFlags | m_Attributes[Attr].AttrProperties.DataFlags) & LFDataBucket;
}

inline BOOL LFApplication::IsAttributeEditable(UINT Attr) const
{
	ASSERT(Attr<LFAttributeCount);

	return (m_Attributes[Attr].AttrProperties.DataFlags & LFDataEditable);
}

inline BOOL LFApplication::IsAttributeFormatRight(UINT Attr) const
{
	ASSERT(Attr<LFAttributeCount);

	return (m_Attributes[Attr].TypeProperties.DataFlags & LFDataFormatRight);
}

inline BOOL LFApplication::IsAttributeSortable(INT Context, UINT Attr, INT SubfolderAttribute) const
{
	ASSERT(Context>=0);
	ASSERT(Context<LFContextCount);
	ASSERT(Attr<LFAttributeCount);

	return ((INT)Attr!=SubfolderAttribute) &&
		((SubfolderAttribute!=LFAttrMediaCollection) || (Attr!=LFAttrCreator)) &&
		(!LFIsSubfolderContext(Context) || IsAttributeSortableInSubfolder(Attr));
}

inline BOOL LFApplication::IsAttributeSortableInSubfolder(UINT Attr) const
{
	ASSERT(Attr<LFAttributeCount);

	return (m_Attributes[Attr].TypeProperties.DataFlags & LFDataSortableInSubfolder);
}

inline BOOL LFApplication::IsPlaceholderIcon(UINT IconID)
{
	return IconID>=IDI_FLD_PLACEHOLDER_DEFAULT;
}

inline BOOL LFApplication::IsViewAllowed(INT Context, INT View) const
{
	ASSERT(Context>=0);
	ASSERT(Context<LFContextCount);
	ASSERT(View>=0);
	ASSERT(View<=31);

	return (m_Contexts[Context].CtxProperties.AvailableViews>>View) & 1;
}

inline BOOL LFApplication::ShowRepresentativeThumbnail(UINT Attr, UINT Context) const
{
	ASSERT(Attr<LFAttributeCount);

	return (GetAttributeIcon(Attr, Context)>=IDI_FIRSTPLACEHOLDERICON) && (m_Attributes[Attr].AttrProperties.DataFlags & LFDataShowRepresentativeThumbnail);
}

inline BOOL LFApplication::ShowRepresentativeThumbnail(INT SubfolderAttribute, UINT Context) const
{
	return (SubfolderAttribute>=0) && (SubfolderAttribute!=LFAttrCreator) && ShowRepresentativeThumbnail((UINT)SubfolderAttribute, Context);
}
