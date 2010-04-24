
#include "stdafx.h"
#include "LFNamespaceExtension.h"
#include "CFileItem.h"
#include "CFolderItem.h"
#include "LFCore.h"
#include "liquidFOLDERS.h"
#include "CCategoryCategorizer.h"
#include "CAttributeCategorizer.h"
#include <io.h>


IMPLEMENT_DYNCREATE(CFolderItem, CNSEFolder)

// The GUID of the class representing the root folder is used as the GUID for the namespace extension
// 3F2D914F-FE57-414F-9F88-A377C7841DA4
IMPLEMENT_OLECREATE_EX(CFolderItem, _T("LFNamespaceExtension.RootFolder"),
	0x3f2d914f, 0xfe57, 0x414f, 0x9f, 0x88, 0xa3, 0x77, 0xc7, 0x84, 0x1d, 0xa4)


// This function is called when you register the namespace extension dll file
// using the regsvr32.exe or similar utility

// The classfactory is nested in your class and has a name formed
// by concatenating the class name with "Factory".
BOOL CFolderItem::CFolderItemFactory::UpdateRegistry(BOOL bRegister)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (bRegister)
	{
		BOOL ret = AfxOleRegisterServerClass(m_clsid, m_lpszProgID, m_lpszProgID, m_lpszProgID, OAT_DISPATCH_OBJECT);
		
		// Register the namespace extension
		CNSEFolder::RegisterExtension(RUNTIME_CLASS(CFolderItem));
		return ret; 
	}
	else
	{
		// Unregister the namespace extension
		CNSEFolder::UnregisterExtension(RUNTIME_CLASS(CFolderItem));
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
	}
}


// Class CFolderItem

CFolderItem::CFolderItem()
{
	data.Level = LevelRoot;
}

CFolderItem::CFolderItem(FolderSerialization _data)
{
	data = _data;
}

void CFolderItem::GetCLSID(LPCLSID pCLSID)
{
	*pCLSID = guid;
}

void CFolderItem::GetExtensionTargetInfo(CExtensionTargetInfo& info)
{
	CNSETargetInfo* nti = new CNSETargetInfo();

	nti->nseTarget=NSET_MyComputer;
	nti->name = _T("liquidFOLDERS");
	nti->infoTip.LoadStringA(IDS_MyComputerHint);
	nti->attributes = (NSEItemAttributes)(NSEIA_CFOLDERITEM | NSEIA_HasSubFolder);
	nti->iconFile = theApp.m_IconFile;
	nti->iconIndex = IDI_FLD_Default-1;

	info.AddTarget(nti);
}

NSEItemAttributes CFolderItem::GetAttributes(NSEItemAttributes /*requested*/)
{
	int ret = NSEIA_CFOLDERITEM;

	if (data.Level==LevelStores)
		ret |= NSEIA_CanRename | NSEIA_CanDelete;
	if (data.Level<=LevelAttribute)
		ret |= NSEIA_HasSubFolder;

	return (NSEItemAttributes)ret;
}

void CFolderItem::Serialize(CArchive& ar)
{
	ar << (BYTE)CLFNamespaceExtensionVersion;
	ar << (BYTE)1;
	ar << data.Level;
	ar << data.Icon;
	ar << data.Type;
	ar << data.CategoryID;
	ar << data.DisplayName;
	ar << data.Hint;
	ar << data.Comment;
	ar << data.FileID;
	ar << data.StoreID;
	ar << data.CreationTime.dwHighDateTime;
	ar << data.CreationTime.dwLowDateTime;
	ar << data.FileTime.dwHighDateTime;
	ar << data.FileTime.dwLowDateTime;
	ar << data.DomainID;
	ar << data.AttributeID;
	ar << data.AttributeValue;
}

