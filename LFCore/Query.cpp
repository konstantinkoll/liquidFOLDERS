
#include "stdafx.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "LFVariantData.h"
#include "Mutex.h"
#include "Query.h"
#include "Stores.h"
#include "StoreCache.h"
#include <assert.h>


extern HMODULE LFCoreModuleHandle;
extern HANDLE Mutex_Stores;
extern const int CoreOffsets[];
extern const unsigned char AttrTypes[];


wchar_t* wcsistr(const wchar_t* String, const wchar_t* Pattern)
{
	for (wchar_t* start=(wchar_t*)String; *start!=L'\0'; start++)
	{
		for (; ((*start!=L'\0') && (toupper(*start)!=toupper(*Pattern))); start++)
		;

		if (*start==L'\0')
			return NULL;

		wchar_t* pptr = (wchar_t*)Pattern;
		wchar_t* sptr = start;

		while (toupper(*sptr)==toupper(*pptr))
		{
			sptr++;
			pptr++;

			if (*pptr==L'\0')
				return start;
			
		}
	}

	return NULL;
}

char* stristr(const char* String, const char* Pattern)
{
	for (char* start=(char*)String; *start!='\0'; start++)
	{
		for (; ((*start!='\0') && (toupper(*start)!=toupper(*Pattern))); start++)
		;

		if (*start=='\0')
			return NULL;

		char* pptr = (char*)Pattern;
		char* sptr = start;

		while (toupper(*sptr)==toupper(*pptr))
		{
			sptr++;
			pptr++;

			if (*pptr=='\0')
				return start;
			
		}
	}

	return NULL;
}

unsigned char GetRatingCategory(const unsigned char rating)
{
	return (rating==1) ? 1 : rating>>1;
}

unsigned int GetSizeCategory(const __int64 sz)
{
	if (sz<32*1024)
		return 0;
	if (sz<128*1024)
		return 1;
	if (sz<1024*1024)
		return 2;
	if (sz<16384*1024)
		return 3;
	if (sz<131072*1024)
		return 4;

	return 5;
}

unsigned int GetDurationCategory(const unsigned int duration)
{
	if (duration<5*1000)
		return 0;
	if (duration<15*1000)
		return 1;
	if (duration<30*1000)
		return 2;
	if (duration<1*60*1000)
		return 3;
	if (duration<2*60*1000)
		return 4;
	if (duration<3*60*1000)
		return 5;
	if (duration<5*60*1000)
		return 6;
	if (duration<15*60*1000)
		return 7;
	if (duration<30*60*1000)
		return 8;
	if (duration<45*60*1000)
		return 9;
	if (duration<60*60*1000)
		return 10;
	if (duration<90*60*1000)
		return 11;
	if (duration<120*60*1000)
		return 12;

	return 13;
}

bool GetNamePrefix(wchar_t* FullName, wchar_t* Buffer)
{
#define Choose if ((P2) && ((!P1) || (P2<P1))) P1 = P2;

	wchar_t* P1 = wcsstr(FullName, L" —");
	wchar_t* P2;

	P2 = wcsstr(FullName, L" –"); Choose;
	P2 = wcsstr(FullName, L" -"); Choose;
	P2 = wcsstr(FullName, L" \""); Choose;
	P2 = wcsstr(FullName, L" ("); Choose;
	P2 = wcsstr(FullName, L" /"); Choose;
	P2 = wcsstr(FullName, L" »"); Choose;
	P2 = wcsstr(FullName, L" «"); Choose;
	P2 = wcsstr(FullName, L" „"); Choose;
	P2 = wcsstr(FullName, L" “"); Choose;
	P2 = wcsstr(FullName, L"—"); Choose;

	// Wenn kein Trenner gefunden wurde, von rechts nach Ziffern suchen
	if (!P1)
	{
		unsigned char Stelle = 1;

		P2 = &FullName[wcslen(FullName)-1];
		while (P2>FullName)
			switch (Stelle)
			{
			case 1:
				switch (*P2)
				{
				case L'0':
				case L'1':
				case L'2':
				case L'3':
				case L'4':
				case L'5':
				case L'6':
				case L'7':
				case L'8':
				case L'9':
				case L'.':
				case L',':
					P2--;
					break;
				case L' ':
					P2--;
					Stelle = 2;
					break;
				default:
					goto Skip;
				}
				break;
			case 2:
				if (*P2==L' ')
				{
					P2--;
				}
				else
				{
					goto Fertig;
				}
			}

Fertig:
		if (Stelle==2)
			P1 = P2+1;
	}

Skip:
	if (P1)
		wcsncpy_s(Buffer, 256, FullName, P1-FullName);

	return (P1!=NULL);
}

