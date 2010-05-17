// helper.h: interface for the helper class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

CString PathCombineNSE(LPCTSTR path1,LPCTSTR path2);
BOOL FileExists(LPCTSTR dir);
CString GetFileLastWriteTimeAsString(LPCTSTR path);
CString SizeStrFromLength(INT64 length);
BOOL CopyDirectory(LPCTSTR src,LPCTSTR dst);