CNSEItem* CFolderItem::DeserializeChild(CArchive& ar)
{
	BYTE version;
	ar >> version;

	if (version!=CLFNamespaceExtensionVersion)
		return NULL;

	BYTE ItemType;
	ar >> ItemType;

	switch (ItemType)
	{
	case 0:
		break;
	case 1:
		FolderSerialization d;
		ar >> d.Level;
		ar >> d.Icon;
		ar >> d.Type;
		ar >> d.CategoryID;
		ar >> d.DisplayName;
		ar >> d.Hint;
		ar >> d.Comment;
		ar >> d.FileID;
		ar >> d.StoreID;
		ar >> d.CreationTime.dwHighDateTime;
		ar >> d.CreationTime.dwLowDateTime;
		ar >> d.FileTime.dwHighDateTime;
		ar >> d.FileTime.dwLowDateTime;
		ar >> d.DomainID;
		ar >> d.AttributeID;
		ar >> d.AttributeValue;

		return new CFolderItem(d);
		break;
	}

	return NULL;
}

BOOL CFolderItem::GetChildren(CGetChildrenEventArgs& e)
{
	LFFilter* f = NULL;
	LFSearchResult* res = NULL;

	switch (data.Level)
	{
	case LevelRoot:
		f = LFAllocFilter();
		f->Mode = LFFilterModeStores;
		f->Legacy = true;
		res = LFQuery(f);
		break;
	case LevelStores:
		f = LFAllocFilter();
		f->Mode = LFFilterModeStoreHome;
		f->Legacy = true;
		strcpy(f->StoreID, (LPCTSTR)data.StoreID);
		res = LFQuery(f);
		break;
	case LevelStoreHome:
		if (e.childrenType & NSECT_Folders)
		{
			CString sortStr;
			BOOL bValidName = sortStr.LoadString(IDS_AttributeHint);
			ASSERT(bValidName);

			FolderSerialization d;
			d.Level = data.Level+2;
			d.Type = data.Type;
			d.DisplayName.LoadString(IDS_AllFiles);
			d.Hint.LoadString(IDS_AllFilesHint);
			d.CategoryID = LFAttrCategoryCount;
			d.FileID = "ALL";
			d.StoreID = data.StoreID;
			d.DomainID = data.DomainID;
			d.Icon = IDI_FLD_All;
			e.children->AddTail(new CFolderItem(d));

			for (UINT a=0; a<LFAttributeCount; a++)
				if (theApp.m_Domains[data.DomainID]->ImportantAttributes->IsSet(a))
				{
					FolderSerialization d;
					d.Level = data.Level+1;
					d.Type = data.Type;
					d.CategoryID = data.CategoryID;
					d.DisplayName = theApp.m_Attributes[a]->Name;
					d.CategoryID = theApp.m_Attributes[a]->Category;
					d.FileID.Format(_T("%d"), a);
					d.StoreID = data.StoreID;
					d.DomainID = data.DomainID;

					switch (a)
					{
					case LFAttrFileTime:
					case LFAttrRecordingTime:
					case LFAttrCreationTime:
					case LFAttrDuration:
					case LFAttrDueTime:
					case LFAttrDoneTime:
						d.Icon = IDI_FLD_Calendar;
						break;
					case LFAttrRating:
						d.Icon = IDI_FLD_Favorites;
						break;
					case LFAttrRoll:
						d.Icon = IDI_FLD_Roll;
						break;
					case LFAttrLocationName:
					case LFAttrLocationIATA:
					case LFAttrLocationGPS:
						d.Icon = IDI_FLD_Location;
						break;
					case LFAttrArtist:
					case LFAttrResponsible:
						d.Icon = IDI_FLD_Contacts;
						break;
					case LFAttrLanguage:
						d.Icon = IDI_FLD_Fonts;
						break;
					default:
						d.Icon = IDI_FLD_Default;
					}

					CString tmpStr = d.DisplayName;
					if ((sortStr[0]=='L') && (tmpStr[0]>='A') && (tmpStr[0]<='Z') && (tmpStr[1]>'Z'))
						tmpStr = tmpStr.MakeLower().Mid(0, 1)+tmpStr.Mid(1, tmpStr.GetLength()-1);
					d.Hint.Format(sortStr.Mid(1, sortStr.GetLength()-1), tmpStr);

					e.children->AddTail(new CFolderItem(d));
				}
		}
		break;
	}

	if (res)
	{
		if (res->m_LastError!=LFOk)
		{
			LFFreeSearchResult(res);
			LFFreeFilter(f);
			return FALSE;
		}

		for (UINT a=0; a<res->m_Count; a++)
		{
			LFItemDescriptor* i = res->m_Files[a];

			if ((i->CoreAttributes.FileFormat == 0) && (e.childrenType & NSECT_Folders))
			{
				FolderSerialization d;
				d.Level = data.Level+1;
				d.Icon = i->IconID;
				d.Type = i->Type;
				d.CategoryID = i->CategoryID;
				d.DisplayName = i->CoreAttributes.FileName;
				d.Comment = i->CoreAttributes.Comment;
				d.Hint = i->Hint;
				d.StoreID = i->CoreAttributes.StoreID;
				d.FileID = i->CoreAttributes.FileID;
				d.CreationTime = i->CoreAttributes.CreationTime;
				d.FileTime = i->CoreAttributes.FileTime;

				switch (data.Level)
				{
				case LevelStores:
					d.DomainID = atoi(d.FileID);

					switch (d.DomainID)
					{
					case LFDomainTrash:
					case LFDomainUnknown:
						d.Level = LevelAttrValue;
						break;
					case LFDomainFavorites:
						d.Level = LevelAttribute;
						d.AttributeID = LFAttrRating;
					}
					break;
				}

				e.children->AddTail(new CFolderItem(d));
			}

			if ((i->CoreAttributes.FileFormat != 0) && (e.childrenType & NSECT_NonFolders))
			{
			}
		}
		LFFreeSearchResult(res);
	}

	LFFreeFilter(f);
	return TRUE;
}

