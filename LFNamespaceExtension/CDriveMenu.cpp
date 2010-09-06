
#include "stdafx.h"
#include "LFCore.h"
#include "LFNamespaceExtension.h"
#include "CDriveMenu.h"
#include "MenuIcons.h"
#include "resource.h"

#define HIDA_GetPIDLFolder(pida) (LPITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[0])
#define HIDA_GetPIDLItem(pida, i) (LPITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[i+1])


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
		BOOL ret = AfxOleRegisterServerClass(m_clsid, m_lpszProgID, m_lpszProgID, m_lpszProgID, OAT_DISPATCH_OBJECT);

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
	info.AddProgIDTarget(SpecialProgIDTargets_AllFolders);
	info.registryKeyName = _T("liquidFOLDERS.DriveMenu");
}

BOOL CDriveMenu::OnInitialize(LPDATAOBJECT dataObject)
{
	COleDataObject d;
	d.Attach(dataObject, FALSE);

	// Files
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
					if (strlen(szNextFile)==3)
						Drive = Drive ? '\2' : szNextFile[0];
		}

		GlobalUnlock(hG);
	}

	// Shell items
	IShellFolder* pDesktopPtr;
	if (SUCCEEDED(SHGetDesktopFolder(&pDesktopPtr)))
	{
		UINT CF_IDLIST = RegisterClipboardFormat(CFSTR_SHELLIDLIST);

		FORMATETC fetc;
		fetc.cfFormat = (CLIPFORMAT)CF_IDLIST;
		fetc.ptd = NULL;
		fetc.dwAspect = DVASPECT_CONTENT;
		fetc.lindex = -1;
		fetc.tymed = TYMED_HGLOBAL;

		STGMEDIUM stgm;
		if (d.GetData((CLIPFORMAT)CF_IDLIST, &stgm, &fetc))
		{
			LPIDA pIDList = (LPIDA)GlobalLock(stgm.hGlobal);
			if (pIDList)
			{
				for (UINT uItem=0; uItem<pIDList->cidl; uItem++)
				{
					LPITEMIDLIST pidl = HIDA_GetPIDLItem(pIDList, uItem);
					STRRET strret;
					if (SUCCEEDED(pDesktopPtr->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &strret)))
						if (wcscmp(strret.pOleStr, L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}")==0)
							Drive = '\1';
				}
			}

			GlobalUnlock(pIDList);
		}

		pDesktopPtr->Release();
	}

	d.Detach();
	return TRUE;
}

void CDriveMenu::OnGetMenuItems(CGetMenuitemsEventArgs& e)
{
	BOOL Add = (Drive=='\1');

	if ((Drive>='A') && (Drive<='Z'))
		Add |= (LFGetLogicalDrives(LFGLD_External) & (1<<(Drive-'A')));

	if (Add)
	{
		CString tmpStr;
		CString tmpHint;
		ENSURE(tmpStr.LoadString(IDS_MENU_CreateNewStore));
		ENSURE(tmpHint.LoadString(IDS_HINT_CreateNewStore));

		e.menu->AddItem(_T(""))->SetSeparator(TRUE);

		CShellMenuItem* item = e.menu->AddItem(tmpStr, _T(VERB_CREATENEWSTOREDRIVE), tmpHint);
		item->SetEnabled(!theApp.m_PathRunCmd.IsEmpty());

		HMODULE hModCore = LoadLibrary("LFCORE.DLL");
		if (hModCore)
		{
			int cx;
			int cy;
			theApp.GetIconSize(cx, cy);

			HICON hIcon = (HICON)LoadImage(hModCore, MAKEINTRESOURCE((Drive=='\1') ? IDI_STORE_Internal : IDI_STORE_Bag), IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);
			FreeLibrary(hModCore);

			item->SetBitmap(IconToBitmap(hIcon, cx, cy));
			DestroyIcon(hIcon);
		}
	}
}

BOOL CDriveMenu::OnExecuteMenuItem(CExecuteItemEventArgs& e)
{
	if (e.menuItem->GetVerb()==_T(VERB_CREATENEWSTOREDRIVE))
	{
		if ((Drive>='A') && (Drive<='Z'))
		{
			CString id(Drive);
			ShellExecute(NULL, "open", theApp.m_PathRunCmd, _T("NEWSTOREDRIVE ")+id, NULL, SW_SHOW);

			return TRUE;
		}

		if (Drive=='\1')
		{
			ShellExecute(NULL, "open", theApp.m_PathRunCmd, _T("NEWSTORE"), NULL, SW_SHOW);

			return TRUE;
		}
	}

	return FALSE;
}
