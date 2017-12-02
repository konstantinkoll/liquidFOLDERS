
#pragma once
#include "LF.h"


BOOL IsNullValue(UINT Type, LPCVOID pValue);
INT CompareValues(UINT Type, LPCVOID pData1, LPCVOID pData2, BOOL CaseSensitive=TRUE);
void ToString(LPCVOID pValue, UINT Type, LPWSTR pStr, SIZE_T cCount);
BOOL GetNextHashtag(LPCWSTR* ppUnicodeArray, LPWSTR Hashtag, SIZE_T cCount);
