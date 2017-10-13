
#pragma once
#include "LF.h"


#define APPLICATIONCOUNT     25


#pragma pack(push,1)

struct RegisteredApplication
{
	WCHAR Name[14];
	BYTE ApplicationID;
};

#pragma pack(pop)

extern const RegisteredApplication ApplicationRegistry[APPLICATIONCOUNT];
