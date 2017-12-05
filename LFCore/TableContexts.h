
#pragma once
#include "LF.h"


#define FILEFORMATCOUNT     247


#pragma pack(push,1)

struct RegisteredFileFormat
{
	CHAR Format[7];
	BYTE SystemContextID;
	BYTE UserContextID;
};

#pragma pack(pop)

extern const RegisteredFileFormat ContextRegistry[FILEFORMATCOUNT];
extern const BYTE ContextSlaves[LFLastPersistentContext+1];
