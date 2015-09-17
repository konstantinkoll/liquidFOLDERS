
// CFolderItem.cpp: Implementierung der Klasse CFolderItem
//

#include "stdafx.h"
#include "Categorizer.h"
#include "Commands.h"
#include "CFileItem.h"
#include "CFolderItem.h"
#include "LFNamespaceExtension.h"
#include <afxsettingsstore.h>


CShellMenuItem* InsertItem(CShellMenu* Menu, UINT ResID, CString Verb, INT Pos=0)
{
	CString tmpStr((LPCSTR)ResID);
	CString tmpHint((LPCSTR)ResID+1);

	return Menu->InsertItem(tmpStr, Verb, tmpHint, Pos);
}

void AddSeparator(CShellMenu* Menu)
{
	Menu->AddItem(_T(""))->SetSeparator(TRUE);
}

CShellMenuItem* AddItem(CShellMenu* Menu, UINT ResID, CString Verb)
{
	CString tmpStr((LPCSTR)ResID);
	CString tmpHint((LPCSTR)ResID+1);

	return Menu->AddItem(tmpStr, Verb, tmpHint);
}

BOOL RunPath(HWND hWnd, CString Path, CString Parameter)
{
	if (!Path.IsEmpty())
	{
		ShellExecute(hWnd, _T("open"), Path, Parameter, NULL, SW_SHOW);
		return TRUE;
	}

	return FALSE;
}


