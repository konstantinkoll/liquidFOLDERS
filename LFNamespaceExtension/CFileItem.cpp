
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
}

CFileItem::CFileItem(LPCTSTR _StoreID, LFCoreAttributes* _Attrs)
{
	StoreID = _StoreID;
	Attrs = *_Attrs;
}


// PIDL handling

void CFileItem::Serialize(CArchive& ar)
{
	ar << (BYTE)LFNamespaceExtensionVersion;
	ar << (BYTE)0;
	ar << StoreID;
	ar << (UINT)sizeof(LFCoreAttributes);
	ar.Write(&Attrs, sizeof(LFCoreAttributes));
}


// IEnumIDList

BOOL CFileItem::IsValid()
{
	return AS(GetParentFolder(), CFolderItem)->IsValid();
}


// IMoniker

void CFileItem::GetDisplayName(CString& displayName)
{
	displayName = Attrs.FileName;
}

void CFileItem::GetDisplayNameEx(CString& displayName, DisplayNameFlags flags)
{
	if ((flags & (NSEDNF_InFolder | NSEDNF_ForParsing))==NSEDNF_ForParsing)
	{
		char Path[MAX_PATH];
		displayName = (LFGetFileLocation((char*)(LPCSTR)StoreID, &Attrs, Path, MAX_PATH)==LFOk) ? Path : _T("?");
		return;
	}

	displayName = Attrs.FileName;
	if ((!(flags & NSEDNF_ForEditing)) && (Attrs.FileFormat[0]!='\0') && ((flags & NSEDNF_ForParsing) || (!(flags & NSEDNF_InFolder)) || (!theApp.HideFileExt())))
	{
		displayName += '.';
		displayName += Attrs.FileFormat;
	}
}


// IExtractIcon

