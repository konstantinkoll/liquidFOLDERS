#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "LFItemDescriptor.h"
#include "IATA.h"
#include <assert.h>


extern unsigned char AttrTypes[];


// Der Inhalt dieses Segments wird �ber alle Instanzen von LFCore geteilt.

#pragma data_seg("common_iata")

#include "IATA_DE.h"
#include "IATA_EN.h"

#pragma data_seg()
#pragma comment(linker, "/SECTION:common_iata,RWS")

bool UseGermanDB = false;


void InitAirportDatabase()
{
	UseGermanDB = (GetThreadLocale() & 0x1FF)==LANG_GERMAN;
}

LFCore_API unsigned int LFIATAGetCountryCount()
{
	return UseGermanDB ? CountryCount_DE : CountryCount_EN;
}

LFCore_API unsigned int LFIATAGetAirportCount()
{
	return UseGermanDB ? AirportCount_DE : AirportCount_EN;
}

LFCore_API LFCountry* LFIATAGetCountry(unsigned int ID)
{
	return UseGermanDB ? &Countries_DE[ID] : &Countries_EN[ID];
}

LFCore_API int LFIATAGetNextAirport(int last, LFAirport** pBuffer)
{
	if (last>=(int)LFIATAGetAirportCount()-1)
		return -1;

	*pBuffer = UseGermanDB ? &Airports_DE[++last] : &Airports_EN[++last];
	return last;
}

LFCore_API int LFIATAGetNextAirportByCountry(unsigned int CountryID, int last, LFAirport** pBuffer)
{
	int count = (int)LFIATAGetAirportCount();

	do
	{
		if (last>=count-1)
			return -1;

		*pBuffer = UseGermanDB ? &Airports_DE[++last] : &Airports_EN[++last];
	}
	while ((*pBuffer)->CountryID!=CountryID);

	return last;
}

LFCore_API bool LFIATAGetAirportByCode(char* Code, LFAirport** pBuffer)
{
	if (!Code)
		return false;
	if (*Code=='\0')
		return false;

	int first = 0;
	int last = (int)LFIATAGetAirportCount()-1;

	while (first<=last)
	{
		int mid = (first+last)/2;

		*pBuffer = UseGermanDB ? &Airports_DE[mid] : &Airports_EN[mid];
		int res = strcmp((*pBuffer)->Code, Code);
		if (res==0)
			return true;

		if (res<0)
		{
			first = mid+1;
		}
		else
		{
			last = mid-1;
		}
	}

	return false;
}

void CustomizeFolderForAirport(LFItemDescriptor* i, LFAirport* airport)
{
	i->IconID = IDI_FLD_Location;
	i->Type = LFTypeVirtual;

	// Ortsname in ANSI
	char tmpStr1[256];
	strcpy_s(tmpStr1, 256, airport->Name);
	strcat_s(tmpStr1, 256, ", ");
	strcat_s(tmpStr1, 256, LFIATAGetCountry(airport->CountryID)->Name);

	// Dateiname in ANSI
	char tmpStr2[256];
	strcpy_s(tmpStr2, 256, airport->Code);
	strcat_s(tmpStr2, 256, UseGermanDB ? " � " : "�");
	strcat_s(tmpStr2, 256, tmpStr1);

	// Dateiname in Unicode
	wchar_t tmpStr3[256];
	MultiByteToWideChar(CP_ACP, 0, tmpStr2, -1, tmpStr3, 256);
	SetAttribute(i, LFAttrFileName, tmpStr3);

	// Ortsname in Unicode
	MultiByteToWideChar(CP_ACP, 0, tmpStr1, -1, tmpStr3, 256);
	SetAttribute(i, LFAttrLocationName, tmpStr3);

	// Weitere Attribute
	SetAttribute(i, LFAttrLocationIATA, airport->Code);
	SetAttribute(i, LFAttrLocationGPS, &airport->Location);
}
