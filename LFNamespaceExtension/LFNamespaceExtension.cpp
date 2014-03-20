
// LFNamespaceExtension.cpp : definiert die Initalisierung der DLL
//

#include "stdafx.h"
#include "LFNamespaceExtension.h"
#include "LFCore.h"
#include "..\\LFCore\\resource.h"
#include "MenuIcons.h"
#include "afxsettingsstore.h"
#include <eznamespaceextensions.h>
#include <ezshellextensions.h>
#include <io.h>


// LFNamespaceExtensionApp
//

#define ResetNagCounter     m_NagCounter = 0;

LFNamespaceExtensionApp::LFNamespaceExtensionApp()
{
	//Version
	ZeroMemory(&osInfo, sizeof(OSVERSIONINFO));
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osInfo);

	// Dateinamen mit Icons
	GetModuleFileName((HINSTANCE)&__ImageBase, m_ThisFile, MAX_PATH);

	HMODULE hModCore = GetModuleHandle(_T("LFCORE.DLL"));
	if (hModCore)
	{
		GetModuleFileName(hModCore, m_CoreFile, MAX_PATH);

		ENSURE(m_Categories[2][0].LoadString(hModCore, IDS_Size1));

		for (UINT a=1; a<6; a++)
		{
			ENSURE(m_Categories[0][a].LoadString(hModCore, IDS_Rating1+a-1));
			ENSURE(m_Categories[1][a].LoadString(hModCore, IDS_Priority1+a-1));
			ENSURE(m_Categories[2][a].LoadString(hModCore, IDS_Size1+a));
		}
	}
	else
	{
		wcscpy_s(m_CoreFile, MAX_PATH, L"LFCORE.DLL");

		for (UINT a=1; a<6; a++)
			m_Categories[0][a] = m_Categories[1][a] = m_Categories[2][a] = _T("?");
	}

	// Get attribute information
	for (UINT a=0; a<LFAttributeCount; a++)
		m_Attributes[a] = LFGetAttributeInfo(a);

	// Get item category information
	for (UINT a=0; a<LFItemCategoryCount; a++)
		m_ItemCategories[a] = LFGetItemCategoryInfo(a);

	// Get category names
	for (UINT a=0; a<LFAttrCategoryCount+1; a++)
		m_AttrCategoryNames[a] = LFGetAttrCategoryName(a);

	// Shell-API initalisieren
	CoInitialize(NULL);

	// Link-Datei schreiben
	LFCreateSendTo();
}

LFNamespaceExtensionApp::~LFNamespaceExtensionApp()
{
	// Shell-API freigeben
	CoUninitialize();

	// Daten freigeben
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

	// liquidFOLDERS initalisieren
	LFInitialize();

	// Lizenz
	EZNamespaceExtensionsMFC::CNSEFolder::RegisterExtensionData(_T("Name:KonstantinKoll*Company:BLUefolders*Email:ceo@bluefolders.net#Oo0m5Ouz+xz64KV57IinRTUvhkNojDZGjBd5MNXfwDEmgcr4baoQFMono3odGhqP"));
	EZShellExtensionsMFC::CExtensionTargetInfo::RegisterExtensionData(_T("Name:KonstantinKoll*Company:BLUefolders*Email:ceo@bluefolders.net#B2/22Ctegy/B3wHN28jR2uUsStzxt2RNPvEEmoFUuY4XGheEmCPKFVrhwK823NwN"));

	// Pfade
	if (!GetApplicationPath(_T("LFRunCmd"), m_PathRunCmd))
		m_PathRunCmd.Empty();
	if (!GetApplicationPath(_T("StoreManager"), m_PathStoreManager))
		m_PathStoreManager.Empty();
	if (!GetApplicationPath(_T("Migrate"), m_PathMigrate))
		m_PathMigrate.Empty();

	// Strings
	ENSURE(m_Store.LoadString(IDS_Store));
	ENSURE(m_Folder.LoadString(IDS_Folder));

	CString sortStr;
	ENSURE(sortStr.LoadString(IDS_NULLFOLDER_NameMask));

	m_Categories[0][0] = FrmtAttrStr(sortStr, CString(m_Attributes[LFAttrRating]->Name));
	m_Categories[1][0] = FrmtAttrStr(sortStr, CString(m_Attributes[LFAttrPriority]->Name));

	ResetNagCounter;

	return CWinApp::InitInstance();
}

BOOL LFNamespaceExtensionApp::GetApplicationPath(CString App, CString& Path)
{
	// Registry
	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(TRUE, TRUE);

	if (reg.Open(_T("Software\\liquidFOLDERS\\")))
		if (reg.Read(App, Path))
			if (_waccess(Path, 0)==0)
				return TRUE;

	// Modulpfad probieren
	Path = theApp.m_ThisFile;
	INT pos = Path.ReverseFind('\\');
	if (pos)
		Path = Path.Left(pos+1);

	Path.Append(App);
	Path.Append(_T(".exe"));
	if (_waccess(Path, 0)==0)
		return TRUE;

	// Festen Pfad probieren
	WCHAR tmpStr[MAX_PATH];
	if (!SHGetSpecialFolderPath(NULL, tmpStr, CSIDL_PROGRAM_FILES, FALSE))
		return FALSE;

	Path = tmpStr;
	Path.Append(_T("\\liquidFOLDERS\\"));
	Path.Append(App);
	Path.Append(_T(".exe"));
	return (_waccess(Path, 0)==0);
}

