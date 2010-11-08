#pragma once
#include <shlobj.h>

void FreePIDL(LPITEMIDLIST pidl);
bool GetPIDLForStore(char* StoreID, LPITEMIDLIST* ppidl, LPITEMIDLIST* ppidlDelegate);
