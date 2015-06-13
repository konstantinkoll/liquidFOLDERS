#pragma once
#include "LF.h"
#include "LFDynArray.h"

struct LFFIL1_Item
{
	char StoreID[LFKeySize];
	char FileID[LFKeySize];
	unsigned int LastError;
	bool Processed;
	void* UserData;
};


class LFFileIDList : public LFDynArray<LFFIL1_Item>
{
public:
	LFFileIDList();

	bool AddFileID(char* StoreID, char* FileID, void* UserData=NULL);
	void Reset();
	void SetError(char* key, unsigned int error, LFProgress* pProgress=NULL);
	void SetError(unsigned int idx, unsigned int Result, LFProgress* pProgress=NULL);
	HGLOBAL CreateLiquidFiles();

	bool m_Changes;
};
