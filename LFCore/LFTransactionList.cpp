#include "stdafx.h"
#include "LFCore.h"
#include "LFTransactionList.h"
#include <assert.h>


LFTransactionList::LFTransactionList()
	: DynArray()
{
	m_Changes = false;
}

LFTransactionList::~LFTransactionList()
{
	if (m_Items)
		for (unsigned int a=0; a<m_ItemCount; a++)
			LFFreeItemDescriptor(m_Items[a].Item);
}

bool LFTransactionList::AddItemDescriptor(LFItemDescriptor* i, unsigned int UserData)
{
	assert(i);

	LFTL_Item item = { i, LFOk, UserData, false };

	if (!DynArray::AddItem(item))
		return false;

	i->RefCount++;
	return true;
}
