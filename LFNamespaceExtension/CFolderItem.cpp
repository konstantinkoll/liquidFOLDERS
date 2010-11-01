
#include "stdafx.h"
#include "LFNamespaceExtension.h"
#include "CFileItem.h"
#include "CFolderItem.h"
#include "Commands.h"
#include "LFCore.h"
#include "Categorizer.h"
#include "MenuIcons.h"
#include "afxsettingsstore.h"
#include <io.h>
#include <shlguid.h>
#include <shlobj.h>


CShellMenuItem* InsertItem(CShellMenu* menu, UINT ResID, CString verb, int pos=0)
{
	CString tmpStr;
	CString tmpHint;
	ENSURE(tmpStr.LoadString(ResID));
	ENSURE(tmpHint.LoadString(ResID+1));

	return menu->InsertItem(tmpStr, verb, tmpHint, pos);
}

void AddSeparator(CShellMenu* menu)
{
	menu->AddItem("")->SetSeparator(TRUE);
}

CShellMenuItem* AddItem(CShellMenu* menu, UINT ResID, CString verb)
{
	CString tmpStr;
	CString tmpHint;
	ENSURE(tmpStr.LoadString(ResID));
	ENSURE(tmpHint.LoadString(ResID+1));

	return menu->AddItem(tmpStr, verb, tmpHint);
}

void AddPathItem(CShellMenu* menu, UINT ResID, CString verb, CString path, UINT IconID)
{
	int cx;
	int cy;
	theApp.GetIconSize(cx, cy);

	CShellMenuItem* item =AddItem(menu, ResID, verb);
	item->SetEnabled(!path.IsEmpty());

	HICON hIcon = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IconID), IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);
	item->SetBitmap(IconToBitmap(hIcon, cx, cy));
	DestroyIcon(hIcon);
}

BOOL RunPath(HWND hWnd, CString path, CString parameter)
{
	if (!path.IsEmpty())
	{
		ShellExecute(hWnd, _T("open"), path, parameter, NULL, SW_SHOW);
		return TRUE;
	}

	return FALSE;
}


IMPLEMENT_DYNCREATE(CFolderItem, CNSEFolder)
IMPLEMENT_OLECREATE_EX(CFolderItem, _T("LFNamespaceExtension.RootFolder"),
	0x3F2D914F, 0xFE57, 0x414F, 0x9F, 0x88, 0xA3, 0x77, 0xC7, 0x84, 0x1D, 0xA4)


