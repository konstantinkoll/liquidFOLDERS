
#pragma once
#include "LF.h"


BOOL IsNullValue(UINT Type, const void* pValue);
INT CompareValues(UINT Type, const void* pValue1, const void* pValue2, BOOL CaseSensitive=TRUE);
void ToString(const void* pValue, UINT Type, WCHAR* pStr, SIZE_T cCount);
BOOL GetNextHashtag(WCHAR** ppUnicodeArray, WCHAR* Hashtag, SIZE_T cCount);
