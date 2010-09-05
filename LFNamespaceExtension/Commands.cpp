
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
	static const GUID GProperties = { 0x8ff9154a, 0x5432, 0x40fa, { 0xbe, 0xe0, 0x1e, 0xaf, 0xff, 0xd8, 0x3f, 0xdb } };
	guid = GProperties;
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
	static const GUID GProperties = { 0x8ff9154a, 0x5432, 0x40fa, { 0xbe, 0xe0, 0x1e, 0xaf, 0xff, 0xd8, 0x3f, 0xdb } };
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