void CFolderItem::GetDisplayName(CString& displayName)
{
	displayName = data.DisplayName;
}

void CFolderItem::GetDisplayNameEx(CString& displayName, DisplayNameFlags flags)
{
	// If a fully qualified parsing name is requested, return the full path
	if ((flags & NSEDNF_ForParsing) && (!(flags & NSEDNF_ForAddressBar)))
	{
		// If a fully qualified parsing name is requested, return the full path
		if (!(flags & NSEDNF_InFolder))
		{
			WCHAR buf[39];
			CString id;
			StringFromGUID2(guid, buf, 39);
			id= buf;
			displayName = "::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::"+id;

			if (data.Level>LevelStores)
				displayName += '\\'+data.StoreID;
			if (data.FileID!="")
				displayName += '\\'+data.FileID;
		}
		else
		{
			displayName = data.FileID;
		}

		return;
	}

	// For all other types, return the default name
	CNSEFolder::GetDisplayNameEx(displayName, flags);
}

CNSEItem* CFolderItem::GetChildFromDisplayName(CGetChildFromDisplayNameEventArgs& /*e*/)
{
//	AfxMessageBox(e.displayName);
/*	if(e.assumeChildExists)
	{
		return new CFileItem(this->fullPath, e.displayName);
	}
	else
	{
		// Return only if file truly exists
		if(FileExists(PathCombineNSE(this->fullPath,e.displayName)))
		{
			return new CFileItem(this->fullPath, e.displayName);
		}
		else if (DirectoryExists(PathCombineNSE(this->fullPath, e.displayName)))
		{
			return new CFolderItem(this->fullPath, e.displayName);
		}
	}*/
	return NULL;
}

void CFolderItem::GetIconFileAndIndex(CGetIconFileAndIndexEventArgs& e)
{
	e.iconExtractMode = NSEIEM_IconFileAndIndex;
	e.iconFile = theApp.m_IconFile;
	e.iconIndex = data.Icon-1;
}

