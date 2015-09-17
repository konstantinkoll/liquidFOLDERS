
#pragma once
#include <shlobj.h>


BOOL GetPIDLsForStore(CHAR* pStoreID, LPITEMIDLIST* ppidl, LPITEMIDLIST* ppidlDelegate);
void SendShellNotifyMessage(UINT Msg, CHAR* pStoreID=NULL, LPITEMIDLIST pidlOld=NULL, LPITEMIDLIST pidlOldDelegate=NULL);
