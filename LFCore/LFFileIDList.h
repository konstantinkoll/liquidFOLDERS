#pragma once
#include "liquidFOLDERS.h"
#include "DynArray.h"

struct LFFIL_Item
{
	char StoreID[LFKeySize];
	char FileID[LFKeySize];
	unsigned int LastError;
	bool Processed;
	void* UserData;
};


class LFFileIDList : public DynArray<LFFIL_Item>
{
public:
	LFFileIDList();

	bool AddFileID(char* StoreID, char* FileID, void* UserData);

	bool m_Changes;
};
