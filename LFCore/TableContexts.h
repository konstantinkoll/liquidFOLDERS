
#pragma once
#include "LF.h"


#define FILEFORMATCOUNT     273

#define NOCOMPRESSION     0
#define CANCOMPRESS       1

#pragma pack(push,1)

struct RegisteredFileFormat
{
	CHAR Format[8];
	ITEMCONTEXT SystemContextID;
	ITEMCONTEXT UserContextID;
	BYTE CanCompress;
};

#pragma pack(pop)

extern const RegisteredFileFormat ContextRegistry[FILEFORMATCOUNT];
extern const BYTE ContextSlaves[LFLastPersistentContext+1];
