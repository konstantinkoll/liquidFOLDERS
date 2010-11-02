
// CFileItem.cpp: Implementierung der Klasse CFileItem
//

#include "stdafx.h"
#include "LFNamespaceExtension.h"
#include "CFolderItem.h"
#include "CFileItem.h"
#include "LFCore.h"
#include <shlwapi.h>


IMPLEMENT_DYNCREATE(CFileItem, CNSEItem)


// CFileItem
//

// IPersist

CFileItem::CFileItem()
{
	Item = LFAllocItemDescriptor();
}

CFileItem::CFileItem(char* _StoreID, LFCoreAttributes* Attrs)
{
	Item = LFAllocItemDescriptor(Attrs);
	strcpy_s(Item->StoreID, LFKeySize, _StoreID);
}

CFileItem::CFileItem(LFItemDescriptor* _Item)
{
	Item = _Item;
	Item->RefCount++;
}

CFileItem::~CFileItem()
{
	if (Item)
		LFFreeItemDescriptor(Item);
}


// PIDL handling

void CFileItem::Serialize(CArchive& ar)
{
	ar << (BYTE)LFNamespaceExtensionVersion;
	ar << (BYTE)0;
	ar.Write(&Item->StoreID, sizeof(Item->StoreID));
	ar << (UINT)sizeof(LFCoreAttributes);
	ar.Write(&Item->CoreAttributes, sizeof(LFCoreAttributes));

	LFVariantData v[LFAttributeCount];
	UINT Count = 0;
	for (UINT a=LFLastCoreAttribute+1; a<LFAttributeCount; a++)
	{
		v[a].Attr = a;
		v[a].Type = theApp.m_Attributes[a]->Type;
		LFGetAttributeVariantData(Item, &v[a]);

		if (!LFIsNullVariantData(&v[a]))
			Count++;
	}

	ar << Count;
	for (UINT a=LFLastCoreAttribute+1; a<LFAttributeCount; a++)
		if (!LFIsNullVariantData(&v[a]))
			ar.Write(&v[a], sizeof(LFVariantData));
}


// IEnumIDList

BOOL CFileItem::IsValid()
{
	return AS(GetParentFolder(), CFolderItem)->IsValid();
}


// IMoniker

void CFileItem::GetDisplayName(CString& displayName)
{
	displayName = Item->CoreAttributes.FileName;
}

void CFileItem::GetDisplayNameEx(CString& displayName, DisplayNameFlags flags)
{
	if ((flags & (NSEDNF_InFolder | NSEDNF_ForParsing))==NSEDNF_ForParsing)
	{
		char Path[MAX_PATH];
		displayName = (LFGetFileLocation(Item->StoreID, &Item->CoreAttributes, Path, MAX_PATH)==LFOk) ? Path : "?";
		return;
	}

	displayName = Item->CoreAttributes.FileName;
	if ((!(flags & NSEDNF_ForEditing)) && (Item->CoreAttributes.FileFormat[0]!='\0') && ((flags & NSEDNF_ForParsing) || (!(flags & NSEDNF_InFolder)) || (!theApp.HideFileExt())))
	{
		displayName += '.';
		displayName += Item->CoreAttributes.FileFormat;
	}
}


// IExtractIcon

void CFileItem::GetIconFileAndIndex(CGetIconFileAndIndexEventArgs& e)
{
	char Path[MAX_PATH];
	if (LFGetFileLocation(Item->StoreID, &Item->CoreAttributes, Path, MAX_PATH)==LFOk)
	{
		e.iconExtractMode = NSEIEM_SystemImageListIndexFromPath;
		e.iconFile = Path;
	}
	else
	{
		e.iconExtractMode = NSEIEM_IconFileAndIndex;
		e.iconFile = theApp.m_CoreFile;
		e.iconIndex = IDI_FILE_Generic-1;
	}
}


// IQueryInfo

void CFileItem::GetInfoTip(CString& infotip)
{
	infotip = Item->CoreAttributes.Comment;
}


// IShellFolder2

