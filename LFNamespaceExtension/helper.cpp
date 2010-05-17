// helper.cpp: implementation of the helper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LFNamespaceExtension.h"
#include "helper.h"


CString PathCombineNSE(LPCTSTR path1,LPCTSTR path2)
{
	TCHAR temp[MAX_PATH];
	PathCombine(temp,path1,path2);
	CString ret = temp;
	return ret;
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

CString GetFileLastWriteTimeAsString(LPCTSTR path)
{
	WIN32_FILE_ATTRIBUTE_DATA fd = {0};
	GetFileAttributesEx(path,GetFileExInfoStandard,&fd);

	CTime t(fd.ftLastWriteTime);
	return t.Format(_T("%m/%d/%y %I:%M %p"));
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