void GetServer(char* URL, char* Server)
{
	char* Pos = strstr(URL, "://");
	if (Pos)
		URL = Pos+3;

	strcpy_s(Server, 256, URL);

	Pos = strchr(Server, '/');
	if (Pos)
		*Pos = '\0';
}


bool CheckCondition(void* value, LFFilterCondition* c)
{
	assert(c->Compare>=LFFilterCompareIgnore);
	assert(c->Compare<=LFFilterCompareContains);

	switch (c->Compare)
	{
	case LFFilterCompareIgnore:
		return true;
	case LFFilterCompareIsNull:
		return IsNullValue(c->AttrData.Attr, value);
	}

	if (!value)
		switch(c->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return LFIsNullVariantData(&c->AttrData);
		case LFFilterCompareIsNotEqual:
			return !LFIsNullVariantData(&c->AttrData);
		default:
			return false;
		}

	assert(c->AttrData.Attr<LFAttributeCount);
	assert(c->AttrData.Type==AttrTypes[c->AttrData.Attr]);
	assert(c->AttrData.Type<LFTypeCount);

	ULARGE_INTEGER uli1;
	ULARGE_INTEGER uli2;
	size_t len1;
	size_t len2;
	SYSTEMTIME stUTC;
	SYSTEMTIME stLocal;
	FILETIME ft;
	wchar_t* conditionarray;
	wchar_t condition[256];
	wchar_t* tagarray;
	wchar_t tag[256];
	char Server[256];

	switch (c->AttrData.Type)
	{
	case LFTypeUnicodeString:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
			if (c->AttrData.Attr==LFAttrFileName)
			{
				len1 = wcslen((wchar_t*)value);
				len2 = wcslen(c->AttrData.UnicodeString);
				if (len1<=len2)
					return false;
				return _wcsnicmp((wchar_t*)value, c->AttrData.UnicodeString, len2)==0;
			}
		case LFFilterCompareIsEqual:
			return _wcsicmp((wchar_t*)value, c->AttrData.UnicodeString)==0;
		case LFFilterCompareIsNotEqual:
			return _wcsicmp((wchar_t*)value, c->AttrData.UnicodeString)!=0;
		case LFFilterCompareBeginsWith:
			len1 = wcslen(c->AttrData.UnicodeString);
			len2 = wcslen((wchar_t*)value);
			if (len1>len2)
				return false;
			return _wcsnicmp((wchar_t*)value, c->AttrData.UnicodeString, len1)==0;
		case LFFilterCompareEndsWith:
			len1 = wcslen(c->AttrData.UnicodeString);
			len2 = wcslen((wchar_t*)value);
			if (len1>len2)
				return false;
			return _wcsicmp(&((wchar_t*)value)[len2-len1], c->AttrData.UnicodeString)==0;
		case LFFilterCompareContains:
			return wcsistr((wchar_t*)value, c->AttrData.UnicodeString)!=NULL;
		default:
			
			assert(false);
			return false;
		}
	case LFTypeUnicodeArray:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareContains:
			conditionarray = c->AttrData.UnicodeArray;
			while (GetNextTag(&conditionarray, condition, 256))
			{
				tagarray = (wchar_t*)value;
				while (GetNextTag(&tagarray, tag, 256))
					if (_wcsicmp(tag, condition)==0)
						return true;
			}
			return false;
		default:
			assert(false);
			return false;
		}
	case LFTypeAnsiString:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
			if (c->AttrData.Attr==LFAttrURL)
			{
				GetServer((char*)value, Server);
				return _stricmp(Server, c->AttrData.AnsiString)==0;
			}
		case LFFilterCompareIsEqual:
			return _stricmp((char*)value, c->AttrData.AnsiString)==0;
		case LFFilterCompareIsNotEqual:
			return _stricmp((char*)value, c->AttrData.AnsiString)!=0;
		case LFFilterCompareBeginsWith:
			len1 = strlen(c->AttrData.AnsiString);
			len2 = strlen((char*)value);
			if (len1>len2)
				return false;
			return _strnicmp((char*)value, c->AttrData.AnsiString, len1)==0;
		case LFFilterCompareEndsWith:
			len1 = strlen(c->AttrData.AnsiString);
			len2 = strlen((char*)value);
			if (len1>len2)
				return false;
			return _stricmp(&((char*)value)[len2-len1], c->AttrData.AnsiString)==0;
		case LFFilterCompareContains:
			return stristr((char*)value, c->AttrData.AnsiString)!=NULL;
		default:
			
			assert(false);
			return false;
		}
	case LFTypeFourCC:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return *(unsigned int*)value==c->AttrData.UINT;
		case LFFilterCompareIsNotEqual:
			return *(unsigned int*)value!=c->AttrData.UINT;
		default:
			assert(false);
			return false;
		}
	case LFTypeRating:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
			return GetRatingCategory(*(unsigned char*)value)==GetRatingCategory(c->AttrData.Rating);
		case LFFilterCompareIsEqual:
			return *(unsigned char*)value==c->AttrData.Rating;
		case LFFilterCompareIsNotEqual:
			return *(unsigned char*)value!=c->AttrData.Rating;
		case LFFilterCompareIsAboveOrEqual:
			return *(unsigned char*)value>=c->AttrData.Rating;
		case LFFilterCompareIsBelowOrEqual:
			return *(unsigned char*)value<=c->AttrData.Rating;
		default:
			assert(false);
			return false;
		}
	case LFTypeUINT:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return *(unsigned int*)value==c->AttrData.UINT;
		case LFFilterCompareIsNotEqual:
			return *(unsigned int*)value!=c->AttrData.UINT;
		case LFFilterCompareIsAboveOrEqual:
			return *(unsigned int*)value>=c->AttrData.UINT;
		case LFFilterCompareIsBelowOrEqual:
			return *(unsigned int*)value<=c->AttrData.UINT;
		default:
			assert(false);
			return false;
		}
	case LFTypeINT64:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
			if (c->AttrData.Attr==LFAttrFileSize)
				return GetSizeCategory(*(__int64*)value)==GetSizeCategory(c->AttrData.INT64);
		case LFFilterCompareIsEqual:
			return *(__int64*)value==c->AttrData.INT64;
		case LFFilterCompareIsNotEqual:
			return *(__int64*)value!=c->AttrData.INT64;
		case LFFilterCompareIsAboveOrEqual:
			return *(__int64*)value>=c->AttrData.INT64;
		case LFFilterCompareIsBelowOrEqual:
			return *(__int64*)value<=c->AttrData.INT64;
		default:
			assert(false);
			return false;
		}
	case LFTypeFraction:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return memcmp(value, &c->AttrData.Fraction, sizeof(LFFraction))==0;
		case LFFilterCompareIsNotEqual:
			return memcmp(value, &c->AttrData.Fraction, sizeof(LFFraction))!=0;
		default:
			assert(false);
			return false;
		}
	case LFTypeDouble:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return *(double*)value==c->AttrData.Double;
		case LFFilterCompareIsNotEqual:
			return *(double*)value!=c->AttrData.Double;
		case LFFilterCompareIsAboveOrEqual:
			return *(double*)value>=c->AttrData.Double;
		case LFFilterCompareIsBelowOrEqual:
			return *(double*)value<=c->AttrData.Double;
		default:
			assert(false);
			return false;
		}
	case LFTypeFlags:
		switch (c->Compare)
		{
		case LFFilterCompareIsEqual:
			return (*(unsigned int*)value & c->AttrData.Flags.Mask)==(c->AttrData.Flags.Flags & c->AttrData.Flags.Mask);
		case LFFilterCompareIsNotEqual:
			return (*(unsigned int*)value & c->AttrData.Flags.Mask)!=(c->AttrData.Flags.Flags & c->AttrData.Flags.Mask);
		default:
			assert(false);
			return false;
		}
	case LFTypeGeoCoordinates:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return memcmp(value, &c->AttrData.GeoCoordinates, sizeof(LFGeoCoordinates))==0;
		case LFFilterCompareIsNotEqual:
			return memcmp(value, &c->AttrData.GeoCoordinates, sizeof(LFGeoCoordinates))!=0;
		default:
			assert(false);
			return false;
		}
	case LFTypeTime:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
			FileTimeToSystemTime((FILETIME*)value, &stUTC);
			SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
			stLocal.wHour = stLocal.wMinute = stLocal.wSecond = stLocal.wMilliseconds = 0;
			TzSpecificLocalTimeToSystemTime(NULL, &stLocal, &stUTC);
			SystemTimeToFileTime(&stUTC, &ft);
			return memcmp(&ft, &c->AttrData.Time, sizeof(FILETIME))==0;
		case LFFilterCompareIsEqual:
			return memcmp(value, &c->AttrData.Time, sizeof(FILETIME))==0;
		case LFFilterCompareIsNotEqual:
			return memcmp(value, &c->AttrData.Time, sizeof(FILETIME))!=0;
		case LFFilterCompareIsAboveOrEqual:
			uli1.LowPart = (*(FILETIME*)value).dwLowDateTime;
			uli1.HighPart = (*(FILETIME*)value).dwHighDateTime;
			uli2.LowPart = c->AttrData.Time.dwLowDateTime;
			uli2.HighPart = c->AttrData.Time.dwHighDateTime;
			return uli1.QuadPart>=uli2.QuadPart;
		case LFFilterCompareIsBelowOrEqual:
			uli1.LowPart = (*(FILETIME*)value).dwLowDateTime;
			uli1.HighPart = (*(FILETIME*)value).dwHighDateTime;
			uli2.LowPart = c->AttrData.Time.dwLowDateTime;
			uli2.HighPart = c->AttrData.Time.dwHighDateTime;
			return uli1.QuadPart<=uli2.QuadPart;
		default:
			assert(false);
			return false;
		}
	case LFTypeDuration:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
			return GetDurationCategory(*(unsigned int*)value)==GetDurationCategory(c->AttrData.UINT);
		case LFFilterCompareIsEqual:
			return *(unsigned int*)value==c->AttrData.UINT;
		case LFFilterCompareIsNotEqual:
			return *(unsigned int*)value!=c->AttrData.UINT;
		case LFFilterCompareIsAboveOrEqual:
			return *(unsigned int*)value>=c->AttrData.UINT;
		case LFFilterCompareIsBelowOrEqual:
			return *(unsigned int*)value<=c->AttrData.UINT;
		default:
			assert(false);
			return false;
		}
	case LFTypeBitrate:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return ((*(unsigned int*)value+500)/1000)==((c->AttrData.UINT+500)/1000);
		case LFFilterCompareIsNotEqual:
			return ((*(unsigned int*)value+500)/1000)!=((c->AttrData.UINT+500)/1000);
		case LFFilterCompareIsAboveOrEqual:
			return *(unsigned int*)value>=c->AttrData.UINT;
		case LFFilterCompareIsBelowOrEqual:
			return *(unsigned int*)value<=c->AttrData.UINT;
		default:
			assert(false);
			return false;
		}
	case LFTypeMegapixel:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return (unsigned int)*((double*)value)==(unsigned int)c->AttrData.Megapixel;
		case LFFilterCompareIsNotEqual:
			return (unsigned int)*((double*)value)!=(unsigned int)c->AttrData.Megapixel;
		case LFFilterCompareIsAboveOrEqual:
			return *(double*)value>=c->AttrData.Megapixel;
		case LFFilterCompareIsBelowOrEqual:
			return *(double*)value<=c->AttrData.Megapixel;
		default:
			assert(false);
			return false;
		}
	}

	// Something fishy is going on - play it safe and include the file
	assert(false);
	return true;
}