void CFolderItem::GetInfoTip(CString& infotip)
{
	infotip = data.Hint;
}

int CFolderItem::GetXPTaskPaneColumnIndices(UINT* indices)
{
	indices[0] = LFAttrFileName;
	indices[1] = LFAttrCreationTime;
	indices[2] = LFAttrHint;
	return 3;
}

int CFolderItem::GetTileViewColumnIndices(UINT* indices)
{
	indices[0] = LFAttrCreationTime;
	indices[1] = LFAttrHint;
	return 2;
}

int CFolderItem::GetPreviewDetailsColumnIndices(UINT* indices)
{
	indices[0] = LFAttrFileName;
	indices[1] = LFAttrCreationTime;
	indices[2] = LFAttrHint;
	return 3;
}

CCategorizer* CFolderItem::GetCategorizer(CShellColumn &column)
{
	switch (data.Level)
	{
	case LevelRoot:
	case LevelStores:
		return new CCategoryCategorizer(this, column);
	case LevelStoreHome:
		return new CAttributeCategorizer(this, column);
	default:
		return CFolderItem::GetCategorizer(column);
	}
}

FolderThemes CFolderItem::GetFolderTheme()
{
	FolderThemes t = NSEFT_None;
	if (data.Level>=LevelStoreHome)
		switch (data.DomainID)
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
		case LFDomainAllMultimediaFiles:
		case LFDomainFavorites:
			t = NSEFT_Search;
			break;
		}

	return t;
}

BOOL CFolderItem::GetColumn(CShellColumn& column, int index)
{
	int limit;
	switch (data.Level)
	{
	case LevelRoot:
		limit = LFAttrFileTime;
		break;
	case LevelAttrValue:
		limit = LFAttributeCount;
		break;
	default:
		limit = LFAttrHint;
	}
	if (index>limit)
		return FALSE;

	column.name = theApp.m_Attributes[index]->Name;
	column.width = theApp.m_Attributes[index]->RecommendedWidth/7;  // Chars, not pixel
	column.fmt = ((theApp.m_Attributes[index]->Type >= LFTypeUINT) || (index==LFAttrStoreID) || (index==LFAttrFileID)) ? NSESCF_Right : NSESCF_Left;
	column.categorizerType = NSECT_Alphabetical;
	column.index = index;

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

	switch (index)
	{
	case LFAttrFileName:
		column.fmtid = GUID_FMTID_NAME;
		column.pid = 10;
		column.state = NSECS_PreferVarCmp;
		break;
	case LFAttrStoreID:
		column.categorizerType = NSECT_String;
		break;
	case LFAttrFileFormat:
		column.fmtid = GUID_FMTID_NAME;
		column.pid = 4;
		column.categorizerType = NSECT_String;
		break;
	case LFAttrFileSize:
		column.fmtid = GUID_FMTID_NAME;
		column.pid = 12;
		column.categorizerType = NSECT_String;
		break;
	case LFAttrCreationTime:
		column.fmtid = GUID_FMTID_PROPERTY;
		column.pid = 12;
		column.categorizerType = NSECT_String;
		break;
	case LFAttrFileTime:
		column.fmtid = GUID_FMTID_PROPERTY;
		column.pid = 13;
		column.categorizerType = NSECT_String;
		break;
	case LFAttrHint:
		if ((data.Level<LevelAttrValue) && (osInfo.dwMajorVersion>=6))
		{
			column.fmtid = GUID_FMTID_PROPERTY;
			column.pid = 6;
		}
		break;
	case LFAttrComment:
		if (data.Level==LevelAttrValue)
		{
			column.fmtid = GUID_FMTID_PROPERTY;
			column.pid = 6;
		}
		break;
	case LFAttrTitle:
		column.fmtid = GUID_FMTID_PROPERTY;
		column.pid = 2;
	case LFAttrArtist:
		column.fmtid = GUID_FMTID_PROPERTY;
		column.pid = 4;
		column.categorizerType = NSECT_String;
		break;
	}

	if (data.Level<=LevelStoreHome)
		column.categorizerType = NSECT_Custom;

	return TRUE;
}