// The classfactory is nested in your class and has a name formed
// by concatenating the class name with "Factory".
BOOL CFolderItem::CFolderItemFactory::UpdateRegistry(BOOL bRegister)
{
	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(TRUE, FALSE);

	if (bRegister)
	{
		BOOL ret = AfxOleRegisterServerClass(m_clsid, m_lpszProgID, m_lpszProgID, m_lpszProgID, OAT_DISPATCH_OBJECT);

		// Register the namespace extension
		CNSEFolder::RegisterExtension(RUNTIME_CLASS(CFolderItem));

		// Move to delegate folders
		reg.DeleteKey(_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{3F2D914F-FE57-414F-9F88-A377C7841DA4}"), TRUE);

		if (reg.Open(_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\DelegateFolders")))
			reg.CreateKey(_T("{3F2D914F-FE57-414F-9F88-A377C7841DA4}"));

		return ret;
	}
	else
	{
		// Unregister the namespace extension
		CNSEFolder::UnregisterExtension(RUNTIME_CLASS(CFolderItem));

		// Remove delegate folder
		reg.DeleteKey(_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\DelegateFolders\\{3F2D914F-FE57-414F-9F88-A377C7841DA4}"), TRUE);

		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
	}
}


// CFolderItem
//

// IPersistFolder

CFolderItem::CFolderItem()
{
	Attrs.Level = LevelRoot;
}

CFolderItem::CFolderItem(FolderSerialization& _Attrs)
{
	Attrs = _Attrs;
}

void CFolderItem::GetCLSID(LPCLSID pCLSID)
{
	*pCLSID = guid;
}


// Registration

void CFolderItem::GetExtensionTargetInfo(CExtensionTargetInfo& info)
{
	CNSETargetInfo* nti = new CNSETargetInfo();
	nti->nseTarget = NSET_MyComputer;
	nti->name = _T("liquidFOLDERS");
	nti->infoTip.LoadString(IDS_InfoTip);
	nti->attributes = (NSEItemAttributes)(NSEIA_CFOLDERITEM | NSEIA_HasSubFolder);
	nti->iconFile = theApp.m_CoreFile;
	nti->iconIndex = IDI_FLD_Default-1;
	nti->AddRootNodeProperty(_T("SortOrderIndex"), (UINT)64);
	nti->AddRootNodeProperty(_T("System.ItemType"), _T("Folder"));
	nti->AddRootNodeProperty(_T("System.ItemTypeText"), nti->infoTip);
	nti->AddRootNodeProperty(_T("System.PerceivedType"), (UINT)8);
	nti->AddRootNodeProperty(_T("System.PropList.DetailsPaneNullSelect"), _T("prop:"));
	nti->AddRootNodeProperty(_T("System.PropList.DetailsPaneNullSelectTitle"), _T("prop:~System.ItemNameDisplay;~System.ItemTypeText"));
	nti->AddRootNodeProperty(_T("System.PropList.PreviewTitle"), _T("prop:~System.ItemNameDisplay;~System.ItemTypeText"));
	info.AddTarget(nti);

	nti = new CNSETargetInfo();
	nti->nseTarget = NSET_Desktop;
	nti->name = _T("liquidFOLDERS");
	nti->infoTip.LoadString(IDS_InfoTip);
	nti->attributes = (NSEItemAttributes)(NSEIA_CFOLDERITEM | NSEIA_HasSubFolder);
	nti->iconFile = theApp.m_CoreFile;
	nti->iconIndex = IDI_FLD_Default-1;
	nti->AddRootNodeProperty(_T("SortOrderIndex"), (UINT)64);
	nti->AddRootNodeProperty(_T("System.ItemType"), _T("Folder"));
	nti->AddRootNodeProperty(_T("System.ItemTypeText"), nti->infoTip);
	nti->AddRootNodeProperty(_T("System.PerceivedType"), (UINT)8);
	nti->AddRootNodeProperty(_T("System.PropList.DetailsPaneNullSelect"), _T("prop:"));
	nti->AddRootNodeProperty(_T("System.PropList.DetailsPaneNullSelectTitle"), _T("prop:~System.ItemNameDisplay;~System.ItemTypeText"));
	nti->AddRootNodeProperty(_T("System.PropList.PreviewTitle"), _T("prop:~System.ItemNameDisplay;~System.ItemTypeText"));
	info.AddTarget(nti);
}


// PIDL handling

void CFolderItem::Serialize(CArchive& ar)
{
	ar << (BYTE)LFNamespaceExtensionVersion;
	ar << (BYTE)1;
	ar << Attrs.Level;
	ar << Attrs.Icon;
	ar << Attrs.Type;
	ar << Attrs.CategoryID;
	ar << Attrs.DisplayName;
	ar << Attrs.Description;
	ar << Attrs.Comment;
	ar << Attrs.FileID;
	ar << Attrs.StoreID;
	ar << Attrs.DomainID;
	ar << Attrs.Compare;
	ar.Write(&Attrs.Value, sizeof(LFVariantData));
	ar << Attrs.CreationTime.dwHighDateTime;
	ar << Attrs.CreationTime.dwLowDateTime;
	ar << Attrs.FileTime.dwHighDateTime;
	ar << Attrs.FileTime.dwLowDateTime;
	ar << Attrs.Count;
	ar << Attrs.Size;
	ar << Attrs.Format;
}

CNSEItem* CFolderItem::DeserializeChild(CArchive& ar)
{
	BYTE version;
	ar >> version;

	if (version!=LFNamespaceExtensionVersion)
		return NULL;

	BYTE ItemType;
	ar >> ItemType;

	CString StoreID;
	UINT Size;
	LFCoreAttributes Attrs;
	ZeroMemory(&Attrs, sizeof(LFCoreAttributes));

	switch (ItemType)
	{
	case 0:
		ar >> StoreID;
		ar >> Size;
		ar.Read(&Attrs, min(Size, sizeof(LFCoreAttributes)));

		return new CFileItem(StoreID, &Attrs);
	case 1:
		FolderSerialization d;
		ar >> d.Level;
		ar >> d.Icon;
		ar >> d.Type;
		ar >> d.CategoryID;
		ar >> d.DisplayName;
		ar >> d.Description;
		ar >> d.Comment;
		ar >> d.FileID;
		ar >> d.StoreID;
		ar >> d.DomainID;
		ar >> d.Compare;
		ar.Read(&d.Value, sizeof(LFVariantData));
		ar >> d.CreationTime.dwHighDateTime;
		ar >> d.CreationTime.dwLowDateTime;
		ar >> d.FileTime.dwHighDateTime;
		ar >> d.FileTime.dwLowDateTime;
		ar >> d.Count;
		ar >> d.Size;
		ar >> d.Format;

		return new CFolderItem(d);
	}

	return NULL;
}


// IEnumIDList

BOOL CFolderItem::GetChildren(CGetChildrenEventArgs& e)
{
	LFFilter* f = NULL;
	LFSearchResult* base = NULL;
	LFSearchResult* res = NULL;

	switch (Attrs.Level)
	{
	case LevelRoot:
		res = LFQuery(NULL);
		break;
	case LevelStores:
		f = LFAllocFilter();
		f->Mode = LFFilterModeStoreHome;
		f->HideEmptyDomains = true;
		strcpy_s(f->StoreID, LFKeySize, (LPCTSTR)Attrs.StoreID);
		res = LFQuery(f);
		break;
	case LevelStoreHome:
		if (e.childrenType & NSECT_Folders)
		{
			CString sortStr;
			ENSURE(sortStr.LoadString(IDS_AttributeComment));

			FolderSerialization d = { 0 };
			d.Level = Attrs.Level+2;
			d.Icon = IDI_FLD_All;
			d.Type = LFTypeVirtual;
			d.CategoryID = LFAttrCategoryCount;
			d.DisplayName.LoadString(IDS_AllFiles);
			d.Comment.LoadString(IDS_AllFilesComment);
			d.StoreID = Attrs.StoreID;
			d.FileID = "ALL";
			d.DomainID = Attrs.DomainID;
			d.Format.LoadString(IDS_Folder);

			e.children->AddTail(new CFolderItem(d));

			for (UINT a=0; a<LFAttributeCount; a++)
				if (theApp.m_Domains[Attrs.DomainID]->ImportantAttributes->IsSet(a))
				{
					FolderSerialization d = { 0 };
					d.Level = Attrs.Level+1;
					d.Icon = theApp.m_Attributes[a]->IconID;
					d.Type = LFTypeVirtual;
					d.CategoryID = theApp.m_Attributes[a]->Category;
					d.DisplayName = theApp.m_Attributes[a]->Name;
					d.Comment = theApp.FrmtAttrStr(sortStr, CString(theApp.m_Attributes[a]->Name));
					d.StoreID = Attrs.StoreID;
					d.FileID.Format(_T("%d"), a);
					d.DomainID = Attrs.DomainID;
					d.Compare = LFFilterCompareSubfolder;
					d.Format.LoadString(IDS_Folder);

					e.children->AddTail(new CFolderItem(d));
				}
		}
		break;
	case LevelAttribute:
		f = LFAllocFilter();
		f->Mode = LFFilterModeDirectoryTree;
		strcpy_s(f->StoreID, LFKeySize, (LPCTSTR)Attrs.StoreID);
		f->DomainID = (unsigned char)Attrs.DomainID;
		base = LFQuery(f);
		res = LFGroupSearchResult(base, atoi(Attrs.FileID), false, false, Attrs.Icon, atoi(Attrs.FileID)!=LFAttrFileName, f);
		break;
	case LevelAttrValue:
		f = LFAllocFilter();
		f->Mode = LFFilterModeDirectoryTree;
		strcpy_s(f->StoreID, LFKeySize, (LPCTSTR)Attrs.StoreID);
		f->DomainID = (unsigned char)Attrs.DomainID;
		f->ConditionList = LFAllocFilterCondition();
		f->ConditionList->Next = NULL;
		f->ConditionList->Compare = Attrs.Compare;
		f->ConditionList->AttrData = Attrs.Value;
		res = LFQuery(f);
		break;
	}

	if (res)
	{
		if (res->m_LastError!=LFOk)
		{
			LFErrorBox(res->m_LastError);
			LFFreeSearchResult(res);
			return FALSE;
		}

		UINT NullCount = 0;
		INT64 NullSize = 0;

		for (UINT a=0; a<res->m_ItemCount; a++)
		{
			LFItemDescriptor* i = res->m_Items[a];

			if ((((i->Type & LFTypeMask)==LFTypeStore) || (((i->Type & LFTypeMask)==LFTypeVirtual) &&
				(i->CategoryID!=LFItemCategoryHousekeeping))) && (e.childrenType & NSECT_Folders))
			{
				FolderSerialization d = { 0 };
				d.Level = Attrs.Level+1;
				d.Icon = i->IconID;
				d.Type = i->Type;
				d.CategoryID = i->CategoryID;
				d.DisplayName = i->CoreAttributes.FileName;
				d.Description = i->Description;
				d.Comment = i->CoreAttributes.Comment;
				d.StoreID = i->StoreID;
				d.FileID = i->CoreAttributes.FileID;
				d.DomainID = Attrs.DomainID;
				d.Count = i->AggregateCount;
				d.Size = i->CoreAttributes.FileSize;
				d.Format.LoadString(Attrs.Level==LevelRoot ? IDS_Store : IDS_Folder);

				switch (Attrs.Level)
				{
				case LevelRoot:
					d.CreationTime = i->CoreAttributes.CreationTime;
					d.FileTime = i->CoreAttributes.FileTime;
					break;
				case LevelStores:
					d.DomainID = atoi(d.FileID);
					break;
				case LevelAttribute:
					if (i->NextFilter)
						if (i->NextFilter->ConditionList)
						{
							d.Compare = i->NextFilter->ConditionList->Compare;
							d.Value = i->NextFilter->ConditionList->AttrData;
						}
				}

				e.children->AddTail(new CFolderItem(d));
			}

			if ((i->Type & LFTypeMask)==LFTypeFile)
				if ((Attrs.Level==LevelAttribute) && (atoi(Attrs.FileID)!=LFAttrFileName))
				{
					NullCount++;
					NullSize += i->CoreAttributes.FileSize;
				}
				else
					if (e.childrenType & NSECT_NonFolders)
						e.children->AddTail(new CFileItem(i->StoreID, &i->CoreAttributes));
		}
		LFFreeSearchResult(res);

		if ((NullCount) && (e.childrenType & NSECT_Folders))
		{
			UINT attr = atoi(Attrs.FileID);

			FolderSerialization d = { 0 };
			d.Level = Attrs.Level+1;
			d.Icon = IDI_FLD_Default;
			d.Type = LFTypeVirtual;
			d.CategoryID = LFAttrCategoryCount;
			d.StoreID = Attrs.StoreID;
			d.FileID = "NULL";
			d.DomainID = Attrs.DomainID;
			d.Count = NullCount;
			d.Size = NullSize;
			d.Compare = LFFilterCompareIsNull;
			d.Value.Attr = attr;
			d.Format.LoadString(IDS_Folder);

			CString tmpStr;
			ENSURE(tmpStr.LoadString(IDS_NULLFOLDER_NameMask));
			d.DisplayName = theApp.FrmtAttrStr(tmpStr, CString(theApp.m_Attributes[attr]->Name));
			ENSURE(tmpStr.LoadString(IDS_NULLFOLDER_CommentMask));
			d.Comment = theApp.FrmtAttrStr(tmpStr, CString(theApp.m_Attributes[attr]->Name));
			ENSURE(tmpStr.LoadString(NullCount==1 ? IDS_FILES_Singular : IDS_FILES_Plural));
			d.Description.Format(tmpStr, NullCount);

			e.children->AddTail(new CFolderItem(d));
		}
	}

	if (f)
		LFFreeFilter(f);

	LFFreeSearchResult(base);
	return TRUE;
}

BOOL CFolderItem::IsValid()
{
	if (Attrs.Level==LevelRoot)
		return TRUE;

	LFStoreDescriptor store;
	char key[LFKeySize];
	strcpy_s(key, LFKeySize, Attrs.StoreID);

	if (LFGetStoreSettings(key, &store)!=LFOk)
		return FALSE;

	if (Attrs.Level>LevelStores)
		return TRUE;

	wchar_t wsz[256];
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)Attrs.DisplayName, -1, wsz, 256);
	if (wcscmp(wsz, store.StoreName)!=0)
		return FALSE;
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)Attrs.Comment, -1, wsz, 256);
	if (wcscmp(wsz, store.Comment)!=0)
		return FALSE;

	return (memcmp(&Attrs.CreationTime, &store.CreationTime, sizeof(FILETIME))==0) &&
		(memcmp(&Attrs.FileTime, &store.FileTime, sizeof(FILETIME))==0);
}


