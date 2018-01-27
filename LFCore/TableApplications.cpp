
#include "stdafx.h"
#include "TableApplications.h"


#pragma data_seg(".shared")

extern const RegisteredApplication ApplicationRegistry[APPLICATIONCOUNT] = {
	{ L"",             LFApplicationNone },
	{ L"Boomerang",    LFApplicationBoomerang },
	{ L"Cinemagraph" , LFApplicationCinemagraph },
	{ L"Facebook",     LFApplicationFacebook },
	{ L"Hipstamatic",  LFApplicationHipstamatic },
	{ L"Hyperlapse",   LFApplicationHyperlapse },
	{ L"IncrediBooth", LFApplicationIncrediBooth },
	{ L"Instagram",    LFApplicationInstagram },
	{ L"Labelbox",     LFApplicationLabelbox },
	{ L"Layout",       LFApplicationLayout },
	{ L"Pinterest",    LFApplicationPinterest },
	{ L"Retrica",      LFApplicationRetrica },
	{ L"Snapchat",     LFApplicationSnapchat },
	{ L"VintageCam",   LFApplicationVintageCam },
	{ L"VSCO",         LFApplicationVSCO },
	{ L"YouTube",      LFApplicationYouTube },
	{ L"Jodel",        LFApplicationJodel },
	{ L"Vimeo",        LFApplicationVimeo },
	{ L"Picfx",        LFApplicationPicfx },

	{ L"FB",           LFApplicationFacebook },
	{ L"Flixel",       LFApplicationCinemagraph },
	{ L"Hipsta",       LFApplicationHipstamatic },
	{ L"HipstaPrint",  LFApplicationHipstamatic },
	{ L"IG",           LFApplicationInstagram },
	{ L"Insta",        LFApplicationInstagram },
	{ L"Pin",          LFApplicationPinterest },
	{ L"SC",           LFApplicationSnapchat },
	{ L"Snap",         LFApplicationSnapchat },
	{ L"YT",           LFApplicationYouTube }
};

#pragma data_seg()


UINT GetApplicationIcon(BYTE nID)
{
	return (nID<LFApplicationCount) ? IDI_APP_DEFAULT+nID : IDI_APP_DEFAULT;
}
