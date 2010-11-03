#pragma once
#include <shlobj.h>

bool GetPIDLForStore(char* StoreID, LPITEMIDLIST* ppidl, LPITEMIDLIST* ppidlDelegate);