// IMoniker

void CFolderItem::GetDisplayName(CString& displayName)
{
	displayName = Attrs.DisplayName;
}

void CFolderItem::GetDisplayNameEx(CString& displayName, DisplayNameFlags flags)
{
	displayName = Attrs.DisplayName;

	// If a fully qualified parsing name is requested, return the full path
	if ((flags & (NSEDNF_ForParsing | NSEDNF_ForAddressBar))==NSEDNF_ForParsing)
		if (!(flags & NSEDNF_InFolder))
		{
			WCHAR buf[39];
			StringFromGUID2(guid, buf, 39);
			CString id(buf);
			displayName = id;

			if (Attrs.Level>LevelRoot)
				displayName += '\\'+Attrs.StoreID;
		}
		else
		{
			if (Attrs.Level>LevelRoot)
				displayName = Attrs.StoreID;
		}
}


// IExtractIcon

void CFolderItem::GetIconFileAndIndex(CGetIconFileAndIndexEventArgs& e)
{
	e.iconExtractMode = NSEIEM_IconFileAndIndex;
	e.iconFile = theApp.m_CoreFile;
	e.iconIndex = Attrs.Icon-1;
}


// IQueryInfo

void CFolderItem::GetInfoTip(CString& infotip)
{
	infotip = Attrs.Description;
}


// IContextMenu

void CFolderItem::GetMenuItems(CGetMenuitemsEventArgs& e)
{
	// All items can be opened
	if (e.children->GetCount()>=1)
		if (Attrs.Level==LevelAttrValue)
		{
			InsertItem(e.menu, IDS_MENU_OpenWith, _T(VERB_OPENWITH));
			InsertItem(e.menu, IDS_MENU_Open, _T(VERB_OPEN))->SetDefaultItem((e.flags & NSEQCF_NoDefault)==0);
		}
		else
			if (osInfo.dwMajorVersion<6)
			{
				if (Attrs.Level==LevelRoot)
					InsertItem(e.menu, IDS_MENU_OpenStoreManager, _T(VERB_OPENSTOREMANAGER));

				if (e.flags & NSEQCF_NoDefault)
				{
					InsertItem(e.menu, IDS_MENU_Open, _T(VERB_OPEN));
				}
				else
				{
					InsertItem(e.menu, IDS_MENU_Explore, _T(e.flags & NSEQCF_Explore ? VERB_OPEN : VERB_EXPLORE), Attrs.Level==LevelRoot ? 1 : 0)->SetDefaultItem((e.flags & (NSEQCF_Explore | NSEQCF_NoDefault))==NSEQCF_Explore);
					InsertItem(e.menu, IDS_MENU_Open, _T(e.flags & NSEQCF_Explore ? VERB_OPENNEWWINDOW : VERB_OPEN))->SetDefaultItem((e.flags & (NSEQCF_Explore | NSEQCF_NoDefault))==0);
				}
			}
			else
			{
				if (!(e.flags & NSEQCF_NoDefault))
					InsertItem(e.menu, IDS_MENU_OpenNewWindow, _T(VERB_OPENNEWWINDOW));
				if (Attrs.Level==LevelRoot)
					InsertItem(e.menu, IDS_MENU_OpenStoreManager, _T(VERB_OPENSTOREMANAGER));

				InsertItem(e.menu, IDS_MENU_Open, _T(VERB_OPEN))->SetDefaultItem((e.flags & NSEQCF_NoDefault)==0);
			}

	switch (Attrs.Level)
	{
	case LevelRoot:
		if (e.children->GetCount()==0)
		{
			AddSeparator(e.menu);

			CShellMenuItem* item = AddItem(e.menu, IDS_MENU_CreateNewStore, _T(VERB_CREATENEWSTORE));
			item->SetEnabled(!theApp.m_PathRunCmd.IsEmpty());
			theApp.SetCoreMenuIcon(item, IDI_STORE_Internal);
		}

		if (e.children->GetCount()==1)
		{
			CFolderItem* f = (CFolderItem*)e.children->GetHead();

			AddSeparator(e.menu);
			AddItem(e.menu, IDS_MENU_MakeDefaultStore, _T(VERB_MAKEDEFAULTSTORE))->SetEnabled(f->Attrs.CategoryID==LFStoreModeInternal);
			AddItem(e.menu, IDS_MENU_MakeHybridStore, _T(VERB_MAKEHYBRIDSTORE))->SetEnabled(f->Attrs.CategoryID==LFStoreModeExternal);
			AddSeparator(e.menu);
			AddItem(e.menu, IDS_MENU_ImportFolder, _T(VERB_IMPORTFOLDER))->SetEnabled((!(f->Attrs.Type & LFTypeNotMounted)) && (!theApp.m_PathRunCmd.IsEmpty()));
		}

		if ((!(e.flags & NSEQCF_NoDefault)) && (e.children->GetCount()>=1))
		{
			AddSeparator(e.menu);
			AddItem(e.menu, IDS_MENU_CreateLink, _T(VERB_CREATELINK));
			AddItem(e.menu, IDS_MENU_Delete, _T(VERB_DELETE))->SetEnabled(!theApp.m_PathRunCmd.IsEmpty());

			if (e.children->GetCount()==1)
			{
				if (e.flags & NSEQCF_CanRename)
					AddItem(e.menu, IDS_MENU_Rename, _T(VERB_RENAME));

				AddSeparator(e.menu);
				AddItem(e.menu, IDS_MENU_Properties, _T(VERB_PROPERTIES))->SetEnabled(!theApp.m_PathRunCmd.IsEmpty());
			}
		}
		break;
	case LevelStores:
		if (e.children->GetCount()==0)
		{
			AddItem(e.menu, IDS_MENU_ImportFolder, _T(VERB_IMPORTFOLDER))->SetEnabled(!theApp.m_PathRunCmd.IsEmpty());
			AddSeparator(e.menu);
			AddItem(e.menu, IDS_MENU_Properties, _T(VERB_PROPERTIES))->SetEnabled(!theApp.m_PathRunCmd.IsEmpty());
		}
	case LevelStoreHome:
	case LevelAttribute:
		if ((!(e.flags & NSEQCF_NoDefault)) && (e.children->GetCount()>=1))
		{
			AddSeparator(e.menu);
			AddItem(e.menu, IDS_MENU_CreateLink, _T(VERB_CREATELINK));
		}
		break;
	case LevelAttrValue:
		if ((!(e.flags & NSEQCF_NoDefault)) && (e.children->GetCount()>=1))
		{
			AddSeparator(e.menu);
			AddItem(e.menu, IDS_MENU_CreateLink, _T(VERB_CREATELINK));
			AddItem(e.menu, IDS_MENU_Delete, _T(VERB_DELETE));
			if ((e.children->GetCount()==1) && (e.flags & NSEQCF_CanRename))
					AddItem(e.menu, IDS_MENU_Rename, _T(VERB_RENAME));

			AddSeparator(e.menu);
			AddItem(e.menu, IDS_MENU_Properties, _T(VERB_PROPERTIES))->SetEnabled(FALSE /*!theApp.m_PathRunCmd.IsEmpty()*/);
		}
		break;
	}
}

