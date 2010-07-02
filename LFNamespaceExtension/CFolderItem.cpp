
#include "stdafx.h"
#include "LFNamespaceExtension.h"
#include "CFileItem.h"
#include "CFolderItem.h"
#include "LFCore.h"
#include "liquidFOLDERS.h"
#include "CCategoryCategorizer.h"
#include "CAttributeCategorizer.h"
#include <io.h>
#include <shlguid.h>


CString FrmtAttrStr(CString Mask, CString Name)
{
	if ((Mask[0]=='L') && (Name[0]>='A') && (Name[0]<='Z') && (Name[1]>'Z'))
		Name = Name.MakeLower().Mid(0,1)+Name.Mid(1, Name.GetLength()-1);

	CString tmpStr;
	tmpStr.Format(Mask.Mid(1, Mask.GetLength()-1), Name);
	return tmpStr;
}


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
//

CFolderItem::CFolderItem()
{
	data.Level = LevelRoot;
}

CFolderItem::CFolderItem(FolderSerialization &_data)
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

	nti->nseTarget = NSET_MyComputer;
	nti->name = _T("liquidFOLDERS");
	nti->infoTip.LoadString(IDS_MyComputerHint);
	nti->attributes = (NSEItemAttributes)(NSEIA_CFOLDERITEM | NSEIA_HasSubFolder);
	nti->iconFile = theApp.m_IconFile;
	nti->iconIndex = IDI_FLD_Default-1;

	info.AddTarget(nti);
}

NSEItemAttributes CFolderItem::GetAttributes(NSEItemAttributes /*requested*/)
{
	UINT ret = NSEIA_CFOLDERITEM;

	if (data.Level==LevelStores)
		ret |= NSEIA_CanRename | NSEIA_CanDelete;
	if (data.Level<LevelAttrValue)
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
	ar << data.Description;
	ar << data.Comment;
	ar << data.FileID;
	ar << data.StoreID;
	ar << data.DomainID;
	ar << data.Compare;
	ar.Write(&data.Value, sizeof(LFVariantData));
	ar << data.CreationTime.dwHighDateTime;
	ar << data.CreationTime.dwLowDateTime;
	ar << data.FileTime.dwHighDateTime;
	ar << data.FileTime.dwLowDateTime;
	ar << data.Size;
	ar << data.Format;
}

