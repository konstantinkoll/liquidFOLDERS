// helper.h: interface for the helper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HELPER_H__DDFB144F_EA5E_47EE_9E84_856E5958D727__INCLUDED_)
#define AFX_HELPER_H__DDFB144F_EA5E_47EE_9E84_856E5958D727__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

CString PathCombineNSE(LPCTSTR path1,LPCTSTR path2);
BOOL FileExists(LPCTSTR dir);
BOOL DirectoryExists(LPCTSTR dir);
BOOL DirectoryHasSubDirectory(LPCTSTR dir);
CString GetFileLastWriteTimeAsString(LPCTSTR path);
CString SizeStrFromLength(INT64 length);
BOOL DeleteDirectory(LPCTSTR fullPath);
BOOL CopyDirectory(LPCTSTR src,LPCTSTR dst);

#endif // !defined(AFX_HELPER_H__DDFB144F_EA5E_47EE_9E84_856E5958D727__INCLUDED_)