BOOL CFolderItem::OnExecuteMenuItem(CExecuteMenuitemsEventArgs& e)
{
	if (e.menuItem->GetVerb()==_T(VERB_IMPORTFOLDER))
		return OnImportFolder(e);

	if (e.menuItem->GetVerb()==_T(VERB_CREATENEWSTORE))
		return RunPath(e.hWnd, theApp.m_PathRunCmd, _T("NEWSTORE"));

	if (e.menuItem->GetVerb()==_T(VERB_EXPLORE))
		return OnExplorer(e);

	if (e.menuItem->GetVerb()==_T(VERB_OPENNEWWINDOW))
	{
		if (osInfo.dwMajorVersion<6)
			e.menuItem->SetVerb(_T(VERB_OPEN));

		return OnExplorer(e);
	}

	if (e.menuItem->GetVerb()==_T(VERB_OPENWITH))
		return OnOpenWith(e);

	if ((e.menuItem->GetVerb()==_T(VERB_MAKEDEFAULTSTORE)) || (e.menuItem->GetVerb()==_T(VERB_MAKEHYBRIDSTORE)))
	{
		POSITION pos = e.children->GetHeadPosition();
		CNSEItem* temp = (CNSEItem*)e.children->GetNext(pos);
		if (IS(temp, CFolderItem))
		{
			CFolderItem* folder = AS(temp, CFolderItem);
			char key[LFKeySize];
			strcpy_s(key, LFKeySize, folder->Attrs.StoreID);

			UINT res = (e.menuItem->GetVerb()==_T(VERB_MAKEDEFAULTSTORE)) ? LFMakeDefaultStore(&key[0]) : LFMakeHybridStore(&key[0]);
			LFErrorBox(res);
			return (res==LFOk);
		}

		return FALSE;
	}

	if (e.menuItem->GetVerb()==_T(VERB_CREATELINK))
	{
		OSVERSIONINFO osInfo;
		ZeroMemory(&osInfo, sizeof(OSVERSIONINFO));
		osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osInfo);

		if (osInfo.dwMajorVersion<6)
		{
			// Ask if link should be created on desktop
			CString tmpStr;
			CString tmpCaption;
			ENSURE(tmpStr.LoadString(IDS_TEXT_CreateLink));
			ENSURE(tmpCaption.LoadString(IDS_CAPT_CreateLink));

			if (MessageBox(GetViewWindow(), tmpStr, tmpCaption, MB_YESNO | MB_ICONQUESTION)==IDNO)
				return FALSE;
		}

		// Create shortcut on desktop
		POSITION pos = e.children->GetHeadPosition();
		while(pos)
		{
			CreateShortcut((CNSEItem*)e.children->GetNext(pos));
		}

		return TRUE;
	}

	return FALSE;
}


// IShellBrowser

void CFolderItem::GetToolbarButtons(CPtrList& commands)
{

	if ((!theApp.m_PathStoreManager.IsEmpty()) || (!theApp.m_PathFileDrop.IsEmpty()) || (!theApp.m_PathMigrate.IsEmpty()))
	{
		CString tmpStr;

		commands.AddTail(new CShellToolbarButton(_T(""), NSESTBT_Separator));

		if (!theApp.m_PathStoreManager.IsEmpty())
		{
			ENSURE(tmpStr.LoadString(IDS_MENU_StoreManager));
			tmpStr.Remove('&');
			commands.AddTail(new CShellToolbarButton(tmpStr, NSESTBT_Normal, (INT_PTR)IDB_StoreManager));
		}

		if (!theApp.m_PathMigrate.IsEmpty())
		{
			ENSURE(tmpStr.LoadString(IDS_MENU_Migrate));
			tmpStr.Remove('&');
			commands.AddTail(new CShellToolbarButton(tmpStr, NSESTBT_Normal, (INT_PTR)IDB_Migrate));
		}

		if (!theApp.m_PathFileDrop.IsEmpty())
		{
			ENSURE(tmpStr.LoadString(IDS_MENU_FileDrop));
			tmpStr.Remove('&');
			commands.AddTail(new CShellToolbarButton(tmpStr, NSESTBT_Normal, (INT_PTR)IDB_FileDrop));
		}
	}
}

void CFolderItem::OnMergeFrameMenu(CMergeFrameMenuEventArgs& e)
{
	CShellMenuItem* item = e.menu->AddItem(_T("&liquidFOLDERS"));
	item->SetHasSubMenu(TRUE);

	CShellMenu* subMenu = item->GetSubMenu();

	AddItem(subMenu, IDS_MENU_About, _T(VERB_ABOUT))->SetEnabled(!theApp.m_PathRunCmd.IsEmpty());
	AddSeparator(subMenu);
	AddPathItem(subMenu, IDS_MENU_StoreManager, _T(VERB_STOREMANAGER), theApp.m_PathStoreManager, IDI_StoreManager);
	AddPathItem(subMenu, IDS_MENU_Migrate, _T(VERB_MIGRATE), theApp.m_PathMigrate, IDI_Migrate);
	AddPathItem(subMenu, IDS_MENU_FileDrop, _T(VERB_FILEDROP), theApp.m_PathFileDrop, IDI_FileDrop);
}

void CFolderItem::OnExecuteFrameCommand(CExecuteFrameCommandEventArgs& e)
{
	if(e.menuItem)
	{
		if (e.menuItem->GetVerb()==_T(VERB_ABOUT))
			RunPath(NULL, theApp.m_PathRunCmd, "ABOUTEXTENSION");

		if (e.menuItem->GetVerb()==_T(VERB_STOREMANAGER))
			RunPath(NULL, theApp.m_PathStoreManager);

		if (e.menuItem->GetVerb()==_T(VERB_MIGRATE))
			RunPath(NULL, theApp.m_PathMigrate);

		if (e.menuItem->GetVerb()==_T(VERB_FILEDROP))
			RunPath(NULL, theApp.m_PathFileDrop);
	}
	else
		switch (e.toolbarButtonIndex)
		{
		case 1:
			RunPath(NULL, theApp.m_PathStoreManager);
			break;
		case 2:
			RunPath(NULL, theApp.m_PathMigrate);
			break;
		case 3:
			RunPath(NULL, theApp.m_PathFileDrop);
			break;
		}
}


