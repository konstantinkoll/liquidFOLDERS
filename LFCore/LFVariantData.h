
#pragma once
#include "LF.h"


extern const BYTE AttrTypes[];
extern const SIZE_T AttrSizes[];

BOOL IsNullValue(UINT Type, const void* v);
INT CompareValues(UINT Type, const void* v1, const void* v2, BOOL CaseSensitive=TRUE);
void ToString(const void* v, UINT Type, WCHAR* pStr, SIZE_T cCount);
BOOL GetNextTag(WCHAR** ppUnicodeArray, WCHAR* Tag, SIZE_T cCount);
