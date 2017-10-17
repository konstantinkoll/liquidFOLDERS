
#pragma once
#include "LF.h"


UINT FindMusicGenre(LPCWSTR lpGenre);
LPCWSTR GetGenreName(UINT nID);
UINT GetGenreIcon(UINT nID);


#define MUSICGENRECOUNT     203

extern const LFMusicGenre MusicGenreRegistry[MUSICGENRECOUNT];
