
// LFNamespaceExtension.h : deklariert die Initalisierung der DLL
//

#pragma once
#include "resource.h"
#include "liquidFOLDERS.h"
#include <objbase.h>
#include <shlguid.h>


// LFNamespaceExtensionApp
//

#define LFNamespaceExtensionVersion   2

#define NAG_NOTLICENSED               0
#define NAG_EXPIRED                   1
#define NAG_COUNTER                   0
#define NAG_FORCE                     2

class LFNamespaceExtensionApp : public CWinApp
{
public:
	LFNamespaceExtensionApp();
	~LFNamespaceExtensionApp();

	virtual BOOL InitInstance();

	static void GetIconSize(INT& cx, INT& cy);
	static CString FrmtAttrStr(CString Mask, CString Name);
	UINT ImportFiles(CHAR* StoreID, IDataObject* pDataObject, BOOL Move);
	BOOL ShowNagScreen(UINT Level, BOOL Abort=FALSE);

	WCHAR m_PathCoreFile[MAX_PATH];
	WCHAR m_PathThisFile[MAX_PATH];
	CString m_PathRunCmd;
	CString m_PathStoreManager;
	CString m_Categories[3][6];
	CString m_Store;
	CString m_Folder;
	LFAttributeDescriptor* m_Attributes[LFAttributeCount];
	LFItemCategoryDescriptor* m_ItemCategories[LFItemCategoryCount];
	WCHAR* m_AttrCategoryNames[LFAttrCategoryCount+1];
	UINT m_NagCounter;

protected:
	static BOOL GetApplicationPath(CString App, CString& Path);
};

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
extern LFNamespaceExtensionApp theApp;
extern OSVERSIONINFO osInfo;
