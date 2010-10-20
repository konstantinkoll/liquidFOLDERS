#pragma once
#include "liquidFOLDERS.h"
#include "DynArray.h"

struct LFTL_Item
{
	LFItemDescriptor* Item;
	unsigned int LastError;
	unsigned int UserData;
	bool Processed;
};


class LFTransactionList : public DynArray<LFTL_Item>
{
public:
	LFTransactionList();
	~LFTransactionList();

	bool AddItemDescriptor(LFItemDescriptor* i, unsigned int UserData);

	bool m_Changes;
};