int PassesFilterCore(LFCoreAttributes* ca, LFFilter* f)
{
	assert(ca);
	assert(f);

	// Contexts
	if (ca->Flags & LFFlagTrash)
	{
		if (f->ContextID!=LFContextTrash)
			return -1;
	}
	else
		if (ca->Flags & LFFlagArchive)
		{
			if (f->ContextID!=LFContextArchive)
				return -1;
		}

	if ((f->ContextID) || (ca->ContextID==LFContextFilters))
		switch (f->ContextID)
		{
		case LFContextFavorites:
			if (!ca->Rating)
				return -1;
			break;
		case LFContextNew:
			if (!(ca->Flags & LFFlagNew))
				return -1;
			break;
		case LFContextArchive:
			if (!(ca->Flags & LFFlagArchive))
				return -1;
			break;
		case LFContextTrash:
			if (!(ca->Flags & LFFlagTrash))
				return -1;
			break;
		case LFContextFilters:
			if (f->Options.IsPersistent)
				return -1;
		default:
			if (f->ContextID!=ca->ContextID)
				return -1;
		}

	// Attributes
	int advanced = (f->Searchterm[0]==L'\0') ? 1 : 0;

	LFFilterCondition* c = f->ConditionList;
	while (c)
	{
		if (c->AttrData.Attr<=LFLastCoreAttribute)
		{
			if (CoreOffsets[c->AttrData.Attr]!=-1)
				if (!CheckCondition((char*)ca+CoreOffsets[c->AttrData.Attr], c))
					return -1;
		}
		else
		{
			advanced = 0;
		}

		c = c->Next;
	}

	return advanced;
}

