
// Commands.cpp: Implementierung der Klassen für Toolbar-Befehle
//

#include "stdafx.h"
#include "CFileItem.h"
#include "CFolderItem.h"
#include "Commands.h"
#include "LFCore.h"
#include "LFNamespaceExtension.h"
#include "liquidFOLDERS.h"
#include "resource.h"
#include <io.h>
#include <shlguid.h>


// CmdImportFolder
//

CmdImportFolder::CmdImportFolder()
{
	static const GUID GImport = { 0x9957AD33, 0x1BC7, 0x4809, { 0xAB, 0x09, 0xF0, 0xEB, 0x5D, 0xBB, 0x23, 0xE8 } };
	guid = GImport;
}

CString CmdImportFolder::GetCaption(CPtrList* /*nseItems*/)
{
	CString caption;
	ENSURE(caption.LoadString(IDS_MENU_ImportFolder));
	caption.Remove('&');
	return caption;
}

CString CmdImportFolder::GetToolTip(CPtrList* /*nseItems*/)
{
	CString hint;
	ENSURE(hint.LoadString(IDS_HINT_ImportFolder));
	return hint;
}

ExplorerCommandState CmdImportFolder::GetState(CPtrList* nseItems)
{
	return ((nseItems->GetCount()==1) && (!theApp.m_PathRunCmd.IsEmpty())) ? ECS_Enabled : ECS_Disabled;
}

BOOL CmdImportFolder::Invoke(CPtrList* nseItems)
{
	POSITION pos = nseItems->GetHeadPosition();
	if (pos)
	{
		CNSEItem* item = (CNSEItem*)nseItems->GetNext(pos);
		if (IS(item, CFolderItem))
		{
			CString id = AS(item, CFolderItem)->data.StoreID;
			ShellExecute(NULL, "open", theApp.m_PathRunCmd, _T("IMPORTFOLDER ")+id, NULL, SW_SHOW);
			return TRUE;
		}
	}

	return FALSE;
}

CString CmdImportFolder::GetIcon(CPtrList* /*nseItems*/)
{
	CString tmpStr(theApp.m_ThisFile);
	tmpStr.Append(_T(",0"));

	return tmpStr;
}


// CmdProperties
//

CmdProperties::CmdProperties()
{
	static const GUID GProperties = { 0x8FF9154A, 0x5432, 0x40FA, { 0xBE, 0xE0, 0x1E, 0xAF, 0xFF, 0xD8, 0x3F, 0xDB } };
	guid = GProperties;
}

CString CmdProperties::GetCaption(CPtrList* /*nseItems*/)
{
	CString caption;
	ENSURE(caption.LoadString(IDS_MENU_Properties));
	caption.Remove('&');
	return caption;
}

CString CmdProperties::GetToolTip(CPtrList* /*nseItems*/)
{
	CString hint;
	ENSURE(hint.LoadString(IDS_HINT_Properties));
	return hint;
}

ExplorerCommandState CmdProperties::GetState(CPtrList* nseItems)
{
	return ((nseItems->GetCount()==1) && (!theApp.m_PathRunCmd.IsEmpty())) ? ECS_Enabled : ECS_Disabled;
}

BOOL CmdProperties::Invoke(CPtrList* nseItems)
{
	POSITION pos = nseItems->GetHeadPosition();
	if (pos)
	{
		CNSEItem* item = (CNSEItem*)nseItems->GetNext(pos);
		if (IS(item, CFolderItem))
		{
			CString id = AS(item, CFolderItem)->data.StoreID;
			ShellExecute(NULL, "open", theApp.m_PathRunCmd, _T("STOREPROPERTIES ")+id, NULL, SW_SHOW);
			return TRUE;
		}
	}

	return FALSE;
}

CString CmdProperties::GetIcon(CPtrList* /*nseItems*/)
{
	CString tmpStr(theApp.m_ThisFile);
	tmpStr.Append(_T(",1"));

	return tmpStr;
}


// CmdCreateNewStore
//

CmdCreateNewStore::CmdCreateNewStore()
{
	static const GUID GCreateNewStore = { 0x69E6720A, 0xBC5D, 0x408D, { 0xB2, 0x09, 0xC3, 0x4B, 0x2F, 0x4B, 0xAF, 0x5C } };
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

ExplorerCommandState CmdCreateNewStore::GetState(CPtrList* /*nseItems*/)
{
	return (!theApp.m_PathRunCmd.IsEmpty()) ? ECS_Enabled : ECS_Disabled;
}

BOOL CmdCreateNewStore::Invoke(CPtrList* /*nseItems*/)
{
	return ((CFolderItem*)nseFolder)->OnCreateNewStore();
}

CString CmdCreateNewStore::GetIcon(CPtrList* /*nseItems*/)
{
	CString tmpStr(theApp.m_ThisFile);
	tmpStr.Append(_T(",2"));

	return tmpStr;
}


// CmdStoreManager
//

CmdStoreManager::CmdStoreManager()
{
	static const GUID GStoreManager = { 0xB1078381, 0xB8F3, 0x4D0B, { 0x9A, 0x79, 0xC5, 0xBE, 0xC8, 0x60, 0xBD, 0xA7 } };
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

ExplorerCommandState CmdStoreManager::GetState(CPtrList* /*nseItems*/)
{
	return (!theApp.m_PathStoreManager.IsEmpty()) ? ECS_Enabled : ECS_Disabled;
}

BOOL CmdStoreManager::Invoke(CPtrList* /*nseItems*/)
{
	return ((CFolderItem*)nseFolder)->OnStoreManager();
}

CString CmdStoreManager::GetIcon(CPtrList* /*nseItems*/)
{
	CString tmpStr(theApp.m_ThisFile);
	tmpStr.Append(_T(",3"));

	return tmpStr;
}


// CmdMigrate
//

CmdMigrate::CmdMigrate()
{
	static const GUID GMigrate = { 0xA43A5A7A, 0xC10C, 0x4EF1, { 0x9A, 0x70, 0xD9, 0x0C, 0x75, 0x35, 0xAE, 0x68 } };
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

ExplorerCommandState CmdMigrate::GetState(CPtrList* /*nseItems*/)
{
	return (!theApp.m_PathMigrate.IsEmpty()) ? ECS_Enabled : ECS_Disabled;
}

BOOL CmdMigrate::Invoke(CPtrList* /*nseItems*/)
{
	return ((CFolderItem*)nseFolder)->OnMigrate();
}

CString CmdMigrate::GetIcon(CPtrList* /*nseItems*/)
{
	CString tmpStr(theApp.m_ThisFile);
	tmpStr.Append(_T(",4"));

	return tmpStr;
}
