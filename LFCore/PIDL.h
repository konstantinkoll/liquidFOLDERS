
#pragma once
#include <shlobj.h>


BOOL GetPIDLsForStore(const CHAR* pStoreID, LPITEMIDLIST* ppidl, LPITEMIDLIST* ppidlDelegate);
void SendShellNotifyMessage(UINT Msg, const CHAR* pStoreID=NULL, LPITEMIDLIST pidlOld=NULL, LPITEMIDLIST pidlOldDelegate=NULL);
