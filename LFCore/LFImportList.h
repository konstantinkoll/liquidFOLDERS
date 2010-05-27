#pragma once
#include "liquidFOLDERS.h"

#define LFIL_FirstAlloc          1024
#define LFIL_SubsequentAlloc     1024

#define LFIL_MemoryAlignment     8

// LFImportList
// Speichert eine Liste mit Zeigen auf Unicode-Strings ab, die die vollständigen
// Pfade zu importierender Dateien enthalten.

class LFImportList
{
public:
	LFImportList();
	virtual ~LFImportList();

	bool AddPath(wchar_t* path);

	wchar_t** m_Entries;
	unsigned int m_LastError;
	unsigned int m_Count;

protected:
	unsigned int m_Allocated;
};
