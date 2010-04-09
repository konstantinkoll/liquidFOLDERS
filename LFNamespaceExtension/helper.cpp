// helper.cpp: implementation of the helper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LFNamespaceExtension.h"
#include "helper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CString PathCombineNSE(LPCTSTR path1,LPCTSTR path2)
{
	TCHAR temp[MAX_PATH];
	PathCombine(temp,path1,path2);
	CString ret = temp;
	return ret;
}

BOOL DirectoryExists(LPCTSTR dir)
{
	
	WIN32_FIND_DATA fData;
	HANDLE hnd = FindFirstFile(dir,&fData);
	if(hnd==INVALID_HANDLE_VALUE)
		return FALSE;
	
	FindClose(hnd);
	
	if((fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
		return FALSE;
	
	return TRUE;
}

BOOL FileExists(LPCTSTR dir)
{
	
	WIN32_FIND_DATA fData;
	HANDLE hnd = FindFirstFile(dir,&fData);
	if(hnd==INVALID_HANDLE_VALUE)
		return FALSE;
	
	FindClose(hnd);
	
	if((fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0)
		return FALSE;
	
	return TRUE;
}

BOOL DirectoryHasSubDirectory(LPCTSTR dir)
{
	BOOL ret = FALSE;

	CString pattern = PathCombineNSE(dir,_T("*.*"));
	WIN32_FIND_DATA fd;
	HANDLE hnd = FindFirstFile(pattern,&fd);
	if(hnd==INVALID_HANDLE_VALUE)
		return FALSE;

	do
	{
		if(_tcscmp(fd.cFileName,_T("."))==0 || _tcscmp(fd.cFileName,_T(".."))==0)
			continue;

		if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			ret = TRUE;
			break;
		}
		
	}while(FindNextFile(hnd,&fd)!=0);
	
	FindClose(hnd);
	
	return ret;

}

CString GetFileLastWriteTimeAsString(LPCTSTR path)
{
	WIN32_FILE_ATTRIBUTE_DATA fd = {0};
	GetFileAttributesEx(path,GetFileExInfoStandard,&fd);

	CTime t(fd.ftLastWriteTime);
	return t.Format(_T("%m/%d/%y %I:%M %p"));
}

BOOL DeleteDirectory(LPCTSTR fullPath)
{
	TCHAR temp[MAX_PATH];
	_tcscpy(temp,fullPath);
	temp[_tcslen(fullPath)+1] = _T('\0');

	SHFILEOPSTRUCT sf = {0};
	sf.wFunc = FO_DELETE;
	sf.pFrom = temp;

	return SHFileOperation(&sf)==S_OK && !sf.fAnyOperationsAborted;
}

BOOL CopyDirectory(LPCTSTR src,LPCTSTR dst)
{
	TCHAR srcTemp[MAX_PATH];
	_tcscpy(srcTemp,src);
	srcTemp[_tcslen(src)+1] = _T('\0');
	
	TCHAR dstTemp[MAX_PATH];
	_tcscpy(dstTemp,dst);
	dstTemp[_tcslen(dst)+1] = _T('\0');

	SHFILEOPSTRUCT sf = {0};
	sf.wFunc = FO_COPY;
	sf.pFrom = srcTemp;
	sf.pTo = dstTemp;
	
	return SHFileOperation(&sf)==S_OK && !sf.fAnyOperationsAborted;
}

CString SizeStrFromLength(INT64 length)
{
	CString ret;

	if (length <= 0)
		return _T("0 bytes");
	
	length = length / 1024;
	if (length < 1024)
	{
		ret.Format(_T("%d KB"),length);
		return ret;
	}
	
	length = length / 1024;
	if (length < 1024)
	{
		ret.Format(_T("%d MB"),length);
		return ret;
	}
	
	length = length / 1024;
	ret.Format(_T("%d GB"),length);
	return ret;
	
}