BOOL CFolderItem::GetColumnValueEx(VARIANT* value, CShellColumn& column)
{
	switch (column.index)
	{
	case LFAttrFileName:
		CUtils::SetVariantCString(value, data.DisplayName);
		break;
	case LFAttrFileID:
		CUtils::SetVariantCString(value, data.FileID);
		break;
	case LFAttrStoreID:
		CUtils::SetVariantCString(value, data.StoreID);
		break;
	case LFAttrHint:
		CUtils::SetVariantCString(value, data.Hint);
		break;
	case LFAttrComment:
		CUtils::SetVariantCString(value, data.Comment);
		break;
	case LFAttrCreationTime:
		if ((data.CreationTime.dwHighDateTime) || (data.CreationTime.dwHighDateTime))
		{
			CUtils::SetVariantFILETIME(value, data.CreationTime);
		}
		else
		{
			return FALSE;
		}
		break;
	case LFAttrFileTime:
		if ((data.FileTime.dwHighDateTime) || (data.FileTime.dwHighDateTime))
		{
			CUtils::SetVariantFILETIME(value, data.FileTime);
		}
		else
		{
			return FALSE;
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

BOOL CFolderItem::IsValid()
{
	return TRUE;
}

void CFolderItem::GetMenuItems(CGetMenuitemsEventArgs& e)
{
	CString tmpStr;
	CString tmpHint;

	if (e.menu->GetItemCount()>0)
		e.menu->AddItem(_T(""))->SetSeparator(TRUE);

	switch (data.Level)
	{
	case LevelRoot:
		if (e.children->GetCount()==0)
		{
			ENSURE(tmpStr.LoadString(IDS_MENU_CreateNewStore));
			ENSURE(tmpHint.LoadString(IDS_HINT_CreateNewStore));
			e.menu->AddItem(tmpStr, _T(VERB_CREATENEWSTORE), tmpHint);
		}
		if (e.children->GetCount()==1)
		{
			CFolderItem* f = (CFolderItem*)e.children->GetHead();
			ENSURE(tmpStr.LoadString(IDS_MENU_MakeDefaultStore));
			ENSURE(tmpHint.LoadString(IDS_HINT_MakeDefaultStore));
			CShellMenuItem* i = e.menu->AddItem(tmpStr, _T(VERB_MAKEDEFAULTSTORE), tmpHint);
			i->SetEnabled((f->data.Type & LFTypeStore) && (f->data.CategoryID==LFStoreModeInternal));
			ENSURE(tmpStr.LoadString(IDS_MENU_MakeHybridStore));
			ENSURE(tmpHint.LoadString(IDS_HINT_MakeHybridStore));
			i = e.menu->AddItem(tmpStr, _T(VERB_MAKEHYBRIDSTORE), tmpHint);
			i->SetEnabled((f->data.Type & LFTypeStore) && (f->data.CategoryID==LFStoreModeExternal));
		}
		break;
	}

	if (e.children->GetCount()>=1)
	{
		ENSURE(tmpStr.LoadString(IDS_MENU_CreateLink));
		ENSURE(tmpHint.LoadString(IDS_HINT_CreateLink));
		e.menu->AddItem(tmpStr, _T(VERB_CREATELINKDESKTOP), tmpHint);
	}
}

BOOL CFolderItem::OnExecuteMenuItem(CExecuteMenuitemsEventArgs& e)
{
	if (e.menuItem->GetVerb()==_T(VERB_CREATENEWSTORE))
	{
		LFStoreDescriptor* s = LFAllocStoreDescriptor();
		s->AutoLocation = TRUE;
		s->StoreMode = LFStoreModeInternal;

		UINT res = LFCreateStore(s);
		if (res!=LFOk)
		{
			wchar_t* tmpStr = LFGetErrorText(res);
			MessageBoxW(NULL, tmpStr, L"liquidFOLDERS", MB_OK | MB_ICONERROR);
			free(tmpStr);
		}
		else
		{
			UpdateItems(TRUE);
		}

		LFFreeStoreDescriptor(s);
		return (res==LFOk);
	}

	if ((e.menuItem->GetVerb()==_T(VERB_MAKEDEFAULTSTORE)) || (e.menuItem->GetVerb()==_T(VERB_MAKEHYBRIDSTORE)))
	{
		POSITION pos = e.children->GetHeadPosition();
		CNSEItem* temp = (CNSEItem*)e.children->GetNext(pos);
		if (IS(temp, CFolderItem))
		{
			CFolderItem* folder = AS(temp, CFolderItem);
			char key[LFKeySize];
			strcpy_s(key, LFKeySize, folder->data.StoreID);

			UINT res;
			if (e.menuItem->GetVerb()==_T(VERB_MAKEDEFAULTSTORE))
			{
				res = LFMakeDefaultStore(&key[0]);
			}
			else
			{
				res = LFMakeHybridStore(&key[0]);
			}

			if (res!=LFOk)
			{
				wchar_t* tmpStr = LFGetErrorText(res);
				MessageBoxW(NULL, tmpStr, L"liquidFOLDERS", MB_OK | MB_ICONERROR);
				free(tmpStr);
			}
			else
			{
				UpdateItems(FALSE);
			}

			return (res==LFOk);
		}

		return FALSE;
	}

	if ((e.menuItem->GetVerb()==_T(VERB_CREATELINK)) || (e.menuItem->GetVerb()==_T(VERB_CREATELINKDESKTOP)))
	{
		// Ask if link should be created on desktop
		if (e.menuItem->GetVerb()==_T(VERB_CREATELINK))
		{
			CString tmpStr;
			CString tmpCaption;

			ENSURE(tmpStr.LoadString(IDS_TEXT_CreateLink));
			ENSURE(tmpCaption.LoadString(IDS_CAPT_CreateLink));

			if (MessageBox(NULL, tmpStr, tmpCaption, MB_YESNO | MB_ICONQUESTION)==IDNO)
				return FALSE;
		}

		// Create link on desktop
		POSITION pos = e.children->GetHeadPosition();
		while(pos)
		{
			CNSEItem* item = (CNSEItem*)e.children->GetNext(pos);
			CString name;
			item->GetDisplayName(name);
			if (IS(item, CFolderItem))
				CreateShortcut(item, name, AS(item, CFolderItem)->data.Hint, AS(item, CFolderItem)->data.Icon);
		}

		return TRUE;
	}

	AfxMessageBox(e.menuItem->GetVerb());
	return TRUE;
}

int CFolderItem::CompareTo(CNSEItem* otherItem, CShellColumn& column)
{
	// This should never occur; if it does, however, folders come before files
	if (IS(otherItem, CFileItem))
		return -1;

	CFolderItem* dir2 = AS(otherItem, CFolderItem);

	CString str1;
	CString str2;
	switch (column.index)
	{
	case LFAttrFileName:
		str1 = data.DisplayName;
		str2 = dir2->data.DisplayName;
		break;
	case LFAttrStoreID:
		str1 = data.StoreID;
		str2 = dir2->data.StoreID;
		break;
	case LFAttrFileID:
		str1 = data.FileID;
		str2 = dir2->data.FileID;
		break;
	case LFAttrHint:
		str1 = data.Hint;
		str2 = dir2->data.Hint;
		break;
	}

	// Items with empty attribute values come last
	if ((str1=="") && (str2!=""))
		return 1;
	if ((str1!="") && (str2==""))
		return -1;

	// Compare desired attribute
	int ret = str1.CompareNoCase(str2);
	if (ret)
		return ret;

	// Compare file names
	ret = data.DisplayName.CompareNoCase(dir2->data.DisplayName);
	if (ret)
		return ret;

	// Compare store IDs
	ret = data.StoreID.Compare(dir2->data.StoreID);
	if (ret)
		return ret;

	// Compare file IDs
	return data.FileID.CompareNoCase(dir2->data.FileID);
}

BOOL CFolderItem::GetFileDescriptor(FILEDESCRIPTOR* fd)
{
	ZeroMemory(fd, sizeof(FILEDESCRIPTOR));

	fd->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	fd->dwFlags = FD_ATTRIBUTES | FD_PROGRESSUI;

	return TRUE;
}

BOOL CFolderItem::OnChangeName(CChangeNameEventArgs& e)
{
	// Stores sind die einzigen CFolderItem, die umbenannt werden können
	if (data.Level!=LevelStores)
		return FALSE;

	char key[LFKeySize];
	strcpy_s(key, LFKeySize, data.StoreID);
	USES_CONVERSION;
	LPWSTR name = T2W(e.newName);
	UINT res = LFSetStoreAttributes(&key[0], name, NULL);
	if (res!=LFOk)
	{
		wchar_t* tmpStr = LFGetErrorText(res);
		MessageBoxW(NULL, tmpStr, L"liquidFOLDERS", MB_OK | MB_ICONERROR);
		free(tmpStr);
	}
	else
	{
		UpdateItems(FALSE);
	}

	return (res==LFOk);
}

BOOL CFolderItem::OnDelete(CExecuteMenuitemsEventArgs& /*e*/)
{
	switch (data.Level)
	{
	case LevelRoot:

		CString caption;
		ENSURE(caption.LoadStringA(IDS_DeleteStore));

		CString msg;
		ENSURE(msg.LoadStringA(IDS_MustNotDeleteStore));

		::MessageBox(NULL, msg, caption, MB_ICONSTOP | MB_OK);
		break;
	}

	return FALSE;
}

void CFolderItem::CreateShortcut(CNSEItem* Item, const CString& LinkFilename, const CString& Description, UINT Icon)
{
	// Get the fully qualified file name for the link file
	TCHAR strPath[MAX_PATH];
	SHGetSpecialFolderPath(NULL, strPath, CSIDL_DESKTOPDIRECTORY, FALSE);
	CString PathLink;
	CString NumberStr = "";
	int Number = 1;

	// Check if link file exists; if not, append number
	do {
		PathLink = strPath;
		PathLink += _T("\\") + LinkFilename + NumberStr + _T(".lnk");
		NumberStr.Format(_T(" (%d)"), ++Number);
	} while (_access(PathLink, 0)==0);

	// Get a pointer to the IShellLink interface
	IShellLink* psl;
	HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (PVOID *)&psl);

	if (SUCCEEDED(hres))
	{
		// Never use icon for default store
		if (Icon==IDI_STORE_Default)
			Icon = IDI_STORE_Empty;

		psl->SetIDList(Item->GetPIDLAbsolute());
		psl->SetIconLocation(theApp.m_IconFile, Icon-1);
		psl->SetDescription((LPCSTR)Description);

		// Query IShellLink for the IPersistFile interface for saving the 
		// shortcut in persistent storage
		IPersistFile* ppf;
		hres = psl->QueryInterface(IID_IPersistFile, (PVOID*)&ppf);

		if (SUCCEEDED(hres))
		{
			// Ensure that the string is ANSI
			wchar_t wsz[MAX_PATH];
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)PathLink, -1, wsz, MAX_PATH);

			// Save the link by calling IPersistFile::Save
			hres = ppf->Save(wsz, TRUE);
			ppf->Release();
		}
		psl->Release();
	}
}

