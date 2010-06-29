
#include "stdafx.h"
#include "LFNamespaceExtension.h"
#include "CFolderItem.h"
#include "CFileItem.h"
#include "LFCore.h"
#include "helper.h"


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

void CFileItem::GetDisplayName(CString& displayName)
{
	displayName = Attrs.FileName;
}

void CFileItem::GetDisplayNameEx(CString& displayName, DisplayNameFlags flags)
{
	if ((flags & NSEDNF_InFolder)==0)
		if ((flags & NSEDNF_ForParsing)!=0)
		{
			displayName = StoreID;
			displayName += _T("\\");
			displayName += Attrs.FileID;
			return;
		}

	CNSEItem::GetDisplayNameEx(displayName, flags);
}

void CFileItem::Serialize(CArchive& ar)
{
	ar << (BYTE)CLFNamespaceExtensionVersion;
	ar << (BYTE)0;
	ar << StoreID;
	ar << (UINT)sizeof(LFCoreAttributes);
	ar.Write(&Attrs, sizeof(LFCoreAttributes));
}







// The CompareTo function is called to perform a comparison of the item with the
// specified item with respect to the specified column. Return a number less 
// than zero if this item should come before otherItem, greater than zero if 
// this item should come after otherItem, and zero if the two items are equal.
int CFileItem::CompareTo(CNSEItem* otherItem, CShellColumn& column)
{
	// Folders should come before files
	if (IS(otherItem, CFolderItem))
		return 1;

	return 1;
	/*
	CFileItem* file2 = AS(otherItem,CFileItem);
	
	if (column.index == 0) // name
	{
		return name.CompareNoCase(file2->name);
	}
	
	if (column.index == 1) // size
	{
		WIN32_FILE_ATTRIBUTE_DATA fd1,fd2;
		GetFileAttributesEx(fullPath,GetFileExInfoStandard,&fd1);
		GetFileAttributesEx(file2->fullPath,GetFileExInfoStandard,&fd2);

		if(fd1.nFileSizeLow==fd2.nFileSizeLow)
			return 0;

		if(fd1.nFileSizeLow<fd2.nFileSizeLow)
			return -1;
		
		return 1;
	}
	
	if (column.index == 2) // type
	{
		CString type1 = CUtils::GetFileType(fullPath);
		CString type2 = CUtils::GetFileType(file2->fullPath);
		
		return type1.CompareNoCase(type2); 
		
	}
	
	if (column.index == 3) // date modified
	{
		WIN32_FILE_ATTRIBUTE_DATA fd1,fd2;
		GetFileAttributesEx(fullPath,GetFileExInfoStandard,&fd1);
		GetFileAttributesEx(file2->fullPath,GetFileExInfoStandard,&fd2);

		return CompareFileTime(&fd1.ftLastWriteTime,&fd2.ftLastWriteTime);
	}
	
	return 0;
*/
}

LPSTREAM CFileItem::GetStream()
{
	LPSTREAM ret = NULL;
	//SHCreateStreamOnFile(fullPath,STGM_READ,&ret);
	return ret;
}

// The GetFileDescriptor function is called to retrieve file-related information
// about the item.
BOOL CFileItem::GetFileDescriptor(FILEDESCRIPTOR* fd)
{
	ZeroMemory(fd,sizeof(FILEDESCRIPTOR));
/*	
	WIN32_FILE_ATTRIBUTE_DATA fad;
	GetFileAttributesEx(fullPath,GetFileExInfoStandard,&fad); 
	
	_tcscpy(fd->cFileName,name);
	fd->dwFileAttributes = fad.dwFileAttributes;
	fd->ftCreationTime = fad.ftCreationTime;
	fd->ftLastAccessTime = fad.ftLastAccessTime;
	fd->ftLastWriteTime = fad.ftLastWriteTime;
	fd->nFileSizeLow = fad.nFileSizeLow;
	fd->nFileSizeHigh = fad.nFileSizeHigh; 
	fd->dwFlags = FD_ACCESSTIME | FD_ATTRIBUTES | FD_CREATETIME | FD_FILESIZE | FD_WRITESTIME | FD_PROGRESSUI;
	 */
	return TRUE; 
}


// The GetAttributes function is called to retrieve the attributes of the item 
// in the namespace extension. Folder items should include 
// NSEItemAttributes.Folder in the return value. Other attributes should be 
// returned if the item supports the corresponding functionality. 
NSEItemAttributes CFileItem::GetAttributes(NSEItemAttributes /*requested*/)
{
	int ret = NSEIA_CanRename
		| NSEIA_CanDelete
		| NSEIA_CanMove
		| NSEIA_CanLink
		| NSEIA_FileSystem
		| NSEIA_CanCopy;
	return (NSEItemAttributes)ret;
}