CNSEItem* CFolderItem::DeserializeChild(CArchive& ar)
{
	BYTE version;
	ar >> version;

	if (version!=CLFNamespaceExtensionVersion)
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
		FolderSerialization d ;
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
		ar >> d.Size;
		ar >> d.Format;

		return new CFolderItem(d);
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
		res = LFQuery(NULL);
		break;
	case LevelStores:
		f = LFAllocFilter();
		f->Mode = LFFilterModeStoreHome;
		f->HideEmptyDomains = true;
		strcpy_s(f->StoreID, LFKeySize, (LPCTSTR)data.StoreID);
		res = LFQuery(f);
		break;
	case LevelStoreHome:
		if (e.childrenType & NSECT_Folders)
		{
			CString sortStr;
			ENSURE(sortStr.LoadString(IDS_AttributeComment));

			FolderSerialization d = { 0 };
			d.Level = data.Level+2;
			d.Icon = IDI_FLD_All;
			d.Type = LFTypeVirtual;
			d.CategoryID = LFAttrCategoryCount;
			d.DisplayName.LoadString(IDS_AllFiles);
			d.Comment.LoadString(IDS_AllFilesComment);
			d.StoreID = data.StoreID;
			d.FileID = "ALL";
			d.DomainID = LFDomainAllFiles;
			d.Format.LoadString(IDS_Folder);

			e.children->AddTail(new CFolderItem(d));

			for (UINT a=0; a<LFAttributeCount; a++)
				if (theApp.m_Domains[data.DomainID]->ImportantAttributes->IsSet(a))
				{
					FolderSerialization d = { 0 };
					d.Level = data.Level+1;
					d.Icon = theApp.m_Attributes[a]->IconID;
					d.Type = LFTypeVirtual;
					d.CategoryID = theApp.m_Attributes[a]->Category;
					d.DisplayName = theApp.m_Attributes[a]->Name;
					d.Comment = FrmtAttrStr(sortStr, CString(theApp.m_Attributes[a]->Name));
					d.StoreID = data.StoreID;
					d.FileID.Format(_T("%d"), a);
					d.DomainID = data.DomainID;
					d.Compare = LFFilterCompareSubfolder;
					d.Format.LoadString(IDS_Folder);

					e.children->AddTail(new CFolderItem(d));
				}
		}
		break;
	case LevelAttribute:
		f = LFAllocFilter();
		f->Mode = LFFilterModeDirectoryTree;
		strcpy_s(f->StoreID, LFKeySize, (LPCTSTR)data.StoreID);
		f->DomainID = (unsigned char)data.DomainID;
		res = LFQuery(f);
		LFSortSearchResult(res, atoi(data.FileID), false);
		LFGroupSearchResult(res, f, atoi(data.FileID), data.Icon, atoi(data.FileID)!=LFAttrFileName);
		break;
	case LevelAttrValue:
		f = LFAllocFilter();
		f->Mode = LFFilterModeDirectoryTree;
		strcpy_s(f->StoreID, LFKeySize, (LPCTSTR)data.StoreID);
		f->DomainID = (unsigned char)data.DomainID;
		f->ConditionList = LFAllocFilterCondition();
		f->ConditionList->Next = NULL;
		f->ConditionList->Compare = data.Compare;
		f->ConditionList->AttrData = data.Value;
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
				(i->CategoryID!=LFCategoryHousekeeping))) && (e.childrenType & NSECT_Folders))
			{
				FolderSerialization d = { 0 };
				d.Level = data.Level+1;
				d.Icon = i->IconID;
				d.Type = i->Type;
				d.CategoryID = i->CategoryID;
				d.DisplayName = i->CoreAttributes.FileName;
				d.Description = i->Description;
				d.Comment = i->CoreAttributes.Comment;
				d.StoreID = i->StoreID;
				d.FileID = i->CoreAttributes.FileID;
				d.DomainID = data.DomainID;
				d.Size = i->CoreAttributes.FileSize;
				d.Format.LoadString(data.Level==LevelRoot ? IDS_Store : IDS_Folder);

				switch (data.Level)
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
				if ((data.Level==LevelAttribute) && (atoi(data.FileID)!=LFAttrFileName))
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
			UINT attr = atoi(data.FileID);

			FolderSerialization d = { 0 };
			d.Level = data.Level+1;
			d.Icon = IDI_FLD_Default;
			d.Type = LFTypeVirtual;
			d.CategoryID = LFAttrCategoryCount;
			d.StoreID = data.StoreID;
			d.FileID = "NULL";
			d.DomainID = data.DomainID;
			d.Size = NullSize;
			d.Compare = LFFilterCompareIsNull;
			d.Value.Attr = attr;
			d.Format.LoadString(IDS_Folder);

			CString tmpStr;
			tmpStr.LoadString(IDS_NULLFOLDER_NameMask);
			d.DisplayName = FrmtAttrStr(tmpStr, CString(theApp.m_Attributes[attr]->Name));
			tmpStr.LoadString(IDS_NULLFOLDER_CommentMask);
			d.Comment = FrmtAttrStr(tmpStr, CString(theApp.m_Attributes[attr]->Name));

			e.children->AddTail(new CFolderItem(d));
		}
	}

	if (f)
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
			StringFromGUID2(guid, buf, 39);
			CString id(buf);
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
	infotip = data.Description;
}

int CFolderItem::GetXPTaskPaneColumnIndices(UINT* indices)
{
	indices[0] = LFAttrFileName;
	indices[1] = LFAttrComment;
	indices[2] = LFAttrDescription;

	switch (data.Level)
	{
	case LevelStores:
		indices[2] = LFAttrCreationTime;
		indices[3] = LFAttrFileTime;
		indices[4] = LFAttrDescription;
		return 5;
	case LevelStoreHome:
	case LevelAttrValue:
		indices[3] = LFAttrFileSize;
		return (data.FileID=="ALL") ? 3 : 4;
	case LevelAttribute:
		return 2;
	}

	return 3;
}

int CFolderItem::GetTileViewColumnIndices(UINT* indices)
{
	indices[0] = LFAttrComment;
	indices[1] = LFAttrDescription;

	switch (data.Level)
	{
	case LevelStores:
		indices[1] = LFAttrFileTime;
		indices[2] = LFAttrDescription;
		return 3;
	case LevelAttrValue:
		indices[2] = LFAttrFileSize;
		return (data.FileID=="ALL") ? 2 : 3;
	}

	return 2;
}

