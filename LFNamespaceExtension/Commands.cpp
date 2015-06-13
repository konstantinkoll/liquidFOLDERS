
// Commands.cpp: Implementierung der Klassen für Toolbar-Befehle
//

#include "stdafx.h"
#include "CFolderItem.h"
#include "Commands.h"
#include "LFNamespaceExtension.h"


// CmdCreateNewStore
//

CmdCreateNewStore::CmdCreateNewStore()
{
	static const GUID GCreateNewStore = { 0x69E6720A, 0xBC5D, 0x408D, { 0xB2, 0x09, 0xC3, 0x4B, 0x2F, 0x4B, 0xAF, 0x5C } };
	guid = GCreateNewStore;
}

CString CmdCreateNewStore::GetCaption(CPtrList* /*nseItems*/)
{
	CString Caption((LPCSTR)IDS_MENU_ADDSTORE);
	Caption.Remove('&');

	return Caption;
}

CString CmdCreateNewStore::GetToolTip(CPtrList* /*nseItems*/)
{
	CString Hint((LPCSTR)IDS_HINT_ADDSTORE);

	return Hint;
}

ExplorerCommandState CmdCreateNewStore::GetState(CPtrList* /*nseItems*/)
{
	return !theApp.m_AppPath.IsEmpty() ? ECS_Enabled : ECS_Disabled;
}

BOOL CmdCreateNewStore::Invoke(CPtrList* /*nseItems*/)
{
	return RunPath(NULL, theApp.m_AppPath, _T("/ADDSTORE"));
}

CString CmdCreateNewStore::GetIcon(CPtrList* /*nseItems*/)
{
	CString tmpStr(theApp.m_PathThisFile);
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
	CString Caption((LPCSTR)IDS_MENU_PROPERTIES);
	Caption.Remove('&');

	return Caption;
}

CString CmdProperties::GetToolTip(CPtrList* /*nseItems*/)
{
	CString Hint((LPCSTR)IDS_HINT_PROPERTIES);

	return Hint;
}

ExplorerCommandState CmdProperties::GetState(CPtrList* nseItems)
{
	return ((nseItems->GetCount()==1) && (!theApp.m_AppPath.IsEmpty())) ? ECS_Enabled : ECS_Disabled;
}

BOOL CmdProperties::Invoke(CPtrList* nseItems)
{
	POSITION pos = nseItems->GetHeadPosition();
	if (pos)
	{
		CNSEItem* item = (CNSEItem*)nseItems->GetNext(pos);
		if (IS(item, CFolderItem))
		{
			CString id(AS(item, CFolderItem)->Attrs.StoreID);
			ShellExecute(NULL, _T("open"), theApp.m_AppPath, _T("/STOREPROPERTIES ")+id, NULL, SW_SHOW);
			return TRUE;
		}
	}

	return FALSE;
}

CString CmdProperties::GetIcon(CPtrList* /*nseItems*/)
{
	CString tmpStr(theApp.m_PathThisFile);
	tmpStr.Append(_T(",1"));

	return tmpStr;
}


// CmdFileDrop
//

CmdFileDrop::CmdFileDrop(CHAR* StoreID)
{
	static const GUID GFileDrop = { 0xDFDFF069, 0xF883, 0x43BC, { 0x85, 0xD3, 0x4C, 0x17, 0x6B, 0xE1, 0xAA, 0x2D } };
	guid = GFileDrop;

	strcpy_s(m_StoreID, LFKeySize, StoreID);
}

CString CmdFileDrop::GetCaption(CPtrList* /*nseItems*/)
{
	CString Caption((LPCSTR)IDS_MENU_OPENFILEDROP);
	Caption.Remove('&');

	return Caption;
}

CString CmdFileDrop::GetToolTip(CPtrList* /*nseItems*/)
{
	CString Hint((LPCSTR)IDS_HINT_OPENFILEDROP);

	return Hint;
}

ExplorerCommandState CmdFileDrop::GetState(CPtrList* /*nseItems*/)
{
	return !theApp.m_AppPath.IsEmpty() ? ECS_Enabled : ECS_Disabled;
}

BOOL CmdFileDrop::Invoke(CPtrList* /*nseItems*/)
{
	CString id(m_StoreID);
	ShellExecute(NULL, _T("open"), theApp.m_AppPath, _T("/FILEDROP ")+id, NULL, SW_SHOW);

	return TRUE;
}

CString CmdFileDrop::GetIcon(CPtrList* /*nseItems*/)
{
	CString tmpStr(theApp.m_AppPath);
	tmpStr.Append(_T(",2"));

	return tmpStr;
}


// CmdImportFolder
//

CmdImportFolder::CmdImportFolder(CHAR* StoreID)
{
	static const GUID GImport = { 0x9957AD33, 0x1BC7, 0x4809, { 0xAB, 0x09, 0xF0, 0xEB, 0x5D, 0xBB, 0x23, 0xE8 } };
	guid = GImport;

	strcpy_s(m_StoreID, LFKeySize, StoreID);
}

CString CmdImportFolder::GetCaption(CPtrList* /*nseItems*/)
{
	CString Caption((LPCSTR)IDS_MENU_IMPORTFOLDER);
	Caption.Remove('&');

	return Caption;
}

CString CmdImportFolder::GetToolTip(CPtrList* /*nseItems*/)
{
	CString Hint((LPCSTR)IDS_HINT_IMPORTFOLDER);

	return Hint;
}

ExplorerCommandState CmdImportFolder::GetState(CPtrList* /*nseItems*/)
{
	return !theApp.m_AppPath.IsEmpty() ? ECS_Enabled : ECS_Disabled;
}

BOOL CmdImportFolder::Invoke(CPtrList* /*nseItems*/)
{
	CString id(m_StoreID);
	ShellExecute(NULL, _T("open"), theApp.m_AppPath, _T("/IMPORTFOLDER ")+id, NULL, SW_SHOW);

	return TRUE;
}

CString CmdImportFolder::GetIcon(CPtrList* /*nseItems*/)
{
	CString tmpStr(theApp.m_PathThisFile);
	tmpStr.Append(_T(",2"));

	return tmpStr;
}