// IExplorerCommandProvider

void CFolderItem::GetToolbarCommands(CPtrList& commands)
{
	if (Attrs.Level==LevelRoot)
	{
		commands.AddTail(new CmdImportFolder());
		commands.AddTail(new CmdProperties());
		commands.AddTail(new CmdCreateNewStore());
	}

	commands.AddTail(new CmdStoreManager());
	commands.AddTail(new CmdMigrate());
	commands.AddTail(new CmdFileDrop());
}


// ICategoryProvider

CCategorizer* CFolderItem::GetCategorizer(CShellColumn& column)
{
	switch (Attrs.Level)
	{
	case LevelRoot:
	case LevelStores:
		return new CCategoryCategorizer(this, column);
	case LevelStoreHome:
		return new CAttributeCategorizer(this, column);
	case LevelAttribute:
		return new CFolderCategorizer(this, column);
	default:
		switch (column.index)
		{
		case LFAttrFileSize:
			return new CSizeCategorizer(this, column);
		case LFAttrRating:
		case LFAttrPriority:
			return new CRatingCategorizer(this, column);
		default:
			return CNSEFolder::GetCategorizer(column);
		}
	}
}


// IColumnProvider

BOOL CFolderItem::GetColumn(CShellColumn& column, int index)
{
	// Determine last column for level
	int LastColumn;
	switch (Attrs.Level)
	{
	case LevelRoot:
		LastColumn = LFAttrFileTime;
		break;
	case LevelStores:
	case LevelAttribute:
		LastColumn = LFAttrFileSize;
		break;
	case LevelStoreHome:
		LastColumn = LFAttrComment;
		break;
	default:
		LastColumn = LFAttributeCount-1;
	}

	if (index>LastColumn)
		return FALSE;

	// Simple settings
	column.name = theApp.m_Attributes[index]->Name;
	column.width = theApp.m_Attributes[index]->RecommendedWidth/7;	// Chars, not pixel
	column.fmt = theApp.m_Attributes[index]->FormatRight ? NSESCF_Right : NSESCF_Left;
	column.state = (index>LFLastCoreAttribute) ? NSECS_SecondaryUi : (index==LFAttrFileName) ? NSECS_PreferVarCmp : NSECS_None;
	column.defaultVisible = (index!=LFAttrStoreID) && (index!=LFAttrFileID) && (index!=LFAttrFileCount) && (index!=LFAttrAddTime) && (index!=LFAttrArchiveTime) && (index!=LFAttrDeleteTime);
	column.index = index;
	if (theApp.m_Attributes[index]->ShPropertyMapping.ID)
	{
		column.fmtid = theApp.m_Attributes[index]->ShPropertyMapping.Schema;
		column.pid = theApp.m_Attributes[index]->ShPropertyMapping.ID;
	}

	// Shell column data type
	switch (theApp.m_Attributes[index]->Type)
	{
	case LFTypeUINT:
	case LFTypeINT64:
	case LFTypeDouble:
		column.dataType = NSESCDT_Numeric;
		break;
	case LFTypeTime:
		column.dataType = NSESCDT_Date;
		break;
	default:
		column.dataType = NSESCDT_String;
	}

	// Shell column categorizer
	if (Attrs.Level<=LevelAttribute)
	{
		column.categorizerType = NSECT_Custom;
	}
	else
		switch (index)
		{
		case LFAttrStoreID:
		case LFAttrCreationTime:
		case LFAttrFileTime:
		case LFAttrAddTime:
		case LFAttrDeleteTime:
		case LFAttrArchiveTime:
		case LFAttrFileFormat:
		case LFAttrFlags:
			column.categorizerType = NSECT_String;
			break;
		case LFAttrFileSize:
		case LFAttrRating:
		case LFAttrPriority:
			column.categorizerType = NSECT_Custom;
			break;
		default:
			column.categorizerType = NSECT_Alphabetical;
		}

	switch (index)
	{
	case LFAttrFileID:
		if (Attrs.Level<LevelAttrValue)
			column.state = NSECS_Hidden;
		break;
	case LFAttrComment:
		if (Attrs.Level==LevelAttribute)
			column.state = NSECS_Hidden;
		break;
	case LFAttrDescription:
		if ((Attrs.Level==LevelStoreHome) || (Attrs.Level==LevelAttrValue))
			column.state = NSECS_Hidden;
		break;
	case LFAttrFileCount:
		if ((Attrs.Level==LevelRoot) || (Attrs.Level==LevelStoreHome) || (Attrs.Level==LevelAttrValue))
			column.state = NSECS_Hidden;
		break;
	case LFAttrFileSize:
		if ((Attrs.Level==LevelRoot) || (Attrs.Level==LevelStoreHome))
			column.state = NSECS_Hidden;
		break;
	case LFAttrCreationTime:
	case LFAttrFileTime:
		if ((Attrs.Level!=LevelRoot) && (Attrs.Level!=LevelAttrValue))
			column.state = NSECS_Hidden;
		break;
	case LFAttrAddTime:
	case LFAttrArchiveTime:
		if (Attrs.Level!=LevelAttrValue)
			column.state = NSECS_Hidden;
		break;
	case LFAttrDeleteTime:
	case LFAttrFlags:
		column.state = NSECS_Hidden;
		break;
	}

	return TRUE;
}


// IShellFolder2

