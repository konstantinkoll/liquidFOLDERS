
#include "stdafx.h"
#include "LFCore.h"
#include "IATA.h"
#include <assert.h>


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

LFCORE_API const LFCountry* LFIATAGetCountry(UINT CountryID)
{
	assert(CountryID<LFIATAGetCountryCount());

	return UseGermanDB ? &Countries_DE[CountryID] : &Countries_EN[CountryID];
}

LFCORE_API INT LFIATAGetNextAirport(INT Last, LFAirport** ppAirport)
{
	if (Last<-1)
		Last = -1;

	if (Last>=(INT)LFIATAGetAirportCount()-1)
		return -1;

	*ppAirport = UseGermanDB ? &Airports_DE[++Last] : &Airports_EN[++Last];

	return Last;
}

LFCORE_API INT LFIATAGetNextAirportByCountry(UINT CountryID, INT Last, LFAirport** ppAirport)
{
	if (Last<-1)
		Last = -1;

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

LFCORE_API BOOL LFIATAGetAirportByCode(LPCSTR pCode, LFAirport** ppAirport)
{
	if (!pCode)
		return FALSE;

	if (strlen(pCode)!=3)
		return FALSE;

	INT First = 0;
	INT Last = (INT)LFIATAGetAirportCount()-1;

	while (First<=Last)
	{
		const INT Mid = (First+Last)/2;

		*ppAirport = UseGermanDB ? &Airports_DE[Mid] : &Airports_EN[Mid];

		const INT Result = strcmp((*ppAirport)->Code, pCode);
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

LFCORE_API void LFIATAGetLocationNameForAirport(LFAirport* pAirport, LPWSTR pStr, SIZE_T cCount)
{
	assert(pAirport);
	assert(pStr);

	CHAR tmpStr[256];
	strcpy_s(tmpStr, 256, pAirport->Name);
	strcat_s(tmpStr, 256, ", ");
	strcat_s(tmpStr, 256, LFIATAGetCountry(pAirport->CountryID)->Name);

	MultiByteToWideChar(CP_ACP, 0, tmpStr, -1, pStr, (INT)cCount);
}

LFCORE_API void LFIATAGetLocationNameForCode(LPCSTR pCode, LPWSTR pStr, SIZE_T cCount)
{
	assert(pCode);
	assert(pStr);

	LFAirport* pAirport;
	if (LFIATAGetAirportByCode(pCode, &pAirport))
	{
		LFIATAGetLocationNameForAirport(pAirport, pStr, cCount);
	}
	else
	{
		*pStr = L'\0';
	}
}