void LFNamespaceExtensionApp::GetIconSize(INT& cx, INT& cy)
{
	cx = GetSystemMetrics((osInfo.dwMajorVersion<6) ? SM_CXMENUCHECK : SM_CXSMICON);
	cy = GetSystemMetrics((osInfo.dwMajorVersion<6) ? SM_CYMENUCHECK : SM_CYSMICON);
}

CString LFNamespaceExtensionApp::FrmtAttrStr(CString Mask, CString Name)
{
	if ((Mask[0]=='L') && (Name[0]>='A') && (Name[0]<='Z') && (Name[1]>'Z'))
		Name = Name.MakeLower().Mid(0,1)+Name.Mid(1, Name.GetLength()-1);

	CString tmpStr;
	tmpStr.Format(Mask.Mid(1, Mask.GetLength()-1), Name);
	return tmpStr;
}

UINT LFNamespaceExtensionApp::ImportFiles(CHAR* StoreID, IDataObject* pDataObject, BOOL Move)
{
	// Allowed?
	if (ShowNagScreen(NAG_EXPIRED | NAG_FORCE, TRUE))
		return DROPEFFECT_NONE;

	// Data object
	COleDataObject dobj;
	dobj.Attach(pDataObject, FALSE);

	CLIPFORMAT CF_HLIQUID = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_LIQUIDFILES);

	HGLOBAL hgDrop = dobj.GetGlobalData(CF_HDROP);
	HGLOBAL hgLiquid = dobj.GetGlobalData(CF_HLIQUID);

	if ((hgDrop==NULL) && (hgLiquid==NULL))
		return DROPEFFECT_NONE;

	// Wenn Default-Store gewünscht: verfügbar ?
	if (StoreID[0]=='\0')
		if (!LFDefaultStoreAvailable())
		{
			LFErrorBox(LFNoDefaultStore, GetForegroundWindow());
			return DROPEFFECT_NONE;
		}

	// Importieren
	if (hgLiquid)
	{
		HLIQUID hLiquid = (HLIQUID)GlobalLock(hgLiquid);
		LFFileIDList* il = LFAllocFileIDList(hLiquid);
		GlobalUnlock(hgLiquid);

		LFTransactionImport(StoreID, il, Move==TRUE);
		UINT res = il->m_LastError;
		LFErrorBox(res, GetForegroundWindow());

		// CF_LIQUIDFILES neu setzen, um nicht veränderte Dateien (Fehler oder Drop auf denselben Store) zu entfernen
		FORMATETC fmt;
		ZeroMemory(&fmt, sizeof(fmt));
		fmt.cfFormat = CF_HLIQUID;
		fmt.dwAspect = DVASPECT_CONTENT;
		fmt.lindex = -1;
		fmt.tymed = TYMED_HGLOBAL;

		STGMEDIUM stg;
		stg.tymed = TYMED_HGLOBAL;
		stg.hGlobal = LFCreateLiquidFiles(il);

		pDataObject->SetData(&fmt, &stg, FALSE);

		LFFreeFileIDList(il);

		return (res==LFOk) ? Move ? DROPEFFECT_MOVE : DROPEFFECT_COPY : DROPEFFECT_NONE;
	}
	else
	{
		HDROP hDrop = (HDROP)GlobalLock(hgDrop);
		LFFileImportList* il = LFAllocFileImportList(hDrop);
		GlobalUnlock(hgDrop);

		// Import
		UINT res = LFOk;
		if (TRUE)
		{
			LFTransactionImport(StoreID, il, NULL, true, Move==TRUE);
			res = il->m_LastError;
			LFErrorBox(res, GetForegroundWindow());
		}

		LFFreeFileImportList(il);

		return (res==LFOk) ? Move ? DROPEFFECT_MOVE : DROPEFFECT_COPY : DROPEFFECT_NONE;
	}
}

BOOL LFNamespaceExtensionApp::ShowNagScreen(UINT Level, BOOL Abort)
{
	if ((Level & NAG_EXPIRED) ? LFIsSharewareExpired() : !LFIsLicensed())
		if ((Level & NAG_FORCE) || (++m_NagCounter)>5)
		{
			CString tmpStr;
			ENSURE(tmpStr.LoadString(IDS_NoLicense));

			MessageBox(GetForegroundWindow(), tmpStr, _T("liquidFOLDERS"), Abort ? (MB_OK | MB_ICONSTOP) : (MB_OK | MB_ICONINFORMATION));
			ResetNagCounter;

			return TRUE;
		}

	return FALSE;
}


// Registrierung mit dem Explorer
//

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