bool PassesFilterSlaves(LFItemDescriptor* i, LFFilter* f)
{
	assert(f);
	assert(i);

	// Attributes
	LFFilterCondition* c = f->ConditionList;
	while (c)
	{
		if (c->AttrData.Attr>LFLastCoreAttribute)
			if (!CheckCondition(i->AttributeValues[c->AttrData.Attr], c))
				return false;

		c = c->Next;
	}

	if (f->Searchterm[0]=='\0')
		return true;

	// Globaler Suchbegriff
	bool checkTime = true;
	for (unsigned int a=0; a<wcslen(f->Searchterm); a++)
	{
		wchar_t ch = f->Searchterm[a];
		if (((ch<L'0') || (ch>L'9')) && (ch!=L':') && (ch!=L'.') && (ch!=L'/') && (ch!=L'.'))
		{
			checkTime = false;
			break;
		}
	}

	char Searchterm[256];
	WideCharToMultiByte(CP_ACP, 0, f->Searchterm, -1, Searchterm, 256, NULL, NULL);

	for (unsigned int a=0; a<LFAttributeCount; a++)
		if ((a!=LFAttrStoreID) && (a!=LFAttrFileID) && (i->AttributeValues[a]))
		{
			wchar_t tmpStr[256];

			switch (AttrTypes[a])
			{
			case LFTypeUnicodeString:
			case LFTypeUnicodeArray:
				if (wcsistr((wchar_t*)i->AttributeValues[a], f->Searchterm)!=NULL)
					return true;
				break;
			case LFTypeAnsiString:
				if (stristr((char*)i->AttributeValues[a], Searchterm)!=NULL)
					return true;
				break;
			case LFTypeGeoCoordinates:
				break;
			case LFTypeTime:
				if (!checkTime)
					break;
			default:
				LFAttributeToString(i, a, tmpStr, 256);
				if (wcsistr(tmpStr, f->Searchterm)!=NULL)
					return true;
			}
		}

	return false;
}


