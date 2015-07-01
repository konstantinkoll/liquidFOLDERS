#pragma once
#include <shlobj.h>

void FreePIDL(LPITEMIDLIST pidl);
BOOL GetPIDLForStore(CHAR* StoreID, LPITEMIDLIST* ppidl, LPITEMIDLIST* ppidlDelegate);