IMPLEMENT_DYNCREATE(CFolderItem, CNSEFolder)
IMPLEMENT_OLECREATE_EX(CFolderItem, "LFNamespaceExtension.RootFolder",
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

		// Hide desktop icon
		if (reg.Open(_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\ClassicStartMenu")))
		{
			reg.Write(_T("{3F2D914F-FE57-414F-9F88-A377C7841DA4}"), (DWORD)1);
			reg.Close();
		}
		if (reg.Open(_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel")))
		{
			reg.Write(_T("{3F2D914F-FE57-414F-9F88-A377C7841DA4}"), (DWORD)1);
			reg.Close();
		}

		// Move to delegate folders
		reg.DeleteKey(_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{3F2D914F-FE57-414F-9F88-A377C7841DA4}"), TRUE);

		if (reg.Open(_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\DelegateFolders")))
		{
			reg.CreateKey(_T("{3F2D914F-FE57-414F-9F88-A377C7841DA4}"));
			reg.Close();
		}

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
	ZeroMemory(&Attrs, sizeof(Attrs));
	Attrs.Level = LEVELROOT;
}

CFolderItem::CFolderItem(FolderSerialization& _Attrs)
{
	Attrs = _Attrs;
}

CFolderItem::CFolderItem(UCHAR Level, LFItemDescriptor* i)
{
	ZeroMemory(&Attrs, sizeof(Attrs));
	Attrs.Level = Level;
	Attrs.Icon = i->IconID;
	Attrs.Type = i->Type;
	Attrs.CategoryID = i->CategoryID;
	wcscpy_s(Attrs.DisplayName, 256, i->CoreAttributes.FileName);
	wcscpy_s(Attrs.Description, 256, i->Description);
	wcscpy_s(Attrs.Comments, 256, i->CoreAttributes.Comments);
	strcpy_s(Attrs.StoreID, LFKeySize, i->NextFilter ? i->NextFilter->StoreID : i->StoreID);
	strcpy_s(Attrs.FileID, LFKeySize, i->CoreAttributes.FileID);
	Attrs.Count = i->AggregateCount;
	Attrs.Size = i->CoreAttributes.FileSize;

	switch (Level)
	{
	case LEVELSTORES:
		Attrs.CreationTime = i->CoreAttributes.CreationTime;
		Attrs.FileTime = i->CoreAttributes.FileTime;
		break;
	case LEVELATTRVALUE:
		if (i->NextFilter)
			if (i->NextFilter->ConditionList)
			{
				Attrs.Compare = i->NextFilter->ConditionList->Compare;
				Attrs.Value = i->NextFilter->ConditionList->AttrData;
			}
	}

	LFFreeItemDescriptor(i);
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
	nti->infoTip.LoadString(IDS_INFOTIP);
	nti->attributes = (NSEItemAttributes)(NSEIA_CFOLDERITEM | NSEIA_HasSubFolder);
	nti->iconFile = theApp.m_PathApplication;
	nti->iconIndex = 0;
	nti->AddRootNodeProperty(_T("SortOrderIndex"), (UINT)64);
	nti->AddRootNodeProperty(_T("System.DescriptionID"), (UINT)20);
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
	nti->infoTip.LoadString(IDS_INFOTIP);
	nti->attributes = (NSEItemAttributes)(NSEIA_CFOLDERITEM | NSEIA_HasSubFolder);
	nti->iconFile = theApp.m_PathApplication;
	nti->iconIndex = 0;
	nti->AddRootNodeProperty(_T("SortOrderIndex"), (UINT)64);
	nti->AddRootNodeProperty(_T("System.DescriptionID"), (UINT)20);
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
	ar << (Attrs.Level==LEVELROOT ? (BYTE)0x1F : (BYTE)0x31);
	ar << (BYTE)LFNSE_VERSION;
	ar.Write(&Attrs, sizeof(Attrs));
}

CNSEItem* CFolderItem::DeserializeChild(CArchive& ar)
{
	BYTE ItemType;
	ar >> ItemType;

	BYTE Version;
	ar >> Version;
	if (Version!=LFNSE_VERSION)
		return NULL;

	switch (ItemType)
	{
	case 0x1F:
	case 0x31:
		{
			FolderSerialization Attrs;
			ar.Read(&Attrs, sizeof(Attrs));

			return new CFolderItem(Attrs);
		}
	case 0x32:
		{
			CHAR StoreID[LFKeySize];
			ar.Read(&StoreID, sizeof(StoreID));

			UINT Size;
			ar >> Size;

			LFCoreAttributes Attrs = { 0 };
			ar.Read(&Attrs, min(Size, sizeof(LFCoreAttributes)));

			CFileItem* f = new CFileItem(StoreID, &Attrs);

			UINT Count;
			ar >> Count;
			for (UINT a=0; a<Count; a++)
			{
				LFVariantData v;
				ar.Read(&v, sizeof(LFVariantData));
				LFSetAttributeVariantData(f->m_pItem, v);
			}

			return f;
		}
	}

	return NULL;
}


// IEnumIDList

void CFolderItem::ConvertSearchResult(CGetChildrenEventArgs& e, LFSearchResult* Result)
{
	if (!Result)
		return;

	if (Result->m_LastError!=LFOk)
	{
		LFCoreErrorBox(Result->m_LastError);
		LFFreeSearchResult(Result);
		return;
	}

	UINT NullCount = 0;
	INT64 NullSize = 0;

	for (UINT a=0; a<Result->m_ItemCount; a++)
	{
		LFItemDescriptor* i = Result->m_Items[a];

		switch (i->Type & LFTypeMask)
		{
		case LFTypeStore:
		case LFTypeFolder:
			if (e.childrenType & NSECT_Folders)
			{
				i->RefCount++;
				e.children->AddTail(new CFolderItem(Attrs.Level+1, i));
			}
			break;
		case LFTypeFile:
			if ((Attrs.Level==LEVELATTRIBUTE) && (atoi(Attrs.FileID)!=LFAttrFileName))
			{
				NullCount++;
				NullSize += i->CoreAttributes.FileSize;
			}
			else
				if (e.childrenType & NSECT_NonFolders)
					e.children->AddTail(new CFileItem(i));
			break;
		}
	}

	// Special folder if files exist that do not contain a certain property
	if ((NullCount) && (e.childrenType & NSECT_Folders))
	{
		UINT attr = atoi(Attrs.FileID);

		FolderSerialization d;
		ZeroMemory(&d, sizeof(d));
		d.Level = Attrs.Level+1;
		d.Icon = IDI_FLD_DEFAULT;
		d.Type = LFTypeFolder;
		d.CategoryID = LFAttrCategoryCount;
		strcpy_s(d.StoreID, LFKeySize, Attrs.StoreID);
		strcpy_s(d.FileID, LFKeySize, "NULL");
		d.Count = NullCount;
		d.Size = NullSize;
		d.Compare = LFFilterCompareIsNull;
		d.Value.Attr = attr;

		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDS_NULLFOLDER_NAMEMASK));
		wcscpy_s(d.DisplayName, 256, theApp.FrmtAttrStr(tmpStr, CString(theApp.m_Attributes[attr].Name)));
		ENSURE(tmpStr.LoadString(IDS_NULLFOLDER_COMMENTMASK));
		wcscpy_s(d.Comments, 256, theApp.FrmtAttrStr(tmpStr, CString(theApp.m_Attributes[attr].Name)));
		LFCombineFileCountSize(NullCount, NullSize, d.Description, 256);

		e.children->AddTail(new CFolderItem(d));
	}

	LFFreeSearchResult(Result);
}

BOOL CFolderItem::GetChildren(CGetChildrenEventArgs& e)
{
	LFFilter* f = NULL;
	LFSearchResult* base = NULL;

	switch (Attrs.Level)
	{
	case LEVELROOT:
		ConvertSearchResult(e, LFQuery(NULL));
		break;
	case LEVELSTORES:
		// This level is created by the namespace extension, and does not exist in the core tree structure
		if (e.childrenType & NSECT_Folders)
		{
			// Query files
			f = LFAllocFilter();
			f->Mode = LFFilterModeDirectoryTree;
			strcpy_s(f->StoreID, LFKeySize, Attrs.StoreID);
			base = LFQuery(f);

			if (base->m_ItemCount)
			{
				CString sortStr((LPCSTR)IDS_ATTRIBUTECOMMENT);

				// All files, regardless of attributes
				FolderSerialization d;
				ZeroMemory(&d, sizeof(d));
				d.Level = Attrs.Level+2;
				d.Icon = IDI_FLD_ALL;
				d.Type = LFTypeFolder;
				d.CategoryID = LFAttrCategoryCount;
				ENSURE(LoadString(AfxGetResourceHandle(), IDS_ALLFILES, d.DisplayName, 256));
				ENSURE(LoadString(AfxGetResourceHandle(), IDS_ALLFILESCOMMENT, d.Comments, 256));
				strcpy_s(d.StoreID, LFKeySize, Attrs.StoreID);
				strcpy_s(d.FileID, LFKeySize, "ALL");

				e.children->AddTail(new CFolderItem(d));

				// Get existing attributes
				BOOL ExistingAttributes[LFAttributeCount];
				ZeroMemory(&ExistingAttributes, sizeof(ExistingAttributes));

				for (UINT a=0; a<base->m_ItemCount; a++)
					for (UINT b=0; b<LFAttributeCount; b++)
						if (!ExistingAttributes[b])
						{
							LFVariantData v;
							LFGetAttributeVariantDataEx(base->m_Items[a], b, v);

							if (!LFIsNullVariantData(v))
								ExistingAttributes[b] = TRUE;
						}

				// Add important attributes which exist
				for (UINT a=0; a<LFAttributeCount; a++)
					switch (a)
					{
					case LFAttrFileName:
					case LFAttrComments:
					case LFAttrCreationTime:
					case LFAttrFileTime:
					case LFAttrHashtags:
					case LFAttrRating:
					case LFAttrPriority:
					case LFAttrLocationName:
					case LFAttrLocationIATA:
					case LFAttrRoll:
					case LFAttrAlbum:
					case LFAttrArtist:
					case LFAttrTitle:
					case LFAttrCopyright:
					case LFAttrRecordingTime:
					case LFAttrEquipment:
					case LFAttrResponsible:
					case LFAttrDueTime:
					case LFAttrDoneTime:
					case LFAttrCustomer:
						if (ExistingAttributes[a])
						{
							FolderSerialization d ;
							ZeroMemory(&d, sizeof(d));
							d.Level = Attrs.Level+1;
							d.Icon = IDI_FLD_DEFAULT;
							d.Type = LFTypeFolder;
							d.CategoryID = theApp.m_Attributes[a].Category;
							wcscpy_s(d.DisplayName, 256, theApp.m_Attributes[a].Name);
							wcscpy_s(d.Comments, 256, theApp.FrmtAttrStr(sortStr, theApp.m_Attributes[a].Name));
							strcpy_s(d.StoreID, LFKeySize, Attrs.StoreID);
							sprintf_s(d.FileID, LFKeySize, "%u", a);

							e.children->AddTail(new CFolderItem(d));
						}
					}
			}
		}
		break;
	case LEVELATTRIBUTE:
		f = LFAllocFilter();
		f->Mode = LFFilterModeDirectoryTree;
		strcpy_s(f->StoreID, LFKeySize, Attrs.StoreID);
		base = LFQuery(f);
		ConvertSearchResult(e, LFGroupSearchResult(base, atoi(Attrs.FileID), FALSE, atoi(Attrs.FileID)!=LFAttrFileName, f));
		break;
	case LEVELATTRVALUE:
		f = LFAllocFilter();
		f->Mode = LFFilterModeDirectoryTree;
		strcpy_s(f->StoreID, LFKeySize, Attrs.StoreID);
		f->ConditionList = LFAllocFilterCondition(Attrs.Compare, Attrs.Value);
		ConvertSearchResult(e, LFQuery(f));
		break;
	}

	LFFreeSearchResult(base);
	LFFreeFilter(f);

	return TRUE;
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
			WCHAR buf[41] = L"::";
			StringFromGUID2(guid, &buf[2], 39);
			displayName = buf;

			if (Attrs.Level>LEVELROOT)
			{
				displayName.Append(_T("\\")+CString(Attrs.StoreID));
				if (Attrs.Level>LEVELSTORES)
					displayName += _T("\\VFOLDER");
			}
		}
		else
		{
			displayName = (Attrs.Level>LEVELSTORES) ? "VFOLDER" : (Attrs.Level>LEVELROOT) ? Attrs.StoreID : "";
		}
}

CNSEItem* CFolderItem::GetChildFromDisplayName(CGetChildFromDisplayNameEventArgs& e)
{
	if (Attrs.Level!=LEVELROOT)
		return NULL;

	CHAR key[LFKeySize];
	WideCharToMultiByte(CP_ACP, 0, e.displayName, -1, key, LFKeySize, NULL, NULL);

	LFStoreDescriptor Store;
	if (LFGetStoreSettings(key, &Store)!=LFOk)
		return NULL;

	return new CFolderItem(LEVELSTORES, LFAllocItemDescriptorEx(&Store));
}


// IExtractIcon

void CFolderItem::GetIconFileAndIndex(CGetIconFileAndIndexEventArgs& e)
{
	e.iconExtractMode = NSEIEM_IconFileAndIndex;
	e.iconFile = theApp.m_PathCoreFile;
	e.iconIndex = Attrs.Icon-1;
}

void CFolderItem::GetOverlayIcon(CGetOverlayIconEventArgs& e)
{
	if (Attrs.Type & LFTypeDefault)
	{
		e.overlayIconType = NSEOIT_Custom;
		e.iconFile = theApp.m_PathCoreFile;
		e.iconIndex = IDI_OVR_DEFAULT-1;
	}
	else
	{
		e.overlayIconType = NSEOIT_None;
	}
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
	if (e.children->GetCount()==1)
		if (Attrs.Level==LEVELATTRVALUE)
		{
			CNSEItem* temp = (CNSEItem*)e.children->GetHead();
			if (IS(temp, CFileItem))
			{
				CFileItem* file = AS(temp, CFileItem);

				if (file->m_pItem->CoreAttributes.URL[0]!='\0')
					InsertItem(e.menu, IDS_MENU_OPENBROWSER, _T(VERB_OPENBROWSER));
			}

			InsertItem(e.menu, IDS_MENU_OPENWITH, _T(VERB_OPENWITH));
			InsertItem(e.menu, IDS_MENU_OPEN, _T(VERB_OPEN))->SetDefaultItem((e.flags & NSEQCF_NoDefault)==0);
		}
		else
		{
			if (Attrs.Level==LEVELROOT)
			{
				InsertItem(e.menu, IDS_MENU_OPENFILEDROP, _T(VERB_OPENFILEDROP))->SetEnabled(theApp.m_PathApplication[0]!=L'\0');
				InsertItem(e.menu, IDS_MENU_OPENLIQUIDFOLDERS, _T(VERB_OPENLIQUIDFOLDERS))->SetEnabled(theApp.m_PathApplication[0]!=L'\0');
			}

			if (osInfo.dwMajorVersion<6)
			{
				InsertItem(e.menu, IDS_MENU_EXPLORE, e.flags & NSEQCF_Explore ? _T(VERB_OPEN) : _T(VERB_EXPLORE), Attrs.Level==LEVELROOT ? 2 : 0)->SetDefaultItem((e.flags & (NSEQCF_Explore | NSEQCF_NoDefault))==NSEQCF_Explore);
				InsertItem(e.menu, IDS_MENU_OPEN, e.flags & NSEQCF_Explore ? _T(VERB_OPENNEWWINDOW) : _T(VERB_OPEN))->SetDefaultItem((e.flags & (NSEQCF_Explore | NSEQCF_NoDefault))==0);
			}
			else
			{
				InsertItem(e.menu, IDS_MENU_OPENNEWWINDOW, _T(VERB_OPENNEWWINDOW));
				if ((e.flags & NSEQCF_NoDefault)==0)
					InsertItem(e.menu, IDS_MENU_OPEN, _T(VERB_OPEN))->SetDefaultItem(TRUE);
			}
		}

	switch (Attrs.Level)
	{
	case LEVELROOT:
		if (e.children->GetCount()==0)
		{
			AddSeparator(e.menu);
			AddItem(e.menu, IDS_MENU_ADDSTORE, _T(VERB_ADDSTORE));
		}

		if (e.children->GetCount()==1)
		{
			CFolderItem* f = (CFolderItem*)e.children->GetHead();

			AddSeparator(e.menu);
			AddItem(e.menu, IDS_MENU_MAKEDEFAULTSTORE, _T(VERB_MAKEDEFAULTSTORE))->SetEnabled(!(f->Attrs.Type & LFTypeDefault));
			AddItem(e.menu, IDS_MENU_IMPORTFOLDER, _T(VERB_IMPORTFOLDER))->SetEnabled((!(f->Attrs.Type & LFTypeNotMounted)) && (theApp.m_PathApplication[0]!=L'\0'));
		}

		if ((!(e.flags & NSEQCF_NoDefault)) && (e.children->GetCount()>=1))
		{
			AddSeparator(e.menu);
			AddItem(e.menu, IDS_MENU_CREATESHORTCUT, _T(VERB_CREATESHORTCUT));

			if (e.children->GetCount()==1)
			{
				AddItem(e.menu, IDS_MENU_DELETE, _T(VERB_DELETE))->SetEnabled(theApp.m_PathApplication[0]!=L'\0');

				if (e.flags & NSEQCF_CanRename)
					AddItem(e.menu, IDS_MENU_RENAME, _T(VERB_RENAME));

				AddSeparator(e.menu);
				AddItem(e.menu, IDS_MENU_PROPERTIES, _T(VERB_PROPERTIES))->SetEnabled(theApp.m_PathApplication[0]!=L'\0');
			}
		}
		break;
	case LEVELSTORES:
		if (e.children->GetCount()==0)
		{
			AddSeparator(e.menu);

			AddItem(e.menu, IDS_MENU_OPENFILEDROP, _T(VERB_OPENFILEDROP))->SetEnabled(theApp.m_PathApplication[0]!=L'\0');
			AddItem(e.menu, IDS_MENU_IMPORTFOLDER, _T(VERB_IMPORTFOLDER))->SetEnabled(theApp.m_PathApplication[0]!=L'\0');
		}
	case LEVELATTRIBUTE:
		if ((!(e.flags & NSEQCF_NoDefault)) && (e.children->GetCount()>=1))
		{
			AddSeparator(e.menu);
			AddItem(e.menu, IDS_MENU_CREATESHORTCUT, _T(VERB_CREATESHORTCUT));
		}
		break;
	case LEVELATTRVALUE:
		if ((!(e.flags & NSEQCF_NoDefault)) && (e.children->GetCount()>=1))
		{
			AddSeparator(e.menu);
			AddItem(e.menu, IDS_MENU_COPY, _T(VERB_COPY));
			AddSeparator(e.menu);
			AddItem(e.menu, IDS_MENU_CREATESHORTCUT, _T(VERB_CREATESHORTCUT));
			AddItem(e.menu, IDS_MENU_DELETE, _T(VERB_DELETE));
			if ((e.children->GetCount()==1) && (e.flags & NSEQCF_CanRename))
				AddItem(e.menu, IDS_MENU_RENAME, _T(VERB_RENAME));
		}
		break;
	}
}

BOOL CFolderItem::OnExecuteMenuItem(CExecuteMenuitemsEventArgs& e)
{
	if (e.menuItem->GetVerb()==_T(VERB_IMPORTFOLDER))
		return RunStoreCommand(e, theApp.m_PathApplication, _T("/IMPORTFOLDER "));

	if (e.menuItem->GetVerb()==_T(VERB_ADDSTORE))
		return RunPath(e.hWnd, theApp.m_PathApplication, _T("/ADDSTORE"));

	if (e.menuItem->GetVerb()==_T(VERB_EXPLORE))
		return OnExplorer(e);

	if (e.menuItem->GetVerb()==_T(VERB_OPENNEWWINDOW))
	{
		if (osInfo.dwMajorVersion<6)
			e.menuItem->SetVerb(_T(VERB_OPEN));

		return OnExplorer(e);
	}

	if (e.menuItem->GetVerb()==_T(VERB_OPENLIQUIDFOLDERS))
		return RunStoreCommand(e, theApp.m_PathApplication, _T(""));

	if (e.menuItem->GetVerb()==_T(VERB_OPENFILEDROP))
		return RunStoreCommand(e, theApp.m_PathApplication, _T("/FILEDROP "));

	if (e.menuItem->GetVerb()==_T(VERB_OPENWITH))
		return OnOpenWith(e);

	if (e.menuItem->GetVerb()==_T(VERB_OPENBROWSER))
	{
		POSITION pos = e.children->GetHeadPosition();
		CNSEItem* temp = (CNSEItem*)e.children->GetNext(pos);
		if (IS(temp, CFileItem))
		{
			CFileItem* file = AS(temp, CFileItem);

			ShellExecuteA(e.hWnd, "open", file->m_pItem->CoreAttributes.URL, NULL, NULL, SW_SHOW);
		}
	}

	if (e.menuItem->GetVerb()==_T(VERB_MAKEDEFAULTSTORE))
	{
		POSITION pos = e.children->GetHeadPosition();
		CNSEItem* temp = (CNSEItem*)e.children->GetNext(pos);
		if (IS(temp, CFolderItem))
		{
			CFolderItem* folder = AS(temp, CFolderItem);

			UINT Result = LFSetDefaultStore(folder->Attrs.StoreID);
			LFCoreErrorBox(Result);
			return (Result==LFOk);
		}

		return FALSE;
	}

	if (e.menuItem->GetVerb()==_T(VERB_CREATESHORTCUT))
	{
		// Create shortcut on desktop
		POSITION pos = e.children->GetHeadPosition();
		while(pos)
			CreateShortcut((CNSEItem*)e.children->GetNext(pos));

		return TRUE;
	}

	return FALSE;
}


// IShellBrowser

void CFolderItem::OnMergeFrameMenu(CMergeFrameMenuEventArgs& e)
{
	CShellMenuItem* item = e.menu->AddItem(_T("&liquidFOLDERS"));
	item->SetHasSubMenu(TRUE);

	CShellMenu* subMenu = item->GetSubMenu();

	AddItem(subMenu, IDS_MENU_ABOUT, _T(VERB_ABOUT))->SetEnabled(theApp.m_PathApplication[0]!=L'\0');
}

void CFolderItem::OnExecuteFrameCommand(CExecuteFrameCommandEventArgs& e)
{
	if(e.menuItem)
		if (e.menuItem->GetVerb()==_T(VERB_ABOUT))
			RunPath(NULL, theApp.m_PathApplication, _T("/ABOUT"));
}


// IExplorerCommandProvider

void CFolderItem::GetToolbarCommands(CPtrList& commands)
{
	switch (Attrs.Level)
	{
	case LEVELROOT:
		commands.AddTail(new CmdCreateNewStore());
		commands.AddTail(new CmdProperties());
		break;
	case LEVELSTORES:
		commands.AddTail(new CmdFileDrop(Attrs.StoreID));
		commands.AddTail(new CmdImportFolder(Attrs.StoreID));
		break;
	default:
		CNSEFolder::GetToolbarCommands(commands);
	}
}


// ICategoryProvider

CCategorizer* CFolderItem::GetCategorizer(CShellColumn& column)
{
	switch (Attrs.Level)
	{
	case LEVELROOT:
		return new CCategoryCategorizer(this, column);
	case LEVELSTORES:
		return new CAttributeCategorizer(this, column);
	case LEVELATTRIBUTE:
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

BOOL CFolderItem::GetColumn(CShellColumn& column, INT index)
{
	// Determine last column for level
	INT LastColumn = (Attrs.Level==LEVELSTORES) ? LFAttrFileFormat : LFAttrHashtags;
	if (index>LastColumn)
		return FALSE;

	// Simple settings
	column.name = theApp.m_Attributes[index].Name;
	column.width = theApp.m_Attributes[index].RecommendedWidth/7;	// Chars, not pixel
	column.fmt = theApp.m_Attributes[index].FormatRight ? NSESCF_Right : NSESCF_Left;
	column.state = (index>LFLastCoreAttribute) ? NSECS_SecondaryUi : (index==LFAttrFileName) ? NSECS_PreferVarCmp : NSECS_None;
	column.defaultVisible = (index!=LFAttrStoreID) && (index!=LFAttrFileID) && (index!=LFAttrFileCount) && (index!=LFAttrAddTime) && (index!=LFAttrArchiveTime) && (index!=LFAttrDeleteTime);
	column.index = index;
	if (theApp.m_Attributes[index].ShPropertyMapping.ID)
	{
		column.fmtid = theApp.m_Attributes[index].ShPropertyMapping.Schema;
		column.pid = theApp.m_Attributes[index].ShPropertyMapping.ID;
	}

	// Shell column data type
	switch (theApp.m_Attributes[index].Type)
	{
	case LFTypeUINT:
	case LFTypeSize:
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
	if (Attrs.Level<=LEVELATTRIBUTE)
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

	// Hidden columns
	ASSERT(LFLastCoreAttribute<=31);
	const UINT AttrMask[4] =
	{
		(1<<LFAttrFileName) | (1<<LFAttrStoreID) | (1<<LFAttrComments) | (1<<LFAttrDescription) | (1<<LFAttrCreationTime) | (1<<LFAttrFileTime) | (1<<LFAttrFileFormat) | (1<<LFAttrFileCount) | (1<<LFAttrFileSize),
		(1<<LFAttrFileName) | (1<<LFAttrComments) | (1<<LFAttrFileFormat),
		(1<<LFAttrFileName) | (1<<LFAttrComments) | (1<<LFAttrFileCount) | (1<<LFAttrFileSize) | (1<<LFAttrFileFormat),
		(UINT)~((1<<LFAttrDescription) | (1<<LFAttrArchiveTime) | (1<<LFAttrDeleteTime) | (1<<LFAttrFileCount) | (1<<LFAttrFlags))
	};
	if (!(AttrMask[Attrs.Level] & (1<<index)))
		column.state = NSECS_Hidden;

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
			((SHDESCRIPTIONID*)value->parray->pvData)->dwDescriptionId = (Attrs.CategoryID==LFItemCategoryRemote) ? SHDID_COMPUTER_NETDRIVE : 20;
			value->vt = VT_ARRAY | VT_UI1;
			return TRUE;
		case 6:
			CUtils::SetVariantLPCTSTR(value, _T("{3F2D914F-FE57-414F-9F88-A377C7841DA4}"));
			return TRUE;
		case 9:
			CUtils::SetVariantINT(value, -1);
			return TRUE;
		case 11:
			CUtils::SetVariantLPCTSTR(value, _T("Folder"));
			return TRUE;
		default:
			return FALSE;
		}

	if (column.fmtid==FMTID_Volume)
		switch (column.pid)
		{
		case 2:
			CUtils::SetVariantINT64(value, 0);
			return TRUE;
		case 3:
			CUtils::SetVariantINT64(value, Attrs.Size);
			return TRUE;
		case 4:
			CUtils::SetVariantLPCTSTR(value, _T("liquidFOLDERS"));
			return TRUE;
		case 5:
			CUtils::SetVariantUINT(value, 100);
			return TRUE;
		case 10:
			CUtils::SetVariantBOOL(value, TRUE);
			return TRUE;
		default:
			return FALSE;
		}

	if (column.fmtid==FMTID_Storage)
		switch (column.pid)
		{
		case 4:
			CUtils::SetVariantCString(value, Attrs.Level==LEVELSTORES ? theApp.m_Store : theApp.m_Folder);
			return TRUE;
		case 10:
			CUtils::SetVariantLPCTSTR(value, Attrs.DisplayName);
			return TRUE;
		case 12:
			if (value->vt==VT_BSTR)
			{
				WCHAR tmpBuf[256];
				LFSizeToString(Attrs.Size, tmpBuf, 256);
				CUtils::SetVariantLPCTSTR(value, tmpBuf);
			}
			else
			{
				CUtils::SetVariantINT64(value, Attrs.Size);
			}
			return TRUE;
		case 13:
			CUtils::SetVariantUINT(value, 0x2016);
			return TRUE;
		case 14:
			if ((Attrs.FileTime.dwHighDateTime) || (Attrs.FileTime.dwLowDateTime))
			{
				CUtils::SetVariantFILETIME(value, Attrs.FileTime);
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		case 15:
			if ((Attrs.CreationTime.dwHighDateTime) || (Attrs.CreationTime.dwLowDateTime))
			{
				CUtils::SetVariantFILETIME(value, Attrs.CreationTime);
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		default:
			return FALSE;
		}

	const GUID FMTID_ManagedVolume = { 0x149C0B69, 0x2C2D, 0x48FC, { 0x80, 0x8F, 0xD3, 0x18, 0xD7, 0x8C, 0x46, 0x36 } };
	if (column.fmtid==FMTID_ManagedVolume)
		switch (column.pid)
		{
		case 2:
			CUtils::SetVariantBOOL(value, FALSE);
			return TRUE;
		default:
			return FALSE;
		}

	const GUID FMTID_Preview = { 0xC9944A21, 0xA406, 0x48FE, { 0x82, 0x25, 0xAE, 0xC7, 0xE2, 0x4C, 0x21, 0x1B } };
	if (column.fmtid==FMTID_Preview)
		switch (column.pid)
		{
		case 6:
			CUtils::SetVariantLPCTSTR(value, _T("prop:~System.ItemNameDisplay;~System.ItemTypeText"));
			return TRUE;
		default:
			return FALSE;
		}

	switch (column.index)
	{
	case LFAttrFileName:
		CUtils::SetVariantLPCTSTR(value, Attrs.DisplayName);
		break;
	case LFAttrFileID:
		CUtils::SetVariantLPCTSTR(value, CString(Attrs.FileID));
		break;
	case LFAttrStoreID:
		CUtils::SetVariantLPCTSTR(value, CString(Attrs.StoreID));
		break;
	case LFAttrDescription:
		CUtils::SetVariantLPCTSTR(value, Attrs.Description);
		break;
	case LFAttrComments:
		CUtils::SetVariantLPCTSTR(value, Attrs.Comments);
		break;
	case LFAttrCreationTime:
		if ((Attrs.CreationTime.dwHighDateTime) || (Attrs.CreationTime.dwLowDateTime))
		{
			CUtils::SetVariantFILETIME(value, Attrs.CreationTime);
		}
		else
		{
			return FALSE;
		}
		break;
	case LFAttrFileTime:
		if ((Attrs.FileTime.dwHighDateTime) || (Attrs.FileTime.dwLowDateTime))
		{
			CUtils::SetVariantFILETIME(value, Attrs.FileTime);
		}
		else
		{
			return FALSE;
		}
		break;
	case LFAttrFileFormat:
		CUtils::SetVariantCString(value, Attrs.Level==LEVELSTORES ? theApp.m_Store : theApp.m_Folder);
		break;
	case LFAttrFileCount:
		CUtils::SetVariantUINT(value, Attrs.Count);
		break;
	case LFAttrFileSize:
		if (value->vt==VT_BSTR)
		{
			WCHAR tmpBuf[256];
			LFSizeToString(Attrs.Size, tmpBuf, 256);
			CUtils::SetVariantLPCTSTR(value, tmpBuf);
		}
		else
		{
			CUtils::SetVariantINT64(value, Attrs.Size);
		}
		break;
	default:
		{
			return FALSE;
		}
	}

	return TRUE;
}


// IShellFolder

NSEItemAttributes CFolderItem::GetAttributes(NSEItemAttributes requested)
{
	UINT Mask = NSEIA_CFOLDERITEM;

	if (Attrs.Level==LEVELSTORES)
	{
		Mask |= NSEIA_CanRename;
		if (theApp.m_PathApplication[0]!=L'\0')
			Mask |= NSEIA_CanDelete | NSEIA_HasPropSheet;
	}
	if (Attrs.Level<LEVELATTRVALUE)
		Mask |= NSEIA_HasSubFolder;

	return (NSEItemAttributes)(requested & Mask);
}

INT CFolderItem::CompareTo(CNSEItem* otherItem, CShellColumn& column)
{
	if (IS(otherItem, CFileItem))
		return -1;

	CFolderItem* dir2 = AS(otherItem, CFolderItem);

	CString str1;
	CString str2;
	INT ret = 0;

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
	case LFAttrComments:
		str1 = Attrs.Comments;
		str2 = dir2->Attrs.Comments;
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
	ret = _wcsicmp(Attrs.DisplayName, dir2->Attrs.DisplayName);
	if (ret)
		return ret;

	// Compare store IDs
	ret = strcmp(Attrs.StoreID, dir2->Attrs.StoreID);
	if (ret)
		return ret;

	// Compare file IDs
	return strcmp(Attrs.FileID, dir2->Attrs.FileID);
}

BOOL CFolderItem::OnOpen(CExecuteMenuitemsEventArgs& e)
{
	if (e.children->GetCount()==1)
	{
		POSITION pos = e.children->GetHeadPosition();
		CNSEItem* item = (CNSEItem*)e.children->GetNext(pos);

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
			WCHAR Path[MAX_PATH];
			UINT Result = LFGetFileLocation(AS(item, CFileItem)->m_pItem, Path, MAX_PATH, TRUE);
			if (Result!=LFOk)
			{
				LFCoreErrorBox(Result);
			}
			else
				if (ShellExecute(e.hWnd, _T("open"), Path, _T(""), _T(""), SW_SHOW)==(HINSTANCE)SE_ERR_NOASSOC)
				{
					WCHAR Cmd[300];
					wcscpy_s(Cmd, 300, L"shell32.dll,OpenAs_RunDLL ");
					wcscat_s(Cmd, 300, Path);
					ShellExecute(e.hWnd, _T("open"), _T("rundll32.exe"), Cmd, Path, SW_SHOW);
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
	if ((Attrs.Level==LEVELROOT) && (theApp.m_PathApplication[0]!=L'\0'))
	{
		POSITION pos = e.children->GetHeadPosition();
		CNSEItem* item = (CNSEItem*)e.children->GetNext(pos);

		if (IS(item, CFolderItem))
		{
			CString StoreID(AS(item, CFolderItem)->Attrs.StoreID);
			return RunPath(e.hWnd, theApp.m_PathApplication, _T("/DELETESTORE ")+StoreID);
		}
	}

	// Files
	BOOL Result = FALSE;
	LFTransactionList* tl = LFAllocTransactionList();

	POSITION pos = e.children->GetHeadPosition();
	while (pos)
	{
		CNSEItem* item = (CNSEItem*)e.children->GetNext(pos);

		if (IS(item, CFileItem))
			LFAddTransactionItemEx(tl, AS(item, CFileItem)->m_pItem->StoreID, AS(item, CFileItem)->m_pItem->CoreAttributes.FileID, NULL, (UINT_PTR)item);
	}

	if (tl->m_ItemCount)
	{
		LFDoTransaction(tl, LFTransactionTypePutInTrash);

		for (UINT a=0; a<tl->m_ItemCount; a++)
			if ((tl->m_Items[a].Processed) && (tl->m_Items[a].LastError==LFOk))
				((CFileItem*)tl->m_Items[a].UserData)->Delete();

		Result = TRUE;
	}

	LFCoreErrorBox(tl->m_LastError);
	LFFreeTransactionList(tl);

	return Result;
}

BOOL CFolderItem::OnChangeName(CChangeNameEventArgs& e)
{
	// Stores sind die einzigen CFolderItem, die umbenannt werden können
	if (Attrs.Level==LEVELSTORES)
	{
		UINT Result = LFSetStoreAttributes(Attrs.StoreID, e.newName.GetBuffer(), NULL);
		if (Result==LFOk)
		{
			wcscpy_s(Attrs.DisplayName, 256, e.newName);
		}
		else
		{
			LFCoreErrorBox(Result);
		}

		return (Result==LFOk);
	}

	return FALSE;
}


// IDropSource

void CFolderItem::InitDataObject(CInitDataObjectEventArgs& e)
{
	if (e.children->GetCount())
	{
		LFTransactionList* tl = LFAllocTransactionList();

		POSITION pos = e.children->GetHeadPosition();
		while (pos)
		{
			CNSEItem* item = (CNSEItem*)e.children->GetNext(pos);

			if (IS(item, CFileItem))
				LFAddTransactionItemEx(tl, AS(item, CFileItem)->m_pItem->StoreID, AS(item, CFileItem)->m_pItem->CoreAttributes.FileID);
		}

		e.dataObject->SetHasFileData();
		if (tl->m_ItemCount)
			e.dataObject->SetHGlobalData(CFSTR_LIQUIDFILES, LFCreateLiquidFiles(tl));

		LFFreeTransactionList(tl);

		e.dataObject->SetPreferredDropEffect(DROPEFFECT_COPY);
	}
}

BOOL CFolderItem::GetFileDescriptor(FILEDESCRIPTOR* fd)
{
	_tcscpy(fd->cFileName, Attrs.DisplayName);

	fd->dwFlags = FD_ATTRIBUTES | FD_CLSID;
	fd->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	fd->clsid = guid;

	switch (Attrs.Level)
	{
	case LEVELROOT:
		fd->dwFlags |= FD_WRITESTIME | FD_CREATETIME;
		fd->ftCreationTime = Attrs.CreationTime;
		fd->ftLastWriteTime = Attrs.FileTime;
		break;
	case LEVELSTORES:
		LARGE_INTEGER sz;
		sz.QuadPart = Attrs.Size;

		fd->dwFlags |= FD_FILESIZE;
		fd->nFileSizeHigh = sz.HighPart;
		fd->nFileSizeLow = sz.LowPart;
		break;
	}

	return TRUE;
}


// IDropTarget

void CFolderItem::DragEnter(CNSEDragEventArgs& e)
{
	e.allowedEffect = DROPEFFECT_MOVE | DROPEFFECT_COPY;
	e.effect = (e.keyState & MK_CONTROL) ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
}

void CFolderItem::DragOver(CNSEDragEventArgs& e)
{
	e.allowedEffect = DROPEFFECT_MOVE | DROPEFFECT_COPY;
	e.effect = (e.keyState & MK_CONTROL) ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
}

void CFolderItem::DragDrop(CNSEDragEventArgs& e)
{
	e.data->SetPerformedDropEffect(theApp.ImportFiles(Attrs.StoreID, e.data, e.keyState & MK_CONTROL));
	e.data->SetPasteSucceded(e.data->GetPerformedDropEffect());

	RefreshView();
}


// Exposed property handlers

INT CFolderItem::GetXPTaskPaneColumnIndices(UINT* indices)
{
	indices[0] = LFAttrFileName;
	indices[1] = LFAttrComments;
	indices[2] = LFAttrDescription;

	switch (Attrs.Level)
	{
	case LEVELSTORES:
		indices[2] = LFAttrCreationTime;
		indices[3] = LFAttrFileTime;
		indices[4] = LFAttrDescription;
		return 5;
	case LEVELATTRVALUE:
		indices[3] = LFAttrFileSize;
		return (strcmp(Attrs.FileID, "ALL")==0) ? 3 : 4;
	case LEVELATTRIBUTE:
		return 2;
	}

	return 3;
}

INT CFolderItem::GetTileViewColumnIndices(UINT* indices)
{
	indices[0] = LFAttrComments;
	indices[1] = LFAttrDescription;

	return 2;
}

INT CFolderItem::GetPreviewDetailsColumnIndices(UINT* indices)
{
	indices[0] = LFAttrComments;
	indices[1] = LFAttrDescription;

	switch (Attrs.Level)
	{
	case LEVELSTORES:
		indices[1] = LFAttrCreationTime;
		indices[2] = LFAttrFileTime;
		indices[3] = LFAttrDescription;
		return 4;
	case LEVELATTRVALUE:
		indices[2] = LFAttrFileSize;
		return (strcmp(Attrs.FileID, "ALL")==0) ? 2 : 3;
	case LEVELATTRIBUTE:
		return 1;
	}

	return 2;
}

INT CFolderItem::GetContentViewColumnIndices(UINT* indices)
{
	return GetXPTaskPaneColumnIndices(indices);
}


// Other

BOOL CFolderItem::SetShellLink(IShellLink* pShellLink)
{
	ASSERT(pShellLink);

	pShellLink->SetIDList(GetPIDLAbsolute());
	pShellLink->SetIconLocation(theApp.m_PathCoreFile, Attrs.Icon-1);
	pShellLink->SetShowCmd(SW_SHOWNORMAL);
	pShellLink->SetDescription(Attrs.Comments);

	return TRUE;
}

void CFolderItem::CreateShortcut(CNSEItem* Item)
{
	// Get a pointer to the IShellLink interface
	IShellLink* pShellLink = NULL;
	if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pShellLink)))
	{
		BOOL Result = FALSE;
		if (IS(Item, CFolderItem))
			Result = AS(Item, CFolderItem)->SetShellLink(pShellLink);
		if (IS(Item, CFileItem))
			Result = AS(Item, CFileItem)->SetShellLink(pShellLink);

		if (Result)
		{
			CString LinkFilename;
			Item->GetDisplayName(LinkFilename);

			LFCreateDesktopShortcut(pShellLink, LinkFilename.GetBuffer());
		}

		pShellLink->Release();
	}
}

BOOL CFolderItem::RunStoreCommand(CExecuteMenuitemsEventArgs& e, CString Path, CString Parameter)
{
	CString StoreID;
	if (Attrs.Level==LEVELROOT)
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

	if ((StoreID.IsEmpty()) || (Path.IsEmpty()))
		return FALSE;

	Parameter.Append(StoreID);

	return RunPath(e.hWnd, Path, Parameter);
}

BOOL CFolderItem::OnProperties(CExecuteMenuitemsEventArgs& e)
{
	return RunStoreCommand(e, theApp.m_PathApplication, _T("/STOREPROPERTIES "));
}

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
			WCHAR Path[MAX_PATH];
			UINT Result = LFGetFileLocation(AS(item, CFileItem)->m_pItem, Path, MAX_PATH, TRUE);
			if (Result!=LFOk)
			{
				LFCoreErrorBox(Result);
			}
			else
			{
				WCHAR Cmd[300];
				wcscpy_s(Cmd, 300, L"shell32.dll,OpenAs_RunDLL ");
				wcscat_s(Cmd, 300, Path);
				ShellExecute(e.hWnd, _T("open"), _T("rundll32.exe"), Cmd, Path, SW_SHOW);
			}

			return TRUE;
		}
	}

	return FALSE;
}