void CFolderItem::UpdateItems(BOOL add)
{
	CNSEFolder* f = GetRootFolder();
	if (add)
		f->RefreshView();
	f->NotifyUpdated();
	f->InternalRelease();
}









// The InitDataObject function is called to fill the data object with data representing 
// the child items.
void CFolderItem::InitDataObject(CInitDataObjectEventArgs& e)
{
	if (e.children->GetCount() <= 0)
		return;

	// Use streams to transfer namespace extension items.
	e.dataObject->SetHasFileData();

	//// Alternate way : Use the CF_HDROP data format

	//	CStringArray files;
	//
	//	POSITION pos = e.children->GetHeadPosition();
	//	while(pos)
	//	{
	//		CNSEItem* temp = (CNSEItem*)e.children->GetNext(pos);
	//		if(IS(temp,CFolderItem))
	//		{
	//			CFolderItem* folder = AS(temp,CFolderItem);
	//			files.Add(folder->fullPath);
	//		}
	//		else if(IS(temp,CFileItem))
	//		{
	//			CFileItem* file = AS(temp,CFileItem);
	//			files.Add(file->fullPath);
	//		}
	//	}
	//
	//	e.dataObject->SetHDROPData(&files);	 
}

void CFolderItem::OnExternalDrop(CNSEDragEventArgs& /*e*/)
{
/*	if(e.data->ShouldDeleteSource() && e.data->DidValidDrop())
	{
		// Delete every file/folder for which the delete command was executed.
		CPtrList* temp = e.data->GetChildren();
		POSITION pos = temp->GetHeadPosition();
		while(pos)
		{
			CNSEItem* item = (CNSEItem*)temp->GetNext(pos);
			if (IS(item,CFileItem))
			{
				CFileItem* c = AS(item,CFileItem);
				if(DeleteDirectory(c->fullPath))
				{
					// Remove from the Windows Explorer view
					c->Delete();
				}
			}
			else if (IS(item,CFolderItem))
			{
				CFolderItem* c = AS(item,CFolderItem);
				//if(DeleteDirectory(c->fullPath))
				{				
					// Remove from the Windows Explorer view
				//	c->Delete();
				}
			}
		}
	}*/
	this->RefreshView();

}

