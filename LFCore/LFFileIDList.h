#pragma once
#include "LF.h"
#include "LFDynArray.h"

struct LFFIL1_Item
{
	CHAR StoreID[LFKeySize];
	CHAR FileID[LFKeySize];
	UINT LastError;
	BOOL Processed;
	void* UserData;
};


class LFFileIDList : public LFDynArray<LFFIL1_Item>
{
public:
	LFFileIDList();

	BOOL AddFileID(CHAR* StoreID, CHAR* FileID, void* UserData=NULL);
	void Reset();
	void SetError(CHAR* key, UINT error, LFProgress* pProgress=NULL);
	void SetError(UINT idx, UINT Result, LFProgress* pProgress=NULL);
	HGLOBAL CreateLiquidFiles();

	BOOL m_Changes;
};