int CFolderItem::GetPreviewDetailsColumnIndices(UINT* indices)
{
	indices[0] = LFAttrComment;
	indices[1] = LFAttrDescription;

	switch (data.Level)
	{
	case LevelStores:
		indices[1] = LFAttrCreationTime;
		indices[2] = LFAttrFileTime;
		indices[3] = LFAttrDescription;
		return 4;
	case LevelStoreHome:
	case LevelAttrValue:
		indices[2] = LFAttrFileSize;
		return (data.FileID=="ALL") ? 2 : 3;
	case LevelAttribute:
		return 1;
	}

	return 2;
}

int CFolderItem::GetContentViewColumnIndices(UINT* indices)
{
	return GetXPTaskPaneColumnIndices(indices);
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
		case LFDomainAllMediaFiles:
		case LFDomainFavorites:
			t = NSEFT_Search;
			break;
		}

	return t;
}

BOOL CFolderItem::GetColumn(CShellColumn& column, int index)
{
	if (index>((data.Level==LevelAttrValue) ? LFLastCoreAttribute : LFAttrFileSize))
		return FALSE;

	column.name = theApp.m_Attributes[index]->Name;
	column.width = theApp.m_Attributes[index]->RecommendedWidth/7;  // Chars, not pixel
	column.fmt = ((theApp.m_Attributes[index]->Type>=LFTypeUINT) || (index==LFAttrStoreID) || (index==LFAttrFileID)) ? NSESCF_Right : NSESCF_Left;
	column.categorizerType = NSECT_Alphabetical;
	column.index = index;
	column.defaultVisible = (index!=LFAttrStoreID) && (index!=LFAttrFileID);
	if (theApp.m_Attributes[index]->ShPropertyMapping.ID)
	{
		column.fmtid = theApp.m_Attributes[index]->ShPropertyMapping.Schema;
		column.pid = theApp.m_Attributes[index]->ShPropertyMapping.ID;
	}

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
		column.state = NSECS_PreferVarCmp;
		break;
	case LFAttrStoreID:
		column.categorizerType = NSECT_String;
		break;
	case LFAttrFileSize:
		column.categorizerType = NSECT_String;
		if ((data.Level==LevelRoot) || (data.Level==LevelStoreHome))
			column.state = NSECS_Hidden;
		break;
	case LFAttrFileID:
		if (data.Level<LevelAttrValue)
			column.state = NSECS_Hidden;
		break;
	case LFAttrComment:
		if (data.Level==LevelAttribute)
			column.state = NSECS_Hidden;
		break;
	case LFAttrDescription:
		if ((data.Level==LevelStoreHome) || (data.Level==LevelAttrValue))
			column.state = NSECS_Hidden;
		break;
	case LFAttrFileFormat:
		column.categorizerType = NSECT_String;
		if (data.Level==LevelAttrValue)
			column.state = NSECS_Hidden;
		break;
	case LFAttrCreationTime:
	case LFAttrFileTime:
		column.categorizerType = NSECT_String;
		if ((data.Level!=LevelRoot) && (data.Level!=LevelAttrValue))
			column.state = NSECS_Hidden;
		break;
	case LFAttrDeleteTime:
	case LFAttrFlags:
		column.categorizerType = NSECT_String;
		column.state = NSECS_Hidden;
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
	case LFAttrDescription:
		CUtils::SetVariantCString(value, data.Description);
		break;
	case LFAttrComment:
		CUtils::SetVariantCString(value, data.Comment);
		break;
	case LFAttrCreationTime:
		if ((data.CreationTime.dwHighDateTime) || (data.CreationTime.dwHighDateTime))
		{
			if (value->vt==VT_BSTR)
			{
				wchar_t tmpBuf[256];
				LFTimeToString(data.CreationTime, tmpBuf, 256);
				CString tmpStr(tmpBuf);
				CUtils::SetVariantCString(value, tmpStr);
			}
			else
			{
				CUtils::SetVariantFILETIME(value, data.CreationTime);
			}
		}
		else
		{
			return FALSE;
		}
		break;
	case LFAttrFileTime:
		if ((data.FileTime.dwHighDateTime) || (data.FileTime.dwHighDateTime))
		{
			if (value->vt==VT_BSTR)
			{
				wchar_t tmpBuf[256];
				LFTimeToString(data.FileTime, tmpBuf, 256);
				CString tmpStr(tmpBuf);
				CUtils::SetVariantCString(value, tmpStr);
			}
			else
			{
				CUtils::SetVariantFILETIME(value, data.FileTime);
			}
		}
		else
		{
			return FALSE;
		}
		break;
	case LFAttrFileFormat:
		CUtils::SetVariantCString(value, data.Format);
		break;
	case LFAttrFileSize:
		if (value->vt==VT_BSTR)
		{
			wchar_t tmpBuf[256];
			LFINT64ToString(data.Size, tmpBuf, 256);
			CString tmpStr(tmpBuf);
			CUtils::SetVariantCString(value, tmpStr);
		}
		else
		{
			CUtils::SetVariantINT64(value, data.Size);
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

	// All items can be opened
	if (e.children->GetCount()>=1)
	{
		ENSURE(tmpStr.LoadString(IDS_MENU_Open));
		ENSURE(tmpHint.LoadString(IDS_HINT_Open));
		e.menu->InsertItem(tmpStr, _T(VERB_OPEN), tmpHint, 0)->SetDefaultItem(TRUE);
	}

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
			e.menu->InsertItem(tmpStr, _T(VERB_MAKEDEFAULTSTORE), tmpHint, 2)->SetEnabled((f->data.Type & LFTypeStore) && (f->data.CategoryID==LFStoreModeInternal));

			ENSURE(tmpStr.LoadString(IDS_MENU_MakeHybridStore));
			ENSURE(tmpHint.LoadString(IDS_HINT_MakeHybridStore));
			e.menu->InsertItem(tmpStr, _T(VERB_MAKEHYBRIDSTORE), tmpHint, 3)->SetEnabled((f->data.Type & LFTypeStore) && (f->data.CategoryID==LFStoreModeExternal));
		}

		if ((e.menu->GetItemCount()<=4) && (e.children->GetCount()>=1))
		{
			e.menu->AddItem(_T(""))->SetSeparator(TRUE);

			ENSURE(tmpStr.LoadString(IDS_MENU_CreateLink));
			ENSURE(tmpHint.LoadString(IDS_HINT_CreateLink));
			e.menu->AddItem(tmpStr, _T(VERB_CREATELINK), tmpHint);

			ENSURE(tmpStr.LoadString(IDS_MENU_Delete));
			ENSURE(tmpHint.LoadString(IDS_HINT_Delete));
			e.menu->AddItem(tmpStr, _T(VERB_DELETE), tmpHint);

			if (e.children->GetCount()==1)
			{
				ENSURE(tmpStr.LoadString(IDS_MENU_Rename));
				ENSURE(tmpHint.LoadString(IDS_HINT_Rename));
				e.menu->AddItem(tmpStr, _T(VERB_RENAME), tmpHint);
			}
		}
		break;
	case LevelStores:
	case LevelStoreHome:
	case LevelAttribute:
		if ((e.menu->GetItemCount()<=4) && (e.children->GetCount()>=1))
		{
			e.menu->AddItem(_T(""))->SetSeparator(TRUE);

			ENSURE(tmpStr.LoadString(IDS_MENU_CreateLink));
			ENSURE(tmpHint.LoadString(IDS_HINT_CreateLink));
			e.menu->AddItem(tmpStr, _T(VERB_CREATELINK), tmpHint);
		}
		break;
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
			LFErrorBox(res);
		}
		else
		{
			UpdateItems();
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

			UINT res = (e.menuItem->GetVerb()==_T(VERB_MAKEDEFAULTSTORE)) ? LFMakeDefaultStore(&key[0]) : LFMakeHybridStore(&key[0]);
			if (res!=LFOk)
			{
				LFErrorBox(res);
			}
			else
			{
				UpdateItems();
			}

			return (res==LFOk);
		}

		return FALSE;
	}

	if (e.menuItem->GetVerb()==_T(VERB_CREATELINK))
	{
		// Ask if link should be created on desktop
		CString tmpStr;
		CString tmpCaption;

		ENSURE(tmpStr.LoadString(IDS_TEXT_CreateLink));
		ENSURE(tmpCaption.LoadString(IDS_CAPT_CreateLink));

		if (MessageBox(NULL, tmpStr, tmpCaption, MB_YESNO | MB_ICONQUESTION)==IDNO)
			return FALSE;

		// Create link on desktop
		POSITION pos = e.children->GetHeadPosition();
		while(pos)
		{
			CNSEItem* item = (CNSEItem*)e.children->GetNext(pos);
			CString name;
			item->GetDisplayName(name);
			if (IS(item, CFolderItem))
				CreateShortcut(item, name, AS(item, CFolderItem)->data.Description, AS(item, CFolderItem)->data.Icon);
		}

		return TRUE;
	}

	AfxMessageBox(e.menuItem->GetVerb());
	return TRUE;
}

