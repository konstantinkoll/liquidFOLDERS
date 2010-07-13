// LFNamespaceExtension.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "LFNamespaceExtension.h"
#include "LFCore.h"
#include "afxsettingsstore.h"
#include <io.h>


LFNamespaceExtensionApp::LFNamespaceExtensionApp()
{
	//Version
	ZeroMemory(&osInfo, sizeof(OSVERSIONINFO));
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osInfo);

	// Dateiname mit Icons
	HMODULE hModCore = LoadLibrary("LFCore.DLL");
	if (hModCore)
	{
		::GetModuleFileName(hModCore, m_IconFile, MAX_PATH);
		FreeLibrary(hModCore);
	}
	else
	{
		strcpy_s(m_IconFile, MAX_PATH, "LFCore.DLL");
	}

	// Get attribute information
	for (UINT a=0; a<LFAttributeCount; a++)
		m_Attributes[a] = LFGetAttributeInfo(a);

	// Get domain information
	for (UINT a=0; a<LFDomainCount; a++)
		m_Domains[a] = LFGetDomainInfo(a);

	// Get item category information
	for (UINT a=0; a<LFItemCategoryCount; a++)
		m_ItemCategories[a] = LFGetItemCategoryInfo(a);

	// Get category names
	for (UINT a=0; a<LFAttrCategoryCount+1; a++)
		m_AttrCategoryNames[a] = LFGetAttrCategoryName(a);

	// Shell-API initalisieren
	CoInitialize(NULL);
}

LFNamespaceExtensionApp::~LFNamespaceExtensionApp()
{
	// Shell-API freigeben
	CoUninitialize();

	// Daten freigeben
	for (UINT a=0; a<LFDomainCount; a++)
		LFFreeDomainDescriptor(m_Domains[a]);
	for (UINT a=0; a<LFAttributeCount; a++)
		LFFreeAttributeDescriptor(m_Attributes[a]);
	for (UINT a=0; a<LFItemCategoryCount; a++)
		LFFreeItemCategoryDescriptor(m_ItemCategories[a]);
}


// Das einzige CStoreManagerApp-Objekt

LFNamespaceExtensionApp theApp;


// Versionsinfo

OSVERSIONINFO osInfo;


BOOL LFNamespaceExtensionApp::InitInstance()
{
	OleInitialize(NULL);
	COleObjectFactory::RegisterAll();

	CNSEFolder::RegisterExtensionData(_T("Name:KonstantinKoll*Company:BLUefolders*Email:ceo@bluefolders.net#Oo0m5Ouz+xz64KV57IinRTUvhkNojDZGjBd5MNXfwDEmgcr4baoQFMono3odGhqP"));

	if (!GetApplicationPath(_T("StoreManager"), PathStoreManager))
		PathStoreManager.Empty();
	if (!GetApplicationPath(_T("Migrate"), PathMigrate))
		PathMigrate.Empty();

	return CWinApp::InitInstance();
}

BOOL LFNamespaceExtensionApp::HideFileExt()
{
	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (reg.Open(_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced")))
	{
		DWORD hide;
		if (reg.Read(_T("HideFileExt"), hide))
			return hide;
	}

	return FALSE;
}

BOOL LFNamespaceExtensionApp::GetApplicationPath(CString App, CString& Path)
{
	// Registry
	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(TRUE, TRUE);

	if (reg.Open(_T("Software\\liquidFOLDERS\\")))
		if (reg.Read(App, Path))
			if (_access(Path, 0)==0)
				return TRUE;

	// Festen Pfad probieren
	char tmpStr[MAX_PATH];
	if (!SHGetSpecialFolderPath(NULL, tmpStr, CSIDL_PROGRAM_FILES, FALSE))
		return FALSE;

	Path = tmpStr;
	Path.Append(_T("\\liquidFOLDERS\\"));
	Path.Append(App);
	Path.Append(_T(".exe"));
	return (_access(Path, 0)==0);
}


// Registrierung mit dem Explorer

STDAPI DllRegisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
		return ResultFromScode(SELFREG_E_CLASS);

	return NOERROR;
}

STDAPI DllUnregisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
		return ResultFromScode(SELFREG_E_CLASS);

	return NOERROR;
}
