
#pragma once
#include "LF.h"


#define FILEFORMATCOUNT     197


#pragma pack(push,1)

struct RegisteredFileFormat
{
	CHAR Format[7];
	BYTE ContextID;
};

#pragma pack(pop)

extern const RegisteredFileFormat ContextRegistry[FILEFORMATCOUNT];
extern const BYTE ContextSlaves[LFLastQueryContext+1];
