
// LFNamespaceExtension.h : deklariert die Initalisierung der DLL
//

#pragma once
#include "resource.h"
#include "LFCore.h"
#include <objbase.h>
#include <shlguid.h>


// LFNamespaceExtensionApp
//

#define LFNSE_VERSION       2

#define NAG_NOTLICENSED     0
#define NAG_EXPIRED         1
#define NAG_COUNTER         0
#define NAG_FORCE           2

class LFNamespaceExtensionApp : public CWinApp
{
public:
	LFNamespaceExtensionApp();
	~LFNamespaceExtensionApp();

	virtual BOOL InitInstance();

	static CString FrmtAttrStr(CString Mask, CString Name);
	UINT ImportFiles(CHAR* StoreID, IDataObject* pDataObject, BOOL Move);
	BOOL ShowNagScreen(UINT Level, BOOL Abort=FALSE);

	WCHAR m_PathCoreFile[MAX_PATH];
	WCHAR m_PathThisFile[MAX_PATH];
	CString m_AppPath;
	CString m_Categories[3][6];
	CString m_Store;
	CString m_Folder;
	WCHAR m_AttrCategoryNames[LFAttrCategoryCount][256];
	LFAttributeDescriptor m_Attributes[LFAttributeCount];
	LFItemCategoryDescriptor m_ItemCategories[LFItemCategoryCount];
	UINT m_NagCounter;

protected:
	static BOOL GetApplicationPath(CString& Path);
};

extern LFNamespaceExtensionApp theApp;
extern OSVERSIONINFO osInfo;