int CFolderItem::CompareTo(CNSEItem* otherItem, CShellColumn& column)
{
	if (IS(otherItem, CFileItem))
		return -1;

	CFolderItem* dir2 = AS(otherItem, CFolderItem);

	CString str1;
	CString str2;
	int ret;

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
	case LFAttrComment:
		str1 = data.Comment;
		str2 = dir2->data.Comment;
		break;
	case LFAttrDescription:
		str1 = data.Description;
		str2 = dir2->data.Description;
		break;
	case LFAttrCreationTime:
		ret = CompareFileTime(&data.CreationTime, &dir2->data.CreationTime);
		goto GotRet;
	case LFAttrFileTime:
		ret = CompareFileTime(&data.FileTime, &dir2->data.FileTime);
		goto GotRet;
	case LFAttrFileSize:
		if (data.Size<dir2->data.Size)
			return -1;
		if (data.Size>dir2->data.Size)
			return 1;
		goto GotRet;
	}

	// Items with empty attribute values come last
	if ((str1=="") && (str2!=""))
		return 1;
	if ((str1!="") && (str2==""))
		return -1;

	// Compare desired attribute
	ret = str1.CompareNoCase(str2);
GotRet:
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
	_tcscpy(fd->cFileName, data.DisplayName);

	fd->dwFlags = FD_ATTRIBUTES | FD_PROGRESSUI;
	fd->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;

	switch (data.Level)
	{
	case LevelRoot:
		fd->dwFlags |= FD_WRITESTIME | FD_CREATETIME;
		fd->ftCreationTime = data.CreationTime;
		fd->ftLastWriteTime = data.FileTime;
		break;
	case LevelStores:
		LARGE_INTEGER sz;
		sz.QuadPart = data.Size;

		fd->dwFlags |= FD_FILESIZE;
		fd->nFileSizeHigh = sz.HighPart;
		fd->nFileSizeLow = sz.LowPart;
		break;
	}

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
		LFErrorBox(res);
	}
	else
	{
		UpdateItems();
	}

	return (res==LFOk);
}

