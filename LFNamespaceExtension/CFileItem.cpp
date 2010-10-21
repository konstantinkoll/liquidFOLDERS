
#include "stdafx.h"
#include "LFNamespaceExtension.h"
#include "CFolderItem.h"
#include "CFileItem.h"
#include "LFCore.h"
#include <shlwapi.h>


IMPLEMENT_DYNCREATE(CFileItem, CNSEItem)


// Class CFileItem
//

CFileItem::CFileItem()
{
}

CFileItem::CFileItem(LPCTSTR _StoreID, LFCoreAttributes* _Attrs)
{
	StoreID = _StoreID;
	Attrs = *_Attrs;
}

CFileItem::~CFileItem()
{
}

NSEItemAttributes CFileItem::GetAttributes(NSEItemAttributes requested)
{
	const UINT mask = NSEIA_FileSystem;

	return (NSEItemAttributes)(requested & mask);
}

void CFileItem::Serialize(CArchive& ar)
{
	ar << (BYTE)LFNamespaceExtensionVersion;
	ar << (BYTE)0;
	ar << StoreID;
	ar << (UINT)sizeof(LFCoreAttributes);
	ar.Write(&Attrs, sizeof(LFCoreAttributes));
}

void CFileItem::GetDisplayName(CString& displayName)
{
	displayName = Attrs.FileName;
}

void CFileItem::GetDisplayNameEx(CString& displayName, DisplayNameFlags flags)
{
	if ((flags & NSEDNF_InFolder)==0)
		if ((flags & NSEDNF_ForParsing)!=0)
		{
			char Path[MAX_PATH];
			UINT res = LFGetFileLocation((char*)(LPCSTR)StoreID, &Attrs, Path, MAX_PATH);
			if (res!=LFOk)
			{
				LFErrorBox(res);
			}
			else
			{
				displayName = Path;
			}
			return;
		}

	displayName = Attrs.FileName;
	if (((flags & NSEDNF_ForParsing)!=0) || (!theApp.HideFileExt()))
		if (Attrs.FileFormat[0]!='\0')
		{
			displayName += '.';
			displayName += Attrs.FileFormat;
		}
}

void CFileItem::GetIconFileAndIndex(CGetIconFileAndIndexEventArgs& e)
{
	char Path[MAX_PATH];
	UINT res = LFGetFileLocation((char*)(LPCSTR)StoreID, &Attrs, Path, MAX_PATH);
	if (res==LFOk)
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

void CFileItem::GetInfoTip(CString& infotip)
{
	infotip = Attrs.Comment;
}

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

BOOL CFileItem::GetColumnValueEx(VARIANT* value, CShellColumn& column)
{
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
		}
		else
		{
			return FALSE;
		}
		break;
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

BOOL CFileItem::IsValid()
{
	return TRUE;
}

int CFileItem::CompareTo(CNSEItem* otherItem, CShellColumn& column)
{
	if (IS(otherItem, CFolderItem))
		return 1;

	CFileItem* dir2 = AS(otherItem, CFileItem);

	CString str1;
	CString str2;
	int ret;

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

BOOL CFileItem::GetFileDescriptor(FILEDESCRIPTOR* fd)
{
	wchar_t* ptr = &Attrs.FileName[0];
	UINT wpos = 0;
	do
	{
		fd->cFileName[wpos++] = (*ptr<=0xFF ? (char)*ptr : '?');
	}
	while (*ptr++!=L'\0');

	fd->dwFlags = FD_PROGRESSUI | FD_WRITESTIME | FD_CREATETIME | FD_FILESIZE;

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








// The OnChangeName function is called when the item has been renamed in Windows Explorer.
// This function should return true of the renaming was successfully applied.
BOOL CFileItem::OnChangeName(CChangeNameEventArgs& /*e*/)
{
	/*BOOL ret = FALSE;
	
	try
	{
		TCHAR temp[MAX_PATH];
		_tcscpy(temp,fullPath);
		PathRemoveFileSpec(temp);
		CString parentFolder = temp;
		CString newName = PathCombineNSE(parentFolder, e.newName);
		MoveFile(fullPath, newName);
		fullPath = newName;
		name = e.newName; // store new name
		ret = TRUE; // success!
	}
	catch(...)
	{
		ret = FALSE; // failure
	}
	return ret;*/

	return FALSE;
}
