
#pragma once
#include "resource.h"
#include "liquidFOLDERS.h"
#include <objbase.h>
#include <shlguid.h>

#define LFNamespaceExtensionVersion   1

class LFNamespaceExtensionApp : public CWinApp
{
public:
	LFNamespaceExtensionApp();
	~LFNamespaceExtensionApp();

	virtual BOOL InitInstance();

	BOOL HideFileExt();

	CString m_PathRunCmd;
	CString m_PathStoreManager;
	CString m_PathMigrate;
	TCHAR m_CoreFile[MAX_PATH];
	TCHAR m_ThisFile[MAX_PATH];
	LFAttributeDescriptor* m_Attributes[LFAttributeCount];
	LFDomainDescriptor* m_Domains[LFDomainCount];
	LFItemCategoryDescriptor* m_ItemCategories[LFItemCategoryCount];
	wchar_t* m_AttrCategoryNames[LFAttrCategoryCount+1];

protected:
	BOOL GetApplicationPath(CString App, CString& Path);
};

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
extern LFNamespaceExtensionApp theApp;
extern OSVERSIONINFO osInfo;
