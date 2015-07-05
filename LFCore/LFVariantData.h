
#pragma once
#include "LF.h"


BOOL IsNullValue(UINT Type, void* v);
INT CompareValues(UINT Type, void* v1, void* v2, BOOL CaseSensitive=TRUE);
void ToString(void* v, UINT Type, WCHAR* pStr, SIZE_T cCount);
BOOL GetNextTag(WCHAR** ppUnicodeArray, WCHAR* Tag, SIZE_T cCount);
