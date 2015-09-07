
#include "stdafx.h"
#include "LFCore.h"
#include "IATA.h"


#pragma data_seg(".shared")

#include "IATA_DE.h"
#include "IATA_EN.h"

#pragma data_seg()


BOOL UseGermanDB = FALSE;


void InitAirportDatabase()
{
	UseGermanDB = (GetUserDefaultUILanguage() & 0x1FF)==LANG_GERMAN;
}

LFCORE_API UINT LFIATAGetCountryCount()
{
	return UseGermanDB ? CountryCount_DE : CountryCount_EN;
}

LFCORE_API UINT LFIATAGetAirportCount()
{
	return UseGermanDB ? AirportCount_DE : AirportCount_EN;
}

LFCORE_API LFCountry* LFIATAGetCountry(UINT CountryID)
{
	assert(CountryID<LFIATAGetCountryCount());

	return UseGermanDB ? &Countries_DE[CountryID] : &Countries_EN[CountryID];
}

LFCORE_API INT LFIATAGetNextAirport(INT Last, LFAirport** ppAirport)
{
	if (Last>=(INT)LFIATAGetAirportCount()-1)
		return -1;

	*ppAirport = UseGermanDB ? &Airports_DE[++Last] : &Airports_EN[++Last];

	return Last;
}

LFCORE_API INT LFIATAGetNextAirportByCountry(UINT CountryID, INT Last, LFAirport** ppAirport)
{
	const UINT Count = LFIATAGetAirportCount();

	do
	{
		if (Last>=(INT)Count-1)
			return -1;

		*ppAirport = UseGermanDB ? &Airports_DE[++Last] : &Airports_EN[++Last];
	}
	while ((*ppAirport)->CountryID!=CountryID);

	return Last;
}

LFCORE_API BOOL LFIATAGetAirportByCode(CHAR* Code, LFAirport** ppAirport)
{
	if (!Code)
		return FALSE;

	if (strlen(Code)!=3)
		return FALSE;

	INT First = 0;
	INT Last = (INT)LFIATAGetAirportCount()-1;

	while (First<=Last)
	{
		INT Mid = (First+Last)/2;

		*ppAirport = UseGermanDB ? &Airports_DE[Mid] : &Airports_EN[Mid];

		INT Result = strcmp((*ppAirport)->Code, Code);
		if (Result==0)
			return TRUE;

		if (Result<0)
		{
			First = Mid+1;
		}
		else
		{
			Last = Mid-1;
		}
	}

	return FALSE;
}
