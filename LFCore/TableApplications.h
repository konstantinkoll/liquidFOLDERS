
#pragma once
#include "LF.h"


UINT GetApplicationIcon(BYTE nID);


#define APPLICATIONCOUNT     35


#pragma pack(push,1)

struct RegisteredApplication
{
	WCHAR Name[19];
	BYTE ApplicationID;
};

#pragma pack(pop)


extern const RegisteredApplication ApplicationRegistry[APPLICATIONCOUNT];