// Query helpers

void RetrieveStore(char* StoreID, LFFilter* f, LFSearchResult* sr)
{
	OPEN_STORE(StoreID, false, sr->m_LastError = res);

	if (idx1)
		idx1->Retrieve(f, sr);

	CLOSE_STORE();
}

void QueryStores(LFFilter* f, LFSearchResult* sr)
{
	sr->m_HasCategories = true;

	// Volumes
	if (f)
		if (f->Options.AddVolumes)
			sr->AddVolumes();

	// Stores
	if (GetMutex(Mutex_Stores))
	{
		AddStoresToSearchResult(sr);
		ReleaseMutex(Mutex_Stores);
	}
	else
	{
		sr->m_LastError = LFMutexError;
	}
}

__forceinline void QueryTree(LFFilter* f, LFSearchResult* sr)
{
	RetrieveStore(f->StoreID, f, sr);
}

__forceinline void QuerySearch(LFFilter* f, LFSearchResult* sr)
{
	if (f->StoreID[0]=='\0')
	{
		// All stores
		if (!GetMutex(Mutex_Stores))
		{
			sr->m_LastError = LFMutexError;
			return;
		}

		char* IDs;
		unsigned int count = FindStores(&IDs);
		ReleaseMutex(Mutex_Stores);

		if (count)
		{
			char* ptr = IDs;
			for (unsigned int a=0; a<count; a++)
			{
				RetrieveStore(ptr, f, sr);
				ptr += LFKeySize;
			}

			free(IDs);
		}
	}
	else
	{
		// Single store
		RetrieveStore(f->StoreID, f, sr);
	}
}