BOOL CFolderItem::OnDelete(CExecuteMenuitemsEventArgs& /*e*/)
{
	switch (data.Level)
	{
	case LevelRoot:

		CString caption;
		CString msg;
		ENSURE(caption.LoadString(IDS_CAPT_DeleteStore));
		ENSURE(msg.LoadString(IDS_TEXT_DeleteStore));

		MessageBox(NULL, msg, caption, MB_ICONSTOP | MB_OK);
		break;
	}

	return FALSE;
}

BOOL CFolderItem::OnOpen(CExecuteMenuitemsEventArgs& e)
{
	if (e.children->GetCount()==1)
	{
		POSITION pos = e.children->GetHeadPosition();
		CNSEItem* item = (CFileItem*)e.children->GetNext(pos);

		if (IS(item, CFolderItem))
			BrowseToChild((CFolderItem*)item);

		if (IS(item, CFileItem))
		{
			char Path[MAX_PATH];
			UINT res = LFGetFileLocation((char*)(LPCSTR)data.StoreID, &((CFileItem*)item)->Attrs, Path, MAX_PATH);
			if (res!=LFOk)
			{
				LFErrorBox(res);
			}
			else
				if (ShellExecuteA(e.hWnd, "open", Path, "", "", SW_SHOW)==(HINSTANCE)SE_ERR_NOASSOC)
				{
					char Cmd[300];
					strcpy_s(Cmd, 300, "shell32.dll,OpenAs_RunDLL ");
					strcat_s(Cmd, 300, Path);
					ShellExecuteA(e.hWnd, "open", "rundll32.exe", Cmd, Path, SW_SHOW);
				}

			return TRUE;
		}
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
	do
	{
		PathLink = strPath;
		PathLink += _T("\\") + LinkFilename + NumberStr + _T(".lnk");
		NumberStr.Format(_T(" (%d)"), ++Number);
	}
	while (_access(PathLink, 0)==0);

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

void CFolderItem::UpdateItems()
{
	NotifyUpdated();

	CNSEFolder* parent = GetParentFolder();
	if (parent)
	{
		parent->NotifyUpdated();
		parent->RefreshView();
		parent->InternalRelease();
	}
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
