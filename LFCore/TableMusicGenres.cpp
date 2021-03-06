
#include "stdafx.h"
#include "LFCore.h"
#include "resource.h"
#include "TableMusicGenres.h"


#pragma data_seg(".shared")

extern const LFMusicGenre MusicGenres[MUSICGENRECOUNT] = {
	{ L"", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Abstract", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Acapella", IDI_FLD_REDCURTAIN, FALSE, TRUE },
	{ L"Acid Jazz", IDI_FLD_JAZZ, FALSE, TRUE },
	{ L"Acid Punk", IDI_FLD_PUNK, FALSE, TRUE },
	{ L"Acid", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Acoustic", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Alternative Rock", IDI_FLD_ALTERNATIVE, FALSE, TRUE },
	{ L"Alternative", IDI_FLD_ALTERNATIVE, TRUE, TRUE },
	{ L"AlternRock", IDI_FLD_ALTERNATIVE, FALSE, FALSE },
	{ L"Ambient", IDI_FLD_AMBIENT, TRUE, TRUE },
	{ L"Anime", IDI_FLD_SOUNDTRACKS, FALSE, TRUE },
	{ L"Art Rock", IDI_FLD_ROCK, FALSE, TRUE },
	{ L"Audio Theatre", IDI_FLD_REDCURTAIN, TRUE, TRUE },
	{ L"Audiobook", IDI_FLD_SPOKENWORD, FALSE, TRUE },
	{ L"Avantgarde", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Ballad", IDI_FLD_REDCURTAIN, FALSE, TRUE },
	{ L"Baroque", IDI_FLD_MUSICSHEET, FALSE, TRUE },
	{ L"Bass", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Beat", IDI_FLD_OLDIES, FALSE, TRUE },
	{ L"Bebob", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Bhangra", IDI_FLD_WORLDMUSIC, FALSE, TRUE },
	{ L"Big Band", IDI_FLD_MUSICSHEET, FALSE, TRUE },
	{ L"Big Beat", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Black Metal", IDI_FLD_METAL, FALSE, TRUE },
	{ L"Bluegrass", IDI_FLD_COUNTRY, FALSE, TRUE },
	{ L"Blues", IDI_FLD_BLUES, TRUE, TRUE },
	{ L"Bollywood", IDI_FLD_WORLDMUSIC, FALSE, TRUE },
	{ L"Booty Bass", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Breakbeat", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Britpop", IDI_FLD_POP, FALSE, TRUE },
	{ L"Cabaret", IDI_FLD_REDCURTAIN, FALSE, TRUE },
	{ L"Celtic", IDI_FLD_WORLDMUSIC, FALSE, TRUE },
	{ L"Chamber Music", IDI_FLD_MUSICSHEET, FALSE, TRUE },
	{ L"Chanson", IDI_FLD_REDCURTAIN, FALSE, TRUE },
	{ L"Chillout", IDI_FLD_AMBIENT, FALSE, TRUE },
	{ L"Chorus", IDI_FLD_REDCURTAIN, FALSE, TRUE },
	{ L"Christian Gangsta Rap", IDI_FLD_HIPHOP, FALSE, TRUE },
	{ L"Christian Rap", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Christian Rock", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Classic Rock", IDI_FLD_ROCK, FALSE, TRUE },
	{ L"Classical", IDI_FLD_MUSICSHEET, TRUE, TRUE },
	{ L"Club", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Club-House", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Comedy", IDI_FLD_REDCURTAIN, FALSE, TRUE },
	{ L"Contemporary Christian", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Country", IDI_FLD_COUNTRY, TRUE, TRUE },
	{ L"Crossover", IDI_FLD_ALTERNATIVE, FALSE, TRUE },
	{ L"Cult", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Dance & DJ", IDI_FLD_DANCE, FALSE, TRUE },
	{ L"Dance Hall", IDI_FLD_REGGAE, FALSE, TRUE },
	{ L"Dance", IDI_FLD_DANCE, TRUE, TRUE },
	{ L"Darkwave", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Death Metal", IDI_FLD_METAL, FALSE, TRUE },
	{ L"Disco", IDI_FLD_DANCE, FALSE, TRUE },
	{ L"Downtempo", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Dream", IDI_FLD_AMBIENT, FALSE, TRUE },
	{ L"Drum & Bass", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Drum Solo", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Dub", IDI_FLD_REGGAE, FALSE, TRUE },
	{ L"Dubstep", IDI_FLD_REGGAE, FALSE, TRUE },
	{ L"Duet", IDI_FLD_REDCURTAIN, FALSE, TRUE },
	{ L"Easy Listening", IDI_FLD_EASYLISTENING, TRUE, TRUE },
	{ L"EBM", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Eclectic", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Electro", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Electroclash", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Electronic", IDI_FLD_ELECTRONIC, TRUE, TRUE },
	{ L"Emo", IDI_FLD_ALTERNATIVE, FALSE, TRUE },
	{ L"Ethnic", IDI_FLD_WORLDMUSIC, FALSE, TRUE },
	{ L"Euro-House", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Euro-Techno", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Eurodance", IDI_FLD_DANCE, FALSE, TRUE },
	{ L"Europop", IDI_FLD_POP, FALSE, TRUE },
	{ L"Experimental", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Fast Fusion", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Folk", IDI_FLD_FOLK, FALSE, TRUE },
	{ L"Folk-Rock", IDI_FLD_FOLK, FALSE, TRUE },
	{ L"Folklore", IDI_FLD_FOLK, FALSE, TRUE },
	{ L"Freestyle", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Funk", IDI_FLD_SOUL, FALSE, TRUE },
	{ L"Fusion", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"G-Funk", IDI_FLD_HIPHOP, FALSE, TRUE },
	{ L"Game", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Gangsta", IDI_FLD_HIPHOP, FALSE, TRUE },
	{ L"Gangsta Rap", IDI_FLD_HIPHOP, FALSE, TRUE },
	{ L"Garage Rock", IDI_FLD_ROCK, FALSE, TRUE },
	{ L"Garage", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Global", IDI_FLD_WORLDMUSIC, FALSE, TRUE },
	{ L"Goa", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Gospel", IDI_FLD_REDCURTAIN, FALSE, TRUE },
	{ L"Gothic Rock", IDI_FLD_ROCK, FALSE, TRUE },
	{ L"Gothic", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Grunge", IDI_FLD_ALTERNATIVE, FALSE, TRUE },
	{ L"Hard Rock", IDI_FLD_ROCK, FALSE, TRUE },
	{ L"Hardcore", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Heavy Metal", IDI_FLD_METAL, FALSE, TRUE },
	{ L"Hip-Hop", IDI_FLD_HIPHOP, TRUE, TRUE },
	{ L"House", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Humour", IDI_FLD_REDCURTAIN, FALSE, TRUE },
	{ L"IDM", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Illbient", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Indie Rock", IDI_FLD_ALTERNATIVE, FALSE, TRUE },
	{ L"Indie", IDI_FLD_ALTERNATIVE, FALSE, TRUE },
	{ L"Industrial", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Industro-Goth", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Instrumental Pop", IDI_FLD_POP, FALSE, TRUE },
	{ L"Instrumental Rock", IDI_FLD_ROCK, FALSE, TRUE },
	{ L"Instrumental", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"International", IDI_FLD_WORLDMUSIC, FALSE, TRUE },
	{ L"Jam Band", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Jazz", IDI_FLD_JAZZ, TRUE, TRUE },
	{ L"Jazz+Funk", IDI_FLD_JAZZ, FALSE, TRUE },
	{ L"J-Pop", IDI_FLD_JPOP, TRUE, TRUE },
	{ L"Jungle", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Krautrock", IDI_FLD_ROCK, FALSE, TRUE },
	{ L"Latin", IDI_FLD_LATIN, TRUE, TRUE },
	{ L"Leftfield", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Lo-Fi", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Lounge", IDI_FLD_AMBIENT, FALSE, TRUE },
	{ L"Math Rock", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Meditative", IDI_FLD_AMBIENT, FALSE, TRUE },
	{ L"Merengue", IDI_FLD_LATIN, FALSE, TRUE },
	{ L"Metal", IDI_FLD_METAL, TRUE, TRUE },
	{ L"Musical", IDI_FLD_REDCURTAIN, FALSE, TRUE },
	{ L"National Folk", IDI_FLD_FOLK, FALSE, TRUE },
	{ L"Native US", IDI_FLD_WORLDMUSIC, FALSE, TRUE },
	{ L"Negerpunk", IDI_FLD_DEFAULTGENRE, FALSE, FALSE },
	{ L"Neoclassical", IDI_FLD_MUSICSHEET, FALSE, TRUE },
	{ L"Neue Deutsche Welle", IDI_FLD_POP, FALSE, TRUE },
	{ L"New Age", IDI_FLD_AMBIENT, FALSE, TRUE },
	{ L"New Romantic", IDI_FLD_AMBIENT, FALSE, TRUE },
	{ L"New Wave", IDI_FLD_POP, FALSE, TRUE },
	{ L"Noise", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Nu-Breakz", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Oldie", IDI_FLD_OLDIES, FALSE, FALSE },
	{ L"Oldies", IDI_FLD_OLDIES, TRUE, TRUE },
	{ L"Opera", IDI_FLD_MUSICSHEET, FALSE, TRUE },
	{ L"Other", IDI_FLD_DEFAULTGENRE, TRUE, TRUE },
	{ L"Podcast", IDI_FLD_SPOKENWORD, FALSE, TRUE },
	{ L"Polka", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Polsk Punk", IDI_FLD_PUNK, FALSE, TRUE },
	{ L"Pop", IDI_FLD_POP, TRUE, TRUE },
	{ L"Pop-Folk", IDI_FLD_POP, FALSE, TRUE },
	{ L"Pop-Punk", IDI_FLD_POP, FALSE, TRUE },
	{ L"Porn Groove", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Post-Punk", IDI_FLD_PUNK, FALSE, TRUE },
	{ L"Post-Rock", IDI_FLD_ROCK, FALSE, TRUE },
	{ L"Power Ballad", IDI_FLD_METAL, FALSE, TRUE },
	{ L"Pranks", IDI_FLD_REDCURTAIN, FALSE, TRUE },
	{ L"Primus", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Progressive Rock", IDI_FLD_ROCK, FALSE, TRUE },
	{ L"PsybientSynthPop", IDI_FLD_POP, FALSE, TRUE },
	{ L"Psychedelic", IDI_FLD_PSYCHEDELIC, FALSE, TRUE },
	{ L"Psychedelic Rock", IDI_FLD_ROCK, FALSE, TRUE },
	{ L"Psytrance", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Punk Rock", IDI_FLD_ROCK, FALSE, TRUE },
	{ L"Punk", IDI_FLD_PUNK, FALSE, TRUE },
	{ L"R&B", IDI_FLD_RANDB, TRUE, TRUE },
	{ L"Rap & Hip-Hop", IDI_FLD_HIPHOP, FALSE, TRUE },
	{ L"Rap", IDI_FLD_HIPHOP, FALSE, TRUE },
	{ L"Rave", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Reggae", IDI_FLD_REGGAE, TRUE, TRUE },
	{ L"Retro", IDI_FLD_OLDIES, FALSE, TRUE },
	{ L"Revival", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Rhythmic Soul", IDI_FLD_SOUL, FALSE, TRUE },
	{ L"Rock & Roll", IDI_FLD_OLDIES, FALSE, TRUE },
	{ L"Rock", IDI_FLD_ROCK, TRUE, TRUE },
	{ L"Salsa", IDI_FLD_LATIN, FALSE, TRUE },
	{ L"Samba", IDI_FLD_LATIN, FALSE, TRUE },
	{ L"Satire", IDI_FLD_SPOKENWORD, FALSE, TRUE },
	{ L"Shoegaze", IDI_FLD_ROCK, FALSE, TRUE },
	{ L"Showtunes", IDI_FLD_SOUNDTRACKS, FALSE, TRUE },
	{ L"Ska", IDI_FLD_REGGAE, FALSE, TRUE },
	{ L"Slow Jam", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Slow Rock", IDI_FLD_ROCK, FALSE, TRUE },
	{ L"Sonata", IDI_FLD_MUSICSHEET, FALSE, TRUE },
	{ L"Soul", IDI_FLD_SOUL, TRUE, TRUE },
	{ L"Sound Clip", IDI_FLD_SOUNDTRACKS, FALSE, TRUE },
	{ L"Soundtrack", IDI_FLD_SOUNDTRACKS, FALSE, FALSE },
	{ L"Soundtracks", IDI_FLD_SOUNDTRACKS, TRUE, TRUE },
	{ L"Southern Rock", IDI_FLD_ROCK, FALSE, TRUE },
	{ L"Space Rock", IDI_FLD_ROCK, FALSE, TRUE },
	{ L"Space", IDI_FLD_AMBIENT, FALSE, TRUE },
	{ L"Speech", IDI_FLD_SPOKENWORD, TRUE, TRUE },
	{ L"Swing", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Symphonic Rock", IDI_FLD_MUSICSHEET, FALSE, TRUE },
	{ L"Symphony", IDI_FLD_MUSICSHEET, FALSE, TRUE },
	{ L"Tango", IDI_FLD_LATIN, FALSE, TRUE },
	{ L"Techno", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Techno-Industrial", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Terror", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Thrash Metal", IDI_FLD_METAL, FALSE, TRUE },
	{ L"Top 100", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Top 40", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Trailer", IDI_FLD_SOUNDTRACKS, FALSE, TRUE },
	{ L"Trance", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Tribal", IDI_FLD_WORLDMUSIC, FALSE, TRUE },
	{ L"Trip-Hop", IDI_FLD_ELECTRONIC, FALSE, TRUE },
	{ L"Trop Rock", IDI_FLD_ROCK, FALSE, TRUE },
	{ L"Vari?t? Fran?aise", IDI_FLD_DEFAULTGENRE, FALSE, TRUE },
	{ L"Vocal", IDI_FLD_REDCURTAIN, FALSE, TRUE },
	{ L"World Music", IDI_FLD_WORLDMUSIC, TRUE, TRUE }
};

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

	for (UINT a=0; a<MUSICGENRECOUNT; a++)
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
	return (nID<MUSICGENRECOUNT) ? MusicGenres[nID].Name : L"?";
}

UINT GetGenreIcon(UINT nID)
{
	return (nID<MUSICGENRECOUNT) ? MusicGenres[nID].IconID : IDI_FLD_DEFAULTGENRE;
}

LFCORE_API INT LFID3GetNextMusicGenre(INT Last, LPCMUSICGENRE& lpcMusicGenre)
{
	if (Last<-1)
		Last = -1;

	if (Last>=MUSICGENRECOUNT-1)
		return -1;

	lpcMusicGenre = &MusicGenres[++Last];

	return Last;
}

LFCORE_API INT LFID3GetNextMusicGenreByIcon(UINT IconID, INT Last, LPCMUSICGENRE& lpcMusicGenre)
{
	if (Last<-1)
		Last = -1;

	do
	{
		if (Last>=MUSICGENRECOUNT-1)
			return -1;

		lpcMusicGenre = &MusicGenres[++Last];
	}
	while (lpcMusicGenre->IconID!=IconID);

	return Last;
}