BOOL CFolderItem::GetColumnValueEx(VARIANT* value, CShellColumn& column)
{
	if (column.fmtid==FMTID_ShellDetails)
		switch (column.pid)
		{
		case 2:
			SAFEARRAYBOUND rgsabound;
			rgsabound.cElements = sizeof(SHDESCRIPTIONID);
			rgsabound.lLbound = 0;

			value->parray = SafeArrayCreate(VT_UI1, 1, &rgsabound);
			((SHDESCRIPTIONID*)value->parray->pvData)->clsid = guid;
			((SHDESCRIPTIONID*)value->parray->pvData)->dwDescriptionId = (Attrs.CategoryID==LFItemCategoryRemoteStores) ? SHDID_COMPUTER_NETDRIVE : 20;
			value->vt = VT_ARRAY | VT_UI1;
			return TRUE;
		case 9:
			CUtils::SetVariantINT(value, -1);
			return TRUE;
		case 11:
			CUtils::SetVariantLPCTSTR(value, "Folder");
			return TRUE;
		default:
			return FALSE;
		}

	if (column.fmtid==FMTID_Volume)
		switch (column.pid)
		{
		case 4:
			CUtils::SetVariantLPCTSTR(value, "liquidFOLDERS");
			return TRUE;
		default:
			return FALSE;
		}

	const GUID FMTID_Preview = { 0xC9944A21, 0xA406, 0x48FE, { 0x82, 0x25, 0xAE, 0xC7, 0xE2, 0x4C, 0x21, 0x1B } };
	if (column.fmtid==FMTID_Preview)
		switch (column.pid)
		{
		case 6:
			CUtils::SetVariantLPCTSTR(value, "prop:~System.ItemNameDisplay;~System.ItemTypeText");
			return TRUE;
		default:
			return FALSE;
		}

	switch (column.index)
	{
	case LFAttrFileName:
		CUtils::SetVariantCString(value, Attrs.DisplayName);
		break;
	case LFAttrFileID:
		CUtils::SetVariantCString(value, Attrs.FileID);
		break;
	case LFAttrStoreID:
		CUtils::SetVariantCString(value, Attrs.StoreID);
		break;
	case LFAttrDescription:
		CUtils::SetVariantCString(value, Attrs.Description);
		break;
	case LFAttrComment:
		CUtils::SetVariantCString(value, Attrs.Comment);
		break;
	case LFAttrCreationTime:
		if ((Attrs.CreationTime.dwHighDateTime) || (Attrs.CreationTime.dwLowDateTime))
		{
			if (value->vt==VT_BSTR)
			{
				wchar_t tmpBuf[256];
				LFTimeToString(Attrs.CreationTime, tmpBuf, 256);
				CString tmpStr(tmpBuf);
				CUtils::SetVariantCString(value, tmpStr);
			}
			else
			{
				CUtils::SetVariantFILETIME(value, Attrs.CreationTime);
			}
		}
		else
		{
			return FALSE;
		}
		break;
	case LFAttrFileTime:
		if ((Attrs.FileTime.dwHighDateTime) || (Attrs.FileTime.dwLowDateTime))
		{
			if (value->vt==VT_BSTR)
			{
				wchar_t tmpBuf[256];
				LFTimeToString(Attrs.FileTime, tmpBuf, 256);
				CString tmpStr(tmpBuf);
				CUtils::SetVariantCString(value, tmpStr);
			}
			else
			{
				CUtils::SetVariantFILETIME(value, Attrs.FileTime);
			}
		}
		else
		{
			return FALSE;
		}
		break;
	case LFAttrAddTime:
	case LFAttrArchiveTime:
	case LFAttrDeleteTime:
		return FALSE;
	case LFAttrFileFormat:
		CUtils::SetVariantCString(value, Attrs.Format);
		break;
	case LFAttrFileCount:
		CUtils::SetVariantUINT(value, Attrs.Count);
		break;
	case LFAttrFileSize:
		if (value->vt==VT_BSTR)
		{
			wchar_t tmpBuf[256];
			LFINT64ToString(Attrs.Size, tmpBuf, 256);
			CString tmpStr(tmpBuf);
			CUtils::SetVariantCString(value, tmpStr);
		}
		else
		{
			CUtils::SetVariantINT64(value, Attrs.Size);
		}
		break;
	default:
		{
			const GUID ShellDetails = { 0x28636AA6, 0x953D, 0x11D2, { 0xB5, 0xD6, 0x00, 0xC0, 0x4F, 0xD9, 0x18, 0xD0 } };
			if (column.fmtid==ShellDetails)
			{
				switch (column.pid)
				{
				case 2:
					SAFEARRAYBOUND rgsabound;
					rgsabound.cElements = sizeof(SHDESCRIPTIONID);
					rgsabound.lLbound = 0;

					value->parray = SafeArrayCreate(VT_UI1, 1, &rgsabound);
					((SHDESCRIPTIONID*)value->parray->pvData)->clsid = guid;
					((SHDESCRIPTIONID*)value->parray->pvData)->dwDescriptionId = (Attrs.CategoryID==LFItemCategoryRemoteStores) ? SHDID_COMPUTER_NETDRIVE : 20;
					value->vt = VT_ARRAY | VT_UI1;
					break;
				case 9:
					CUtils::SetVariantINT(value, -1);
					break;
				case 11:
					CUtils::SetVariantLPCTSTR(value, "Folder");
					break;
				default:
					return FALSE;
				}
			}
			return FALSE;
		}
	}

	return TRUE;
}


// IShellFolder

NSEItemAttributes CFolderItem::GetAttributes(NSEItemAttributes requested)
{
	UINT mask = NSEIA_CFOLDERITEM;

	if (Attrs.Level==LevelStores)
	{
		mask |= NSEIA_CanRename;
		if (!theApp.m_PathRunCmd.IsEmpty())
			mask |= NSEIA_CanDelete | NSEIA_HasPropSheet;
	}
	if (Attrs.Level<LevelAttrValue)
		mask |= NSEIA_HasSubFolder;
	if (Attrs.Level>LevelRoot)
		mask |= NSEIA_DropTarget;

	return (NSEItemAttributes)(requested & mask);
}

int CFolderItem::CompareTo(CNSEItem* otherItem, CShellColumn& column)
{
	if (IS(otherItem, CFileItem))
		return -1;

	CFolderItem* dir2 = AS(otherItem, CFolderItem);

	CString str1;
	CString str2;
	int ret = 0;

	switch (column.index)
	{
	case LFAttrFileName:
		str1 = Attrs.DisplayName;
		str2 = dir2->Attrs.DisplayName;
		break;
	case LFAttrStoreID:
		str1 = Attrs.StoreID;
		str2 = dir2->Attrs.StoreID;
		break;
	case LFAttrFileID:
		str1 = Attrs.FileID;
		str2 = dir2->Attrs.FileID;
		break;
	case LFAttrComment:
		str1 = Attrs.Comment;
		str2 = dir2->Attrs.Comment;
		break;
	case LFAttrDescription:
		str1 = Attrs.Description;
		str2 = dir2->Attrs.Description;
		break;
	case LFAttrCreationTime:
		ret = CompareFileTime(&Attrs.CreationTime, &dir2->Attrs.CreationTime);
		goto GotRet;
	case LFAttrFileTime:
		ret = CompareFileTime(&Attrs.FileTime, &dir2->Attrs.FileTime);
		goto GotRet;
	case LFAttrFileSize:
		if (Attrs.Size<dir2->Attrs.Size)
			return -1;
		if (Attrs.Size>dir2->Attrs.Size)
			return 1;
		goto GotRet;
	}

	// Items with empty attribute values come last
	if ((str1.IsEmpty()) && (!str2.IsEmpty()))
		return 1;
	if ((!str1.IsEmpty()) && (str2.IsEmpty()))
		return -1;

	// Compare desired attribute
	ret = str1.CompareNoCase(str2);
GotRet:
	if (ret)
		return ret;

	// Compare file names
	ret = Attrs.DisplayName.CompareNoCase(dir2->Attrs.DisplayName);
	if (ret)
		return ret;

	// Compare store IDs
	ret = Attrs.StoreID.Compare(dir2->Attrs.StoreID);
	if (ret)
		return ret;

	// Compare file IDs
	return Attrs.FileID.Compare(dir2->Attrs.FileID);
}

BOOL CFolderItem::OnOpen(CExecuteMenuitemsEventArgs& e)
{
	if (e.children->GetCount()==1)
	{
		POSITION pos = e.children->GetHeadPosition();
		CNSEItem* item = (CFileItem*)e.children->GetNext(pos);

		if (IS(item, CFolderItem))
			if (!CUtils::BrowseTo(item->GetPIDLAbsolute(), e.hWnd))
			{
				SHELLEXECUTEINFO sei;
				ZeroMemory(&sei, sizeof(sei));
				sei.cbSize = sizeof(sei);
				sei.fMask = SEE_MASK_IDLIST | SEE_MASK_CLASSNAME;
				sei.lpIDList = item->GetPIDLAbsolute();
				sei.lpClass = _T("folder");
				sei.hwnd = GetViewWindow();
				sei.nShow = SW_SHOWNORMAL;
				sei.lpVerb = _T("open");
				ShellExecuteEx(&sei);
			}

		if (IS(item, CFileItem))
		{
			char Path[MAX_PATH];
			UINT res = LFGetFileLocation((char*)(LPCSTR)Attrs.StoreID, &((CFileItem*)item)->Attrs, Path, MAX_PATH);
			if (res!=LFOk)
			{
				LFErrorBox(res);
			}
			else
				if (ShellExecuteA(e.hWnd, _T("open"), Path, "", "", SW_SHOW)==(HINSTANCE)SE_ERR_NOASSOC)
				{
					char Cmd[300];
					strcpy_s(Cmd, 300, "shell32.dll,OpenAs_RunDLL ");
					strcat_s(Cmd, 300, Path);
					ShellExecuteA(e.hWnd, _T("open"), "rundll32.exe", Cmd, Path, SW_SHOW);
				}
		}

		return TRUE;
	}

	return FALSE;
}

