#pragma once
#include "liquidFOLDERS.h"

#define LFTL_FirstAlloc          1024
#define LFTL_SubsequentAlloc     1024

#define LFTL_MemoryAlignment     8

// LFTransactionList
// Speichert eine Liste mit Zeigen auf LFItemDescriptor ab, auf die eine Transaktion
// angewandt werden kann (Verändern von Attributwerten, löschen usw). Zu jedem Eintrag
// wird ein frei zu vergebendes unsigned int abgespeichert sowie ein weiteres unsigned int
// mit einem Fehlercode. Eine LFTransactionList kann mehrfach benutzt werden, wobei jedes
// Mal nur die LFItemDescriptor mit Fehlercode LFOk verändert werden. Eine LFTransactionList
// wächst dynamisch, zunächst zum LFTL_FirstAlloc, dann jedes Mal um LFTL_SubsequentAlloc
// Plätze.

struct LFTL_Entry
{
	LFItemDescriptor* Item;
	unsigned int LastError;
	unsigned int UserData;
};

class LFTransactionList
{
public:
	LFTransactionList();
	virtual ~LFTransactionList();

	bool AddItemDescriptor(LFItemDescriptor* i, unsigned int UserData);
	void RemoveEntry(unsigned int idx);
	void RemoveFlaggedEntries();
	void RemoveErrorEntries();

	LFTL_Entry* m_Entries;
	unsigned int m_LastError;
	unsigned int m_Count;
	bool m_Changes;

protected:
	unsigned int m_Allocated;
};
