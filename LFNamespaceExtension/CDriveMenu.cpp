
// CDriveMenu.cpp: Implementierung der Klasse CDriveMenu
//

#include "stdafx.h"
#include "LFCore.h"
#include "LFNamespaceExtension.h"
#include "CDriveMenu.h"
#include "MenuIcons.h"
#include "resource.h"


IMPLEMENT_DYNCREATE(CDriveMenu, CContextMenuExtension)
IMPLEMENT_OLECREATE_EX(CDriveMenu, "LFNamespaceExtension.DriveMenu",
	0x3F2D914E, 0xFE57, 0x414F, 0x9F, 0x88, 0xA3, 0x77, 0xC7, 0x84, 0x1D, 0xA4)


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


// CDriveMenu
//

#define HIDA_GetPIDLFolder(pida) (LPITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[0])
#define HIDA_GetPIDLItem(pida, i) (LPITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[i+1])

CDriveMenu::CDriveMenu()
{
	Drive = L'\0';
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
			WCHAR szNextFile[MAX_PATH];

			for (UINT uFile=0; uFile<uNumFiles; uFile++)
				if (DragQueryFile(hDrop, uFile, szNextFile, MAX_PATH))
					if (wcslen(szNextFile)==3)
						Drive = Drive ? L'\2' : szNextFile[0];
		}

		GlobalUnlock(hG);
	}

	// Shell items
	IShellFolder* pDesktop = NULL;
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
					if (SUCCEEDED(pDesktop->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &strret)))
						if (wcscmp(strret.pOleStr, L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}")==0)
							Drive = L'\1';
				}
			}

			GlobalUnlock(pIDList);
		}

		pDesktop->Release();
	}

	d.Detach();
	return TRUE;
}

void CDriveMenu::OnGetMenuItems(CGetMenuitemsEventArgs& e)
{
	BOOL Add = (Drive==L'\1');

	if ((Drive>=L'A') && (Drive<=L'Z'))
		Add |= (LFGetLogicalDrives(LFGLD_External) & (1<<(Drive-L'A')));

	if (Add)
	{
		e.menu->AddItem(_T(""))->SetSeparator(TRUE);

		CString tmpStr;
		CString tmpHint;
		ENSURE(tmpStr.LoadString(IDS_MENU_CreateNewStore));
		ENSURE(tmpHint.LoadString(IDS_HINT_CreateNewStore));

		CShellMenuItem* item = e.menu->AddItem(tmpStr, _T(VERB_CREATENEWSTOREVOLUME), tmpHint);
		item->SetEnabled(!theApp.m_PathRunCmd.IsEmpty());

		HMODULE hModCore = LoadLibrary(_T("LFCORE.DLL"));
		if (hModCore)
		{
			INT cx;
			INT cy;
			theApp.GetIconSize(cx, cy);

			HICON hIcon = (HICON)LoadImage(hModCore, MAKEINTRESOURCE((Drive==L'\1') ? IDI_STORE_Internal : IDI_STORE_Bag), IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);
			FreeLibrary(hModCore);

			item->SetBitmap(IconToBitmap(hIcon, cx, cy));
			DestroyIcon(hIcon);
		}
	}
}

BOOL CDriveMenu::OnExecuteMenuItem(CExecuteItemEventArgs& e)
{
	if (e.menuItem->GetVerb()==_T(VERB_CREATENEWSTOREVOLUME))
	{
		if ((Drive>=L'A') && (Drive<=L'Z'))
		{
			CString id(Drive);
			ShellExecute(NULL, _T("open"), theApp.m_PathRunCmd, _T("NEWSTOREVOLUME ")+id, NULL, SW_SHOW);
			return TRUE;
		}

		if (Drive==L'\1')
		{
			ShellExecute(NULL, _T("open"), theApp.m_PathRunCmd, _T("NEWSTORE"), NULL, SW_SHOW);
			return TRUE;
		}
	}

	return FALSE;
}
