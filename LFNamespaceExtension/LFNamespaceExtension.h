
// LFNamespaceExtension.h : deklariert die Initalisierung der DLL
//

#pragma once
#include "resource.h"
#include "liquidFOLDERS.h"
#include <objbase.h>
#include <shlguid.h>


// LFNamespaceExtensionApp
//

#define LFNamespaceExtensionVersion   1

class LFNamespaceExtensionApp : public CWinApp
{
public:
	LFNamespaceExtensionApp();
	~LFNamespaceExtensionApp();

	virtual BOOL InitInstance();

	BOOL HideFileExt();
	void GetIconSize(int& cx, int& cy);
	void SetCoreMenuIcon(void* item, UINT ResID);

	CString m_PathRunCmd;
	CString m_PathStoreManager;
	CString m_PathMigrate;
	CString m_PathFileDrop;
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
