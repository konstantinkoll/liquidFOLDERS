
#pragma once
#include "LF.h"


#define FILEFORMATCOUNT     262


#pragma pack(push,1)

struct RegisteredFileFormat
{
	CHAR Format[8];
	BYTE SystemContextID;
	BYTE UserContextID;
};

#pragma pack(pop)

extern const RegisteredFileFormat ContextRegistry[FILEFORMATCOUNT];
extern const BYTE ContextSlaves[LFLastPersistentContext+1];
