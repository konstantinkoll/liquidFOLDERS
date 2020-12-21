
#pragma once
#include "LF.h"


#define FILEFORMATCOUNT     279

#pragma pack(push,1)

struct RegisteredFileFormat
{
	CHAR Format[LFExtSize];
	ITEMCONTEXT SystemContextID;
	ITEMCONTEXT UserContextID;
	BYTE CanCompress;
};

#pragma pack(pop)

extern const RegisteredFileFormat ContextRegistry[FILEFORMATCOUNT];
extern const BYTE ContextSlaves[LFLastPersistentContext+1];
