
#include "stdafx.h"
#include "LFCore.h"
#include "LFNamespaceExtension.h"
#include "CDriveMenu.h"
#include "resource.h"


IMPLEMENT_DYNCREATE(CDriveMenu, CContextMenuExtension)

// The GUID and ProgID of the shell extension
IMPLEMENT_OLECREATE_EX(CDriveMenu, _T("LFNamespaceExtension.DriveMenu"),
	0x3f2d914e, 0xfe57, 0x414f, 0x9f, 0x88, 0xa3, 0x77, 0xc7, 0x84, 0x1d, 0xa4)


// This function is called when you register the shell extension dll file
// using the regsvr32.exe or similar utility

//The classfactory is nested in your class and has a name formed
//by concatenating the class name with "Factory".
BOOL CDriveMenu::CDriveMenuFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
	{ 
		BOOL ret = AfxOleRegisterServerClass(m_clsid, m_lpszProgID,
			m_lpszProgID, m_lpszProgID, OAT_DISPATCH_OBJECT);
		
		// Register the shell extension
		CContextMenuExtension::RegisterExtension(RUNTIME_CLASS(CDriveMenu));
		return ret; 
	}
	else
	{
		// Unregister the shell extension
		CContextMenuExtension::UnregisterExtension(RUNTIME_CLASS(CDriveMenu));
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
	}
}


// Class CDriveMenu
//

CDriveMenu::CDriveMenu()
{
	Drive = '\0';
}

void CDriveMenu::GetExtensionTargetInfo(CExtensionTargetInfo& info)
{
	info.AddProgIDTarget(SpecialProgIDTargets_AllDrives);
	info.registryKeyName = _T("liquidFOLDERS.DriveMenu");
}

BOOL CDriveMenu::OnInitialize(LPDATAOBJECT dataObject)
{
	COleDataObject d;
	d.Attach(dataObject, FALSE);

	HGLOBAL hG = d.GetGlobalData(CF_HDROP);
	if (hG)
	{
		HDROP hDrop = (HDROP)GlobalLock(hG);
		if (hDrop)
		{
			UINT uNumFiles = DragQueryFile(hDrop, (UINT)-1, NULL, 0);
			char szNextFile[MAX_PATH];

			for (UINT uFile=0; uFile<uNumFiles; uFile++)
				if (DragQueryFile(hDrop, uFile, szNextFile, MAX_PATH))
					Drive = Drive ? '\1' : szNextFile[0];
		}

		GlobalUnlock(hG);
	}

	d.Detach();
	return TRUE;
}

void CDriveMenu::OnGetMenuItems(CGetMenuitemsEventArgs& e)
{
	if ((Drive>='A') && (Drive<='Z'))
	{
		UINT Drives = LFGetLogicalDrives(LFGLD_External);
		if (Drives & (1<<(Drive-'A')))
		{
			CString tmpStr;
			CString tmpHint;
			ENSURE(tmpStr.LoadString(IDS_MENU_CreateNewStore));
			ENSURE(tmpHint.LoadString(IDS_HINT_CreateNewStore));

			e.menu->AddItem(_T(""))->SetSeparator(TRUE);
			e.menu->AddItem(tmpStr, _T(VERB_CREATENEWSTOREDRIVE), tmpHint)->SetEnabled(!theApp.m_PathRunCmd.IsEmpty());
		}
	}
}

BOOL CDriveMenu::OnExecuteMenuItem(CExecuteItemEventArgs& e)
{
	if (e.menuItem->GetVerb()==_T(VERB_CREATENEWSTOREDRIVE) && (Drive>='A') && (Drive<='Z'))
	{
		CString id(Drive);
		ShellExecute(NULL, "open", theApp.m_PathRunCmd, _T("NEWSTOREDRIVE ")+id, NULL, SW_SHOW);

		return TRUE;
	}

	return FALSE;
}
