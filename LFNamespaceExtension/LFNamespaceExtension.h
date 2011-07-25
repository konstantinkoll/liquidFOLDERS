
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

class LFNamespaceExtensionApp : public CWinApp
{
public:
	LFNamespaceExtensionApp();
	~LFNamespaceExtensionApp();

	virtual BOOL InitInstance();

	static BOOL HideFileExt();
	static void GetIconSize(INT& cx, INT& cy);
	static void SetCoreMenuIcon(void* item, UINT ResID);
	static CString FrmtAttrStr(CString Mask, CString Name);
	static UINT ImportFiles(CHAR* StoreID, IDataObject* pDataObject, BOOL Move);
	void ShowNagscreen();

	CString m_PathRunCmd;
	CString m_PathStoreManager;
	CString m_PathMigrate;
	CString m_PathFileDrop;
	CString m_Categories[3][6];
	CString m_Store;
	CString m_Folder;
	WCHAR m_CoreFile[MAX_PATH];
	WCHAR m_ThisFile[MAX_PATH];
	LFAttributeDescriptor* m_Attributes[LFAttributeCount];
	LFDomainDescriptor* m_Domains[LFDomainCount];
	LFItemCategoryDescriptor* m_ItemCategories[LFItemCategoryCount];
	WCHAR* m_AttrCategoryNames[LFAttrCategoryCount+1];
	UINT NagCounter;

protected:
	static BOOL GetApplicationPath(CString App, CString& Path);
};

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
extern LFNamespaceExtensionApp theApp;
extern OSVERSIONINFO osInfo;
