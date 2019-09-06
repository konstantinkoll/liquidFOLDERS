
#pragma once
#include "LF.h"


UINT GetApplicationIcon(BYTE nID);


#define APPLICATIONCOUNT     32


#pragma pack(push,1)

struct RegisteredApplication
{
	WCHAR Name[14];
	BYTE ApplicationID;
};

#pragma pack(pop)


extern const RegisteredApplication ApplicationRegistry[APPLICATIONCOUNT];