void CFileItem::GetIconFileAndIndex(CGetIconFileAndIndexEventArgs& e)
{
	char Path[MAX_PATH];
	if (LFGetFileLocation((char*)(LPCSTR)StoreID, &Attrs, Path, MAX_PATH)==LFOk)
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
	infotip = Attrs.Comment;
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
			if (Attrs.FileFormat[0]!='\0')
			{
				char Extension[256];
				strcpy_s(Extension, 256, ".");
				strcat_s(Extension, Attrs.FileFormat);
				CUtils::SetVariantLPCTSTR(value, Extension);
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
			CUtils::SetVariantLPCTSTR(value, "prop:~System.ItemNameDisplay;~System.ItemTypeText");
			return TRUE;
		default:
			return FALSE;
		}

	CString tmpStr;
	UINT tmpInt;
	wchar_t tmpBuf[256];

	switch (column.index)
	{
	case LFAttrFileName:
		tmpStr = Attrs.FileName;
		break;
	case LFAttrFileID:
		tmpStr = Attrs.FileID;
		break;
	case LFAttrStoreID:
		tmpStr = StoreID;
		break;
	case LFAttrComment:
		tmpStr = Attrs.Comment;
		break;
	case LFAttrCreationTime:
		if ((Attrs.CreationTime.dwHighDateTime) || (Attrs.CreationTime.dwLowDateTime))
		{
			if (value->vt==VT_BSTR)
			{
				LFTimeToString(Attrs.CreationTime, tmpBuf, 256);
				tmpStr = tmpBuf;
			}
			else
			{
				CUtils::SetVariantFILETIME(value, Attrs.CreationTime);
				return TRUE;
			}
		}
		else
		{
			return FALSE;
		}
		break;
	case LFAttrAddTime:
		if ((Attrs.AddTime.dwHighDateTime) || (Attrs.AddTime.dwLowDateTime))
		{
			if (value->vt==VT_BSTR)
			{
				LFTimeToString(Attrs.AddTime, tmpBuf, 256);
				tmpStr = tmpBuf;
			}
			else
			{
				CUtils::SetVariantFILETIME(value, Attrs.AddTime);
				return TRUE;
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
				LFTimeToString(Attrs.FileTime, tmpBuf, 256);
				tmpStr = tmpBuf;
			}
			else
			{
				CUtils::SetVariantFILETIME(value, Attrs.FileTime);
				return TRUE;
			}
		}
		else
		{
			return FALSE;
		}
		break;
	case LFAttrArchiveTime:
		if ((Attrs.ArchiveTime.dwHighDateTime) || (Attrs.ArchiveTime.dwLowDateTime))
		{
			if (value->vt==VT_BSTR)
			{
				LFTimeToString(Attrs.ArchiveTime, tmpBuf, 256);
				tmpStr = tmpBuf;
			}
			else
			{
				CUtils::SetVariantFILETIME(value, Attrs.ArchiveTime);
				return TRUE;
			}
		}
		else
		{
			return FALSE;
		}
		break;
	case LFAttrFileFormat:
		if (Attrs.FileFormat[0]!='\0')
		{
			tmpStr = _T(".");
			tmpStr.Append(Attrs.FileFormat);

			SHFILEINFO sfi;
			if (SUCCEEDED(SHGetFileInfo(tmpStr, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES)))
			{
				tmpStr = sfi.szTypeName;
				CUtils::SetVariantCString(value, tmpStr);
				break;
			}
		}
		return FALSE;
	case LFAttrFileSize:
		if (value->vt==VT_BSTR)
		{
			LFINT64ToString(Attrs.FileSize, tmpBuf, 256);
			tmpStr = tmpBuf;
		}
		else
		{
			CUtils::SetVariantINT64(value, Attrs.FileSize);
			return TRUE;
		}
		break;
	case LFAttrURL:
		tmpStr = Attrs.URL;
		break;
	case LFAttrTags:
		tmpStr = Attrs.Tags;
		break;
	case LFAttrRating:
		if (value->vt==VT_BSTR)
		{
			if (Attrs.Rating>1)
				tmpStr.Format(_T("%d"), Attrs.Rating/2);
		}
		else
		{
			tmpInt = Attrs.Rating*10;
			if (tmpInt>99)
				tmpInt = 99;
			CUtils::SetVariantUINT(value, tmpInt);
			return TRUE;
		}
		break;
	case LFAttrPriority:
		if (Attrs.Priority>1)
			tmpStr.Format(_T("%d"), Attrs.Priority/2);
		break;
	case LFAttrLocationName:
		tmpStr = Attrs.LocationName;
		break;
	case LFAttrLocationIATA:
		tmpStr = Attrs.LocationIATA;
		break;
	case LFAttrLocationGPS:
		LFGeoCoordinatesToString(Attrs.LocationGPS, tmpBuf, 256, false);
		tmpStr = tmpBuf;
		break;
	default:
		return FALSE;
	}

	CUtils::SetVariantCString(value, tmpStr);
	return TRUE;
}


// IShellFolder