BOOL CFileItem::GetColumnValueEx(VARIANT* value, CShellColumn& column)
{
	if (column.fmtid==FMTID_ShellDetails)
		switch (column.pid)
		{
		case 2:
			SAFEARRAYBOUND rgsabound;
			rgsabound.cElements = sizeof(SHDESCRIPTIONID);
			rgsabound.lLbound = 0;

			value->parray = SafeArrayCreate(VT_UI1, 1, &rgsabound);
			((SHDESCRIPTIONID*)value->parray->pvData)->dwDescriptionId = SHDID_FS_FILE;
			value->vt = VT_ARRAY | VT_UI1;
			return TRUE;
		case 9:
			CUtils::SetVariantINT(value, 0);
			return TRUE;
		case 11:
			if (Item->CoreAttributes.FileFormat[0]!='\0')
			{
				CString tmpStr(Item->CoreAttributes.FileFormat);
				tmpStr.Insert(0, _T("."));
				CUtils::SetVariantCString(value, tmpStr);
				return TRUE;
			}
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

	if (column.index==LFAttrFileFormat)
	{
		if (Item->CoreAttributes.FileFormat[0]!='\0')
		{
			CString tmpStr(Item->CoreAttributes.FileFormat);
			tmpStr.Insert(0, _T("."));

			SHFILEINFO sfi;
			if (SUCCEEDED(SHGetFileInfo(tmpStr, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES)))
			{
				CString tmpStr = sfi.szTypeName;
				CUtils::SetVariantCString(value, tmpStr);
				return TRUE;
			}
		}

		return FALSE;
	}

	if ((column.index>=0) && (column.index<LFAttributeCount))
	{
		UCHAR Type = theApp.m_Attributes[column.index]->Type;
		if ((value->vt!=VT_BSTR) && ((Type==LFTypeRating) || (Type==LFTypeUINT) || (Type==LFTypeINT64) || (Type==LFTypeDouble) || (Type==LFTypeTime)))
		{
			LFVariantData v;
			v.Attr = column.index;
			v.Type = Type;
			LFGetAttributeVariantData(Item, &v);

			if (!LFIsNullVariantData(&v))
			{
				CString tmpStr;
				UINT tmpInt;

				switch (Type)
				{
				case LFTypeRating:
					tmpInt = Item->CoreAttributes.Rating*10;
					CUtils::SetVariantUINT(value, tmpInt>99 ? 99 : tmpInt);
					return TRUE;
				case LFTypeUINT:
					CUtils::SetVariantUINT(value, v.UINT);
					return TRUE;
				case LFTypeINT64:
					CUtils::SetVariantUINT64(value, v.INT64);
					return TRUE;
				case LFTypeDouble:
					CUtils::SetVariantDOUBLE(value, v.Double);
					return TRUE;
				case LFTypeTime:
					CUtils::SetVariantFILETIME(value, v.Time);
					return TRUE;
				}
			}
		}
		else
		{
			wchar_t tmpBuf[256];

			switch (column.index)
			{
			case LFAttrRating:
				CUtils::SetVariantCString(value, theApp.m_Categories[0][Item->CoreAttributes.Rating/2]);
				break;
			case LFAttrPriority:
				CUtils::SetVariantCString(value, theApp.m_Categories[1][Item->CoreAttributes.Priority/2]);
				break;
			default:
				LFAttributeToString(Item, column.index, tmpBuf, 256);
				CString tmpStr(tmpBuf);
				CUtils::SetVariantCString(value, tmpStr);
			}

			return TRUE;
		}
	}

	return FALSE;
}


// IShellFolder

BOOL CFileItem::OnChangeName(CChangeNameEventArgs& e)
{
	UINT res = LFTransactionRename(Item->StoreID, Item->CoreAttributes.FileID, e.newName.GetBuffer());
	if (res==LFOk)
	{
		wcscpy_s(Item->CoreAttributes.FileName, 256, e.newName);
	}
	else
	{
		LFErrorBox(res);
	}

	return (res==LFOk);
}


// IShellItem

NSEItemAttributes CFileItem::GetAttributes(NSEItemAttributes requested)
{
	return (NSEItemAttributes)(requested & NSEIA_FileSystem | NSEIA_CanRename | NSEIA_CanDelete | NSEIA_CanLink);
}

int CFileItem::CompareTo(CNSEItem* otherItem, CShellColumn& column)
{
	if (IS(otherItem, CFolderItem))
		return 1;

	CFileItem* file2 = AS(otherItem, CFileItem);

	CString str1;
	CString str2;
	int ret = 0;

	switch (column.index)
	{
	case LFAttrFileName:
		str1 = Item->CoreAttributes.FileName;
		str2 = file2->Item->CoreAttributes.FileName;
		break;
	case LFAttrStoreID:
		str1 = Item->StoreID;
		str2 = file2->Item->StoreID;
		break;
	case LFAttrFileID:
		str1 = Item->CoreAttributes.FileID;
		str2 = file2->Item->CoreAttributes.FileID;
		break;
	case LFAttrComment:
		str1 = Item->CoreAttributes.Comment;
		str2 = file2->Item->CoreAttributes.Comment;
		break;
	case LFAttrCreationTime:
		ret = CompareFileTime(&Item->CoreAttributes.CreationTime, &file2->Item->CoreAttributes.CreationTime);
		goto GotRet;
	case LFAttrFileTime:
		ret = CompareFileTime(&Item->CoreAttributes.FileTime, &file2->Item->CoreAttributes.FileTime);
		goto GotRet;
	case LFAttrFileSize:
		if (Item->CoreAttributes.FileSize<file2->Item->CoreAttributes.FileSize)
			return -1;
		if (Item->CoreAttributes.FileSize>file2->Item->CoreAttributes.FileSize)
			return 1;
		goto GotRet;
	case LFAttrURL:
		str1 = Item->CoreAttributes.URL;
		str2 = file2->Item->CoreAttributes.URL;
		break;
	case LFAttrRating:
		if (Item->CoreAttributes.Rating<file2->Item->CoreAttributes.Rating)
			return -1;
		if (Item->CoreAttributes.Rating>file2->Item->CoreAttributes.Rating)
			return 1;
		goto GotRet;
	case LFAttrPriority:
		if (Item->CoreAttributes.Priority<file2->Item->CoreAttributes.Priority)
			return -1;
		if (Item->CoreAttributes.Priority>file2->Item->CoreAttributes.Priority)
			return 1;
		goto GotRet;
	case LFAttrLocationName:
		str1 = Item->CoreAttributes.LocationName;
		str2 = file2->Item->CoreAttributes.LocationName;
		break;
	case LFAttrLocationIATA:
		str1 = Item->CoreAttributes.LocationIATA;
		str2 = file2->Item->CoreAttributes.LocationIATA;
		break;
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
	ret = wcscmp(Item->CoreAttributes.FileName, file2->Item->CoreAttributes.FileName);
	if (ret)
		return ret;

	// Compare store IDs
	ret = strcmp(Item->StoreID, file2->Item->StoreID);
	if (ret)
		return ret;

	// Compare file IDs
	return strcmp(Item->CoreAttributes.FileID, file2->Item->CoreAttributes.FileID);
}


// IDropSource

BOOL CFileItem::GetFileDescriptor(FILEDESCRIPTOR* fd)
{
	CString Path;
	GetDisplayNameEx(Path, (DisplayNameFlags)(NSEDNF_InFolder | NSEDNF_ForParsing));
	wcscpy_s(fd->cFileName, MAX_PATH, Path);

	fd->dwFlags = FD_WRITESTIME | FD_CREATETIME | FD_FILESIZE;

	fd->ftCreationTime = Item->CoreAttributes.CreationTime;
	fd->ftLastWriteTime = Item->CoreAttributes.FileTime;

	LARGE_INTEGER sz;
	sz.QuadPart = Item->CoreAttributes.FileSize;
	fd->nFileSizeHigh = sz.HighPart;
	fd->nFileSizeLow = sz.LowPart;

	return TRUE;
}

LPSTREAM CFileItem::GetStream()
{
	LPSTREAM ret = NULL;

	char Path[MAX_PATH];
	UINT res = LFGetFileLocation(Item->StoreID, &Item->CoreAttributes, Path, MAX_PATH);
	if (res!=LFOk)
	{
		LFErrorBox(res);
	}
	else
	{
		SHCreateStreamOnFileA(Path, STGM_READ, &ret);
	}

	return ret;
}


// Exposed property handlers

int CFileItem::GetXPTaskPaneColumnIndices(UINT* indices)
{
	indices[0] = LFAttrFileName;
	indices[1] = LFAttrComment;
	indices[2] = LFAttrCreationTime;
	indices[3] = LFAttrFileTime;
	indices[4] = LFAttrFileSize;

	return 5;
}

int CFileItem::GetTileViewColumnIndices(UINT* indices)
{
	indices[0] = LFAttrComment;
	indices[1] = LFAttrFileTime;
	indices[2] = LFAttrFileSize;

	return 3;
}

int CFileItem::GetPreviewDetailsColumnIndices(UINT* indices)
{
	indices[0] = LFAttrArtist;
	indices[1] = LFAttrTitle;
	indices[2] = LFAttrRecordingTime;
	indices[3] = LFAttrDuration;
	indices[4] = LFAttrTags;
	indices[5] = LFAttrPages;
	indices[6] = LFAttrRating;
	indices[7] = LFAttrLanguage;
	indices[8] = LFAttrWidth;
	indices[9] = LFAttrHeight;
	indices[10] = LFAttrFileSize;
	indices[11] = LFAttrComment;
	indices[12] = LFAttrRecordingEquipment;
	indices[13] = LFAttrBitrate;
	indices[14] = LFAttrCreationTime;
	indices[15] = LFAttrFileTime;

	return 16;
}

int CFileItem::GetContentViewColumnIndices(UINT* indices)
{
	return GetXPTaskPaneColumnIndices(indices);
}


// Other

BOOL CFileItem::SetShellLink(IShellLink* psl)
{
	char Path[MAX_PATH];
	if (LFGetFileLocation(Item->StoreID, &Item->CoreAttributes, Path, MAX_PATH)==LFOk)
	{
		wchar_t tmpBuf[MAX_PATH];
		MultiByteToWideChar(CP_ACP, 0, Path, (int)(strlen(Path)+1), tmpBuf, MAX_PATH);

		psl->SetPath(tmpBuf);
		psl->SetIconLocation(tmpBuf, 0);
		psl->SetShowCmd(SW_SHOWNORMAL);

		return TRUE;
	}

	return FALSE;
}