// Called when a drag-drop operation moves over the item.
void CFolderItem::DragOver(CNSEDragEventArgs& e)
{
	if ((e.keyState & MK_CONTROL) != 0 && (e.keyState & MK_SHIFT) != 0)
		e.effect = DROPEFFECT_LINK;
	else if ((e.keyState & MK_SHIFT) != 0)
		e.effect = DROPEFFECT_MOVE;
	else
		e.effect = DROPEFFECT_COPY;
}

// Called when a drag-drop operation moves over the item for the first time.
void CFolderItem::DragEnter(CNSEDragEventArgs& e)  
{
	if ((e.keyState & MK_CONTROL) != 0 && (e.keyState & MK_SHIFT) != 0)
		e.effect = DROPEFFECT_LINK;
	else if ((e.keyState & MK_SHIFT) != 0)
		e.effect = DROPEFFECT_MOVE;
	else
		e.effect = DROPEFFECT_COPY;
}

// Called when a drop occurs over the item.
void CFolderItem::DragDrop(CNSEDragEventArgs& /*e*/)
{
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

// Called when a non-folder item is double-clicked
BOOL CFolderItem::OnOpen(CExecuteMenuitemsEventArgs& /*e*/)
{
/*	if (e.children->GetCount() == 1)
	{
		POSITION pos = e.children->GetHeadPosition();
		CFileItem* temp = (CFileItem*)e.children->GetNext(pos);

		if (IS(temp,CFileItem))
			ShellExecute(e.hWnd,_T("open"),temp->fullPath,NULL,NULL,SW_SHOWNORMAL);
	}*/

	return TRUE;
}