// Public functions

LFCore_API LFSearchResult* LFQuery(LFFilter* f)
{
	DWORD start = GetTickCount();

	LFSearchResult* sr = new LFSearchResult(f);

	if (!f)
	{
		QueryStores(f, sr);
	}
	else
	{
		// Ggf. Default Store einsetzen
		if ((f->StoreID[0]=='\0') && (f->Mode==LFFilterModeDirectoryTree))
			if (LFDefaultStoreAvailable())
			{
				char* ds = LFGetDefaultStore();
				strcpy_s(f->StoreID, LFKeySize, ds);
				free(ds);
			}
			else
			{
				sr->m_LastError = LFNoDefaultStore;
				goto Finish;
			}

		// Query
		switch (f->Mode)
		{
		case LFFilterModeStores:
			QueryStores(f, sr);
			break;
		case LFFilterModeDirectoryTree:
			QueryTree(f, sr);
			break;
		case LFFilterModeSearch:
			QuerySearch(f, sr);
			break;
		default:
			sr->m_LastError = LFIllegalQuery;
		}
	}

Finish:
	sr->m_QueryTime = GetTickCount()-start;

	return sr;
}

LFCore_API LFSearchResult* LFQuery(LFFilter* f, LFSearchResult* base, int first, int last)
{
	DWORD start = GetTickCount();

	LFSearchResult* sr;

	if ((f->Mode>=LFFilterModeDirectoryTree) && (f->Options.IsSubfolder) && (base->m_RawCopy) &&
		(first<=last) && (first>=0) && (first<(int)base->m_ItemCount) && (last>=0) && (last<(int)base->m_ItemCount))
	{
		sr = base;

		sr->m_LastError = LFOk;
		sr->KeepRange(first, last);
		sr->SetMetadataFromFilter(f);
	}
	else
	{
		sr = new LFSearchResult(LFContextSubfolderDefault);
		sr->m_LastError = LFIllegalQuery;
	}

	sr->m_QueryTime = GetTickCount()-start;

	return sr;
}

LFCore_API LFStatistics* LFQueryStatistics(char* StoreID)
{
	LFStatistics* stat = new LFStatistics();
	ZeroMemory(stat, sizeof(LFStatistics));

	if (!StoreID)
	{
		stat->LastError = LFIllegalKey;
		return stat;
	}

	if (!GetMutex(Mutex_Stores))
	{
		stat->LastError = LFMutexError;
		return stat;
	}

	// All stores
	char* IDs;
	unsigned int count = FindStores(&IDs);
	ReleaseMutex(Mutex_Stores);

	if (count)
	{
		char* ptr = IDs;
		for (unsigned int a=0; a<count; a++)
		{
			if ((*StoreID=='\0') || (strcmp(ptr, StoreID)==0))
			{
				HANDLE StoreLock = NULL;
				LFStoreDescriptor* slot = FindStore(ptr, &StoreLock);

				if (slot)
					for (unsigned int b=0; b<min(LFLastQueryContext, 32); b++)
					{
						stat->FileCount[b] += slot->FileCount[b];
						stat->FileSize[b] += slot->FileSize[b];
					}

				ReleaseMutexForStore(StoreLock);
			}

			ptr += LFKeySize;
		}

		free(IDs);
	}

	return stat;
}