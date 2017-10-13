
#include "stdafx.h"
#include "LFCore.h"
#include "resource.h"
#include <assert.h>


#pragma data_seg(".shared")

#include "TableGenres.h"

#pragma data_seg()


void SanitizeGenre(LPCWSTR lpGenre, LPWSTR lpBuffer, SIZE_T cCount)
{
	assert(lpGenre);
	assert(lpBuffer);
	assert(cCount>1);

	while(*lpGenre)
	{
		if (*lpGenre==L'/')
			break;

		if (*lpGenre>=L'A')
		{
			*(lpBuffer++) = (WCHAR)toupper(*lpGenre);

			if (--cCount==1)
				break;
		}

		lpGenre++;
	}

	*lpBuffer = L'\0';
}

UINT FindMusicGenre(LPCWSTR lpGenre)
{
	assert(lpGenre);

	WCHAR Genre[256];
	SanitizeGenre(lpGenre, Genre, 256);

	for (UINT a=0; a<MusicGenreCount; a++)
	{
		WCHAR Name[256];
		SanitizeGenre(MusicGenres[a].Name, Name, 256);

		if (wcscmp(Genre, Name)==0)
			return a;
	}

	return 0;
}

LPCWSTR GetGenreName(UINT nID)
{
	return (nID<MusicGenreCount) ? MusicGenres[nID].Name : L"?";
}

UINT GetGenreIcon(UINT nID)
{
	return (nID<MusicGenreCount) ? MusicGenres[nID].IconID : IDI_FLD_DEFAULTGENRE;
}

LFCORE_API INT LFID3GetNextMusicGenre(INT Last, LFMusicGenre** ppMusicGenre)
{
	assert(ppMusicGenre);

	if (Last<-1)
		Last = -1;

	if (Last>=MusicGenreCount-1)
		return -1;

	*ppMusicGenre = &MusicGenres[++Last];

	return Last;
}

LFCORE_API INT LFID3GetNextMusicGenreByIcon(UINT IconID, INT Last, LFMusicGenre** ppMusicGenre)
{
	if (Last<-1)
		Last = -1;

	do
	{
		if (Last>=MusicGenreCount-1)
			return -1;

		*ppMusicGenre = &MusicGenres[++Last];
	}
	while ((*ppMusicGenre)->IconID!=IconID);

	return Last;
}