BOOL CFolderItem::OnDelete(CExecuteMenuitemsEventArgs& e)
{
	if (!e.children->GetCount())
		return FALSE;

	// Folder
	if ((Attrs.Level==LevelRoot) && (!theApp.m_PathRunCmd.IsEmpty()))
	{
		POSITION pos = e.children->GetHeadPosition();
		CNSEItem* item = (CNSEItem*)e.children->GetNext(pos);

		if (IS(item, CFolderItem))
		{
			CString id = AS(item, CFolderItem)->Attrs.StoreID;
			ShellExecute(e.hWnd, _T("open"), theApp.m_PathRunCmd, _T("DELETESTORE ")+id, NULL, SW_SHOW);
			return TRUE;
		}
	}

	// Files
	BOOL res = FALSE;
	LFFileIDList* il = LFAllocFileIDList();

	POSITION pos = e.children->GetHeadPosition();
	while (pos)
	{
		CNSEItem* item = (CNSEItem*)e.children->GetNext(pos);

		if (IS(item, CFileItem))
		{
			char StoreID[LFKeySize];
			char FileID[LFKeySize];
			strcpy_s(StoreID, LFKeySize, AS(item, CFileItem)->StoreID);
			strcpy_s(FileID, LFKeySize, AS(item, CFileItem)->Attrs.FileID);

			LFAddFileID(il, StoreID, FileID, item);
		}
	}

	if (il->m_ItemCount)
	{
		LFTransactionDelete(il);

		for (UINT a=0; a<il->m_ItemCount; a++)
			if ((il->m_Items[a].Processed) && (il->m_Items[a].LastError==LFOk))
				((CFileItem*)il->m_Items[a].UserData)->Delete();

		res = TRUE;
	}

	LFErrorBox(il->m_LastError);
	LFFreeFileIDList(il);
	return res;
}

BOOL CFolderItem::OnChangeName(CChangeNameEventArgs& e)
{
	// Stores sind die einzigen CFolderItem, die umbenannt werden k�nnen
	if (Attrs.Level==LevelStores)
	{
		char key[LFKeySize];
		strcpy_s(key, LFKeySize, Attrs.StoreID);

		USES_CONVERSION;
		LPWSTR name = T2W(e.newName);

		LFErrorBox(LFSetStoreAttributes(&key[0], name, NULL));
	}

	return FALSE;
}


// IDropSource

void CFolderItem::InitDataObject(CInitDataObjectEventArgs& e)
{
	if (e.children->GetCount())
		e.dataObject->SetHasFileData();
}

BOOL CFolderItem::GetFileDescriptor(FILEDESCRIPTOR* fd)
{
	_tcscpy(fd->cFileName, Attrs.DisplayName);

	fd->dwFlags = FD_ATTRIBUTES | FD_CLSID;
	fd->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	fd->clsid = guid;

	switch (Attrs.Level)
	{
	case LevelRoot:
		fd->dwFlags |= FD_WRITESTIME | FD_CREATETIME;
		fd->ftCreationTime = Attrs.CreationTime;
		fd->ftLastWriteTime = Attrs.FileTime;
		break;
	case LevelStores:
		LARGE_INTEGER sz;
		sz.QuadPart = Attrs.Size;

		fd->dwFlags |= FD_FILESIZE;
		fd->nFileSizeHigh = sz.HighPart;
		fd->nFileSizeLow = sz.LowPart;
		break;
	}

	return TRUE;
}

void CFolderItem::OnExternalDrop(CNSEDragEventArgs& e)
{
	if (e.data->ShouldDeleteSource() && e.data->DidValidDrop())
	{
		LFFileIDList* il = LFAllocFileIDList();

		POSITION pos = e.data->GetChildren()->GetHeadPosition();
		while (pos)
		{
			CNSEItem* item = (CNSEItem*)e.data->GetChildren()->GetNext(pos);

			if (IS(item, CFileItem))
			{
				char StoreID[LFKeySize];
				char FileID[LFKeySize];
				strcpy_s(StoreID, LFKeySize, AS(item, CFileItem)->StoreID);
				strcpy_s(FileID, LFKeySize, AS(item, CFileItem)->Attrs.FileID);

				LFAddFileID(il, StoreID, FileID, item);
			}
		}

		if (il->m_ItemCount)
		{
			LFTransactionDelete(il, false);

			for (UINT a=0; a<il->m_ItemCount; a++)
				if ((il->m_Items[a].Processed) && (il->m_Items[a].LastError==LFOk))
					((CFileItem*)il->m_Items[a].UserData)->Delete();
		}

		LFErrorBox(il->m_LastError);
		LFFreeFileIDList(il);
	}
}


// IDropTarget

void CFolderItem::DragEnter(CNSEDragEventArgs& e)
{
	e.effect = (Attrs.Level==LevelRoot) ? DROPEFFECT_NONE : (e.keyState & MK_CONTROL) ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
}

void CFolderItem::DragOver(CNSEDragEventArgs& e)
{
	e.effect = (Attrs.Level==LevelRoot) ? DROPEFFECT_NONE : (e.keyState & MK_CONTROL) ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
}

void CFolderItem::DragDrop(CNSEDragEventArgs& e)
{
	MessageBox(e.hWnd, _T("Not implemented yet"), _T("Drop"), 0);
	// If file drop data is present, do the copy/move
/*	CStringArray files;
	if (e.data->GetHDROPData(&files))
	{
		for(int i=0;i<files.GetSize();i++)
		{
			CString file = files[i];
			CString fileName = PathFindFileName(file);
			CString destPath = PathCombineNSE(fullPath, fileName);
			if((GetFileAttributes(file) & FILE_ATTRIBUTE_DIRECTORY)!=0)
			{
				try
				{
					CopyDirectory(file, destPath);
				}
				catch(...)
				{
				}
			}
			else
			{
				try
				{
					CopyFile(file,destPath,TRUE);
				}
				catch(...)
				{
				}
			}
			e.data->SetPerformedDropEffect(e.effect);

			if(e.data->GetPreferredDropEffect()==DROPEFFECT_MOVE)
				e.data->SetPasteSucceded(DROPEFFECT_MOVE);

			this->RefreshView();
		}
	}
*/
}


// Exposed property handlers

int CFolderItem::GetXPTaskPaneColumnIndices(UINT* indices)
{
	indices[0] = LFAttrFileName;
	indices[1] = LFAttrComment;
	indices[2] = LFAttrDescription;

	switch (Attrs.Level)
	{
	case LevelStores:
		indices[2] = LFAttrCreationTime;
		indices[3] = LFAttrFileTime;
		indices[4] = LFAttrDescription;
		return 5;
	case LevelStoreHome:
	case LevelAttrValue:
		indices[3] = LFAttrFileSize;
		return (Attrs.FileID=="ALL") ? 3 : 4;
	case LevelAttribute:
		return 2;
	}

	return 3;
}

