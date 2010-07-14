
#include "stdafx.h"
#include "CFileItem.h"
#include "CFolderItem.h"
#include "Commands.h"
#include "LFCore.h"
#include "liquidFOLDERS.h"
#include "resource.h"
#include <io.h>
#include <shlguid.h>


// CmdCreateNewStore
//

CmdCreateNewStore::CmdCreateNewStore()
{
	static const GUID GCreateNewStore = { 0x69e6720a, 0xbc5d, 0x408d, { 0xb2, 0x9, 0xc3, 0x4b, 0x2f, 0x4b, 0xaf, 0x5c } };
	guid = GCreateNewStore;
}

CString CmdCreateNewStore::GetCaption(CPtrList* /*nseItems*/)
{
	CString caption;
	ENSURE(caption.LoadString(IDS_MENU_CreateNewStore));
	caption.Remove('&');
	return caption;
}

CString CmdCreateNewStore::GetToolTip(CPtrList* /*nseItems*/)
{
	CString hint;
	ENSURE(hint.LoadString(IDS_HINT_CreateNewStore));
	return hint;
}

BOOL CmdCreateNewStore::Invoke(CPtrList* /*nseItems*/)
{
	return ((CFolderItem*)nseFolder)->OnCreateNewStore();
}

CString CmdCreateNewStore::GetIcon(CPtrList* /*nseItems*/)
{
	return _T("LFNamespaceExtension.dll,1");
}


// CmdStoreManager
//

CmdStoreManager::CmdStoreManager()
{
	static const GUID GStoreManager = { 0xb1078381, 0xb8f3, 0x4d0b, { 0x9a, 0x79, 0xc5, 0xbe, 0xc8, 0x60, 0xbd, 0xa7 } };
	guid = GStoreManager;
}

CString CmdStoreManager::GetCaption(CPtrList* /*nseItems*/)
{
	CString caption;
	ENSURE(caption.LoadString(IDS_MENU_StoreManager));
	caption.Remove('&');
	return caption;
}

CString CmdStoreManager::GetToolTip(CPtrList* /*nseItems*/)
{
	CString hint;
	ENSURE(hint.LoadString(IDS_HINT_StoreManager));
	return hint;
}

BOOL CmdStoreManager::Invoke(CPtrList* /*nseItems*/)
{
	return ((CFolderItem*)nseFolder)->OnStoreManager();
}

CString CmdStoreManager::GetIcon(CPtrList* /*nseItems*/)
{
	return _T("LFNamespaceExtension.dll,2");
}


// CmdMigrate
//

CmdMigrate::CmdMigrate()
{
	static const GUID GMigrate = { 0xa43a5a7a, 0xc10c, 0x4ef1, { 0x9a, 0x70, 0xd9, 0xc, 0x75, 0x35, 0xae, 0x68 } };
	guid = GMigrate;
}

CString CmdMigrate::GetCaption(CPtrList* /*nseItems*/)
{
	CString caption;
	ENSURE(caption.LoadString(IDS_MENU_Migrate));
	caption.Remove('&');
	return caption;
}

CString CmdMigrate::GetToolTip(CPtrList* /*nseItems*/)
{
	CString hint;
	ENSURE(hint.LoadString(IDS_HINT_Migrate));
	return hint;
}

BOOL CmdMigrate::Invoke(CPtrList* /*nseItems*/)
{
	return ((CFolderItem*)nseFolder)->OnMigrate();
}

CString CmdMigrate::GetIcon(CPtrList* /*nseItems*/)
{
	return _T("LFNamespaceExtension.dll,3");
}
