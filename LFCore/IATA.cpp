
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

LFCORE_API LPCCOUNTRY LFIATAGetCountry(UINT CountryID)
{
	assert(CountryID<LFIATAGetCountryCount());

	return UseGermanDB ? &Countries_DE[CountryID] : &Countries_EN[CountryID];
}

LFCORE_API INT LFIATAGetNextAirport(INT Last, LPCAIRPORT& lpcAirport)
{
	if (Last<-1)
		Last = -1;

	if (Last>=(INT)LFIATAGetAirportCount()-1)
		return -1;

	lpcAirport = UseGermanDB ? &Airports_DE[++Last] : &Airports_EN[++Last];

	return Last;
}

LFCORE_API INT LFIATAGetNextAirportByCountry(UINT CountryID, INT Last, LPCAIRPORT& lpcAirport)
{
	if (Last<-1)
		Last = -1;

	const UINT Count = LFIATAGetAirportCount();

	do
	{
		if (Last>=(INT)Count-1)
			return -1;

		lpcAirport = UseGermanDB ? &Airports_DE[++Last] : &Airports_EN[++Last];
	}
	while (lpcAirport->CountryID!=CountryID);

	return Last;
}

LFCORE_API BOOL LFIATAGetAirportByCode(LPCSTR lpcszCode, LPCAIRPORT& lpcAirport)
{
	if (!lpcszCode)
		return FALSE;

	if (strlen(lpcszCode)!=3)
		return FALSE;

	INT First = 0;
	INT Last = (INT)LFIATAGetAirportCount()-1;

	while (First<=Last)
	{
		const INT Mid = (First+Last)/2;

		lpcAirport = UseGermanDB ? &Airports_DE[Mid] : &Airports_EN[Mid];

		const INT Result = strcmp(lpcAirport->Code, lpcszCode);
		if (!Result)
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

LFCORE_API void LFIATAGetLocationNameForAirport(LPCAIRPORT lpcAirport, LPWSTR pStr, SIZE_T cCount)
{
	assert(lpcAirport);
	assert(pStr);

	CHAR tmpStr[256];
	strcpy_s(tmpStr, 256, lpcAirport->Name);
	strcat_s(tmpStr, 256, ", ");
	strcat_s(tmpStr, 256, LFIATAGetCountry(lpcAirport->CountryID)->Name);

	MultiByteToWideChar(CP_ACP, 0, tmpStr, -1, pStr, (INT)cCount);
}

LFCORE_API void LFIATAGetLocationNameForCode(LPCSTR lpcszCode, LPWSTR pStr, SIZE_T cCount)
{
	assert(lpcszCode);
	assert(pStr);

	LPCAIRPORT lpcAirport;
	if (LFIATAGetAirportByCode(lpcszCode, lpcAirport))
	{
		LFIATAGetLocationNameForAirport(lpcAirport, pStr, cCount);
	}
	else
	{
		*pStr = L'\0';
	}
}