BOOL CFileItem::OnChangeName(CChangeNameEventArgs& e)
{
	char StoreID[LFKeySize];
	char FileID[LFKeySize];
	strcpy_s(StoreID, LFKeySize, this->StoreID);
	strcpy_s(FileID, LFKeySize, Attrs.FileID);

	USES_CONVERSION;
	LPWSTR name = T2W(e.newName);

	UINT res = LFTransactionRename(StoreID, FileID, name);
	if (res==LFOk)
	{
		wcscpy_s(Attrs.FileName, 256, name);
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

	CFileItem* dir2 = AS(otherItem, CFileItem);

	CString str1;
	CString str2;
	int ret = 0;

	switch (column.index)
	{
	case LFAttrFileName:
		str1 = Attrs.FileName;
		str2 = dir2->Attrs.FileName;
		break;
	case LFAttrStoreID:
		str1 = StoreID;
		str2 = dir2->StoreID;
		break;
	case LFAttrFileID:
		str1 = Attrs.FileID;
		str2 = dir2->Attrs.FileID;
		break;
	case LFAttrComment:
		str1 = Attrs.Comment;
		str2 = dir2->Attrs.Comment;
		break;
	case LFAttrCreationTime:
		ret = CompareFileTime(&Attrs.CreationTime, &dir2->Attrs.CreationTime);
		goto GotRet;
	case LFAttrFileTime:
		ret = CompareFileTime(&Attrs.FileTime, &dir2->Attrs.FileTime);
		goto GotRet;
	case LFAttrFileSize:
		if (Attrs.FileSize<dir2->Attrs.FileSize)
			return -1;
		if (Attrs.FileSize>dir2->Attrs.FileSize)
			return 1;
		goto GotRet;
	case LFAttrURL:
		str1 = Attrs.URL;
		str2 = dir2->Attrs.URL;
		break;
	case LFAttrRating:
		if (Attrs.Rating<dir2->Attrs.Rating)
			return -1;
		if (Attrs.Rating>dir2->Attrs.Rating)
			return 1;
		goto GotRet;
	case LFAttrPriority:
		if (Attrs.Priority<dir2->Attrs.Priority)
			return -1;
		if (Attrs.Priority>dir2->Attrs.Priority)
			return 1;
		goto GotRet;
	case LFAttrLocationName:
		str1 = Attrs.LocationName;
		str2 = dir2->Attrs.LocationName;
		break;
	case LFAttrLocationIATA:
		str1 = Attrs.LocationIATA;
		str2 = dir2->Attrs.LocationIATA;
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
	ret = wcscmp(Attrs.FileName, dir2->Attrs.FileName);
	if (ret)
		return ret;

	// Compare store IDs
	ret = StoreID.Compare(dir2->StoreID);
	if (ret)
		return ret;

	// Compare file IDs
	return strcmp(Attrs.FileID, dir2->Attrs.FileID);
}


// IDropSource

BOOL CFileItem::GetFileDescriptor(FILEDESCRIPTOR* fd)
{
	CString Path;
	GetDisplayNameEx(Path, (DisplayNameFlags)(NSEDNF_InFolder | NSEDNF_ForParsing));
	strcpy_s(fd->cFileName, MAX_PATH, Path);

	fd->dwFlags = FD_WRITESTIME | FD_CREATETIME | FD_FILESIZE;

	fd->ftCreationTime = Attrs.CreationTime;
	fd->ftLastWriteTime = Attrs.FileTime;

	LARGE_INTEGER sz;
	sz.QuadPart = Attrs.FileSize;
	fd->nFileSizeHigh = sz.HighPart;
	fd->nFileSizeLow = sz.LowPart;

	return TRUE;
}

LPSTREAM CFileItem::GetStream()
{
	LPSTREAM ret = NULL;

	char Path[MAX_PATH];
	UINT res = LFGetFileLocation((char*)(LPCSTR)StoreID, &Attrs, Path, MAX_PATH);
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
	indices[0] = LFAttrComment;
	indices[1] = LFAttrCreationTime;
	indices[2] = LFAttrFileTime;
	indices[3] = LFAttrFileSize;
	indices[4] = LFAttrURL;
	indices[5] = LFAttrTags;
	indices[6] = LFAttrRating;

	return 7;
}

int CFileItem::GetContentViewColumnIndices(UINT* indices)
{
	return GetXPTaskPaneColumnIndices(indices);
}


// Other

BOOL CFileItem::SetShellLink(IShellLink* psl)
{
	char Path[MAX_PATH];
	if (LFGetFileLocation((char*)(LPCSTR)StoreID, &Attrs, Path, MAX_PATH)==LFOk)
	{
		psl->SetPath(Path);
		psl->SetIconLocation(Path, 0);
		psl->SetShowCmd(SW_SHOWNORMAL);
		return TRUE;
	}

	return FALSE;
}