int CFolderItem::GetTileViewColumnIndices(UINT* indices)
{
	indices[0] = LFAttrComment;
	indices[1] = LFAttrDescription;

	switch (Attrs.Level)
	{
	case LevelStores:
		indices[1] = LFAttrFileTime;
		indices[2] = LFAttrDescription;
		return 3;
	case LevelAttrValue:
		indices[2] = LFAttrFileSize;
		return (Attrs.FileID==_T("ALL")) ? 2 : 3;
	}

	return 2;
}

int CFolderItem::GetPreviewDetailsColumnIndices(UINT* indices)
{
	indices[0] = LFAttrComment;
	indices[1] = LFAttrDescription;

	switch (Attrs.Level)
	{
	case LevelStores:
		indices[1] = LFAttrCreationTime;
		indices[2] = LFAttrFileTime;
		indices[3] = LFAttrDescription;
		return 4;
	case LevelStoreHome:
	case LevelAttrValue:
		indices[2] = LFAttrFileSize;
		return (Attrs.FileID==_T("ALL")) ? 2 : 3;
	case LevelAttribute:
		return 1;
	}

	return 2;
}

int CFolderItem::GetContentViewColumnIndices(UINT* indices)
{
	return GetXPTaskPaneColumnIndices(indices);
}


FolderThemes CFolderItem::GetFolderTheme()
{
	FolderThemes t = NSEFT_None;
	if (Attrs.Level>=LevelStoreHome)
		switch (Attrs.DomainID)
		{
		case LFDomainAudio:
			t = NSEFT_Music;
			break;
		case LFDomainPictures:
		case LFDomainPhotos:
			t = NSEFT_Picture;
			break;
		case LFDomainVideos:
			t = NSEFT_Video;
			break;
		case LFDomainAllFiles:
		case LFDomainAllMediaFiles:
		case LFDomainFavorites:
			t = NSEFT_Search;
			break;
		}

	return t;
}


// Other

BOOL CFolderItem::SetShellLink(IShellLink* psl)
{
	psl->SetIDList(GetPIDLAbsolute());
	psl->SetIconLocation(theApp.m_CoreFile, (Attrs.Icon==IDI_STORE_Default ? IDI_STORE_Internal : Attrs.Icon)-1);
	psl->SetShowCmd(SW_SHOWNORMAL);
	psl->SetDescription(Attrs.Comment);
	return TRUE;
}

void CFolderItem::CreateShortcut(CNSEItem* Item)
{
	// Get a pointer to the IShellLink interface
	IShellLink* psl;
	if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (PVOID *)&psl)))
	{
		BOOL res = FALSE;
		if (IS(Item, CFolderItem))
			res = AS(Item, CFolderItem)->SetShellLink(psl);
		if (IS(Item, CFileItem))
			res = AS(Item, CFileItem)->SetShellLink(psl);

		if (res)
		{
			CString LinkFilename;
			Item->GetDisplayName(LinkFilename);

			// Get the fully qualified file name for the link file
			TCHAR strPath[MAX_PATH];
			SHGetSpecialFolderPath(NULL, strPath, CSIDL_DESKTOPDIRECTORY, FALSE);
			CString PathLink;
			CString NumberStr = "";
			int Number = 1;

			// Check if link file exists; if not, append number
			do
			{
				PathLink = strPath;
				PathLink += _T("\\")+LinkFilename+NumberStr+_T(".lnk");
				NumberStr.Format(_T(" (%d)"), ++Number);
			}
			while (_access(PathLink, 0)==0);

			// Query IShellLink for the IPersistFile interface for saving the 
			// shortcut in persistent storage
			IPersistFile* ppf;
			if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (PVOID*)&ppf)))
			{
				// Ensure that the string is ANSI
				wchar_t wsz[MAX_PATH];
				MultiByteToWideChar(CP_ACP, 0, (LPCSTR)PathLink, -1, wsz, MAX_PATH);

				// Save the link by calling IPersistFile::Save
				ppf->Save(wsz, TRUE);
				ppf->Release();
			}
		}

		psl->Release();
	}
}

BOOL CFolderItem::OnImportFolder(CExecuteMenuitemsEventArgs& e)
{
	CString StoreID;
	if (Attrs.Level==LevelRoot)
	{
		POSITION pos = e.children->GetHeadPosition();
		if (pos)
		{
			CNSEItem* item = (CNSEItem*)e.children->GetNext(pos);
			if (IS(item, CFolderItem))
				StoreID = AS(item, CFolderItem)->Attrs.StoreID;
		}
	}
	else
	{
		StoreID = Attrs.StoreID;
	}

	if ((StoreID.IsEmpty()) || (theApp.m_PathRunCmd.IsEmpty()))
		return FALSE;

	ShellExecute(e.hWnd, _T("open"), theApp.m_PathRunCmd, _T("IMPORTFOLDER ")+StoreID, NULL, SW_SHOW);
	return TRUE;
}

BOOL CFolderItem::OnProperties(CExecuteMenuitemsEventArgs& e)
{
	CString StoreID;
	if (Attrs.Level==LevelRoot)
	{
		POSITION pos = e.children->GetHeadPosition();
		if (pos)
		{
			CNSEItem* item = (CNSEItem*)e.children->GetNext(pos);
			if (IS(item, CFolderItem))
				StoreID = AS(item, CFolderItem)->Attrs.StoreID;
		}
	}
	else
	{
		StoreID = Attrs.StoreID;
	}

	if ((StoreID.IsEmpty()) || (theApp.m_PathRunCmd.IsEmpty()))
		return FALSE;

	ShellExecute(e.hWnd, _T("open"), theApp.m_PathRunCmd, _T("STOREPROPERTIES ")+StoreID, NULL, SW_SHOW);
	return TRUE;}


BOOL CFolderItem::OnExplorer(CExecuteMenuitemsEventArgs& e)
{
	if (e.children->GetCount()==1)
	{
		POSITION pos = e.children->GetHeadPosition();
		CNSEItem* item = (CFileItem*)e.children->GetNext(pos);

		if (IS(item, CFolderItem))
		{
			SHELLEXECUTEINFO sei;
			ZeroMemory(&sei, sizeof(sei));
			sei.cbSize = sizeof(sei);
			sei.fMask = SEE_MASK_IDLIST | SEE_MASK_CLASSNAME;
			sei.lpIDList = item->GetPIDLAbsolute();
			sei.lpClass = _T("folder");
			sei.hwnd = GetViewWindow();
			sei.nShow = SW_SHOWNORMAL;
			sei.lpVerb = e.menuItem->GetVerb();
			ShellExecuteEx(&sei);

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CFolderItem::OnOpenWith(CExecuteMenuitemsEventArgs& e)
{
	if (e.children->GetCount()==1)
	{
		POSITION pos = e.children->GetHeadPosition();
		CNSEItem* item = (CFileItem*)e.children->GetNext(pos);

		if (IS(item, CFileItem))
		{
			char Path[MAX_PATH];
			UINT res = LFGetFileLocation((char*)(LPCSTR)Attrs.StoreID, &((CFileItem*)item)->Attrs, Path, MAX_PATH);
			if (res!=LFOk)
			{
				LFErrorBox(res);
			}
			else
			{
				char Cmd[300];
				strcpy_s(Cmd, 300, "shell32.dll,OpenAs_RunDLL ");
				strcat_s(Cmd, 300, Path);
				ShellExecuteA(e.hWnd, _T("open"), "rundll32.exe", Cmd, Path, SW_SHOW);
			}

			return TRUE;
		}
	}

	return FALSE;
}