int CFileItem::GetTileViewColumnIndices(UINT* indices)
{
	// Display last two column values in Tile View
	indices[0]=1;
	indices[1]=2;
	indices[2]=3;
	return 3;
}

int CFileItem::GetPreviewDetailsColumnIndices(UINT* indices)
{
	// Display last two column values in Tile View
	indices[0]=0;
	indices[1]=1;
	indices[2]=2;
	indices[3]=3;
	return 4;
}

// Called to get the infotip for the item
void CFileItem::GetInfoTip(CString& infotip)
{
	infotip = _T("Infotip");
}

void CFileItem::GetOverlayIcon(CGetOverlayIconEventArgs& e)
{
}

// The IsValid is called to verify whether the item is valid or not. Typically, 
// your implementation should check with the actual underlying data that this 
// item represents and return true or false based on the status of the data.
BOOL CFileItem::IsValid()
{
	//return FileExists(fullPath);
	return TRUE;
}


int CFileItem::GetXPTaskPaneColumnIndices(UINT* indices)
{
	// Display all four column values in the Task Pane of Windows Explorer
	indices[0]=0;
	indices[1]=1;
	indices[2]=2;
	indices[3]=3;
	return 4;
}

/*
// The GetColumnValue method is deprecated. Use the GetColumnValueEx instead (see below).
BOOL CFileItem::GetColumnValue(CString& value,CShellColumn& column)
{
	// Only the 'Name' column displays data for keys.
	switch (column.index)
	{
	case 0: // name
		value = name;
		break;
		
	case 1: // Size
		WIN32_FILE_ATTRIBUTE_DATA fd;
		GetFileAttributesEx(fullPath,GetFileExInfoStandard,&fd);
		value = SizeStrFromLength(fd.nFileSizeLow);
		break;
		
	case 2: // Type
		value = CUtils::GetFileType(fullPath);
		break;

	case 3: // Date Modified
		value = GetFileLastWriteTimeAsString(fullPath);
		break;
		
	}
	
	return TRUE;
}
*/

// The GetColumnValueEx function is called to retrieve the value to be displayed for the
// item in Details/Report mode for the specified column.
BOOL CFileItem::GetColumnValueEx(VARIANT* value,CShellColumn& column)
{
	CString dummy(_T("ABC"));
	CUtils::SetVariantCString(value, dummy);
/*	WIN32_FILE_ATTRIBUTE_DATA fd;
	// Only the 'Name' column displays data for keys.
	switch (column.index)
	{
	case 0: // name
		CUtils::SetVariantCString(value,name);
		break;

	case 1: // Size
		GetFileAttributesEx(fullPath,GetFileExInfoStandard,&fd);
		UINT64 size; size = (((UINT64)fd.nFileSizeHigh) << 32 ) + fd.nFileSizeLow;
		if(value->vt == VT_BSTR) // Value is asked in string format(on XP and before)
		{
			CUtils::SetVariantCString(value,SizeStrFromLength(size));
		}
		else // Value is being asked as VARIANT 
		{
			CUtils::SetVariantUINT64(value,size);
		}
		break;

	case 2: // Type
		CUtils::SetVariantCString(value,CUtils::GetFileType(fullPath)); 
		break;

	case 3: // Date Modified
		if(value->vt==VT_BSTR) // Value is asked in string format(on XP and before)
			CUtils::SetVariantCString(value,GetFileLastWriteTimeAsString(fullPath));
		else // Value is being asked as VARIANT 
		{
			WIN32_FILE_ATTRIBUTE_DATA fd;
			GetFileAttributesEx(fullPath,GetFileExInfoStandard,&fd);
			CUtils::SetVariantFILETIME(value,fd.ftLastWriteTime);
		}
		break;

	default:
		return FALSE;

	}*/

	return TRUE;
}


// The OnChangeName function is called when the item has been renamed in Windows Explorer.
// This function should return true of the renaming was successfully applied.
BOOL CFileItem::OnChangeName(CChangeNameEventArgs& e)
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

void CFileItem::GetIconFileAndIndex(CGetIconFileAndIndexEventArgs& e)
{
	e.iconExtractMode = NSEIEM_IconFileAndIndex;
	e.iconFile = theApp.m_IconFile;
	e.iconIndex = IDI_FILE_Generic-1;
}
