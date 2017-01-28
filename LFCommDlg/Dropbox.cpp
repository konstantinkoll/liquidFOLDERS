
// Dropbox.cpp: Implementierung der Klasse Dropbox
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "Dropbox.h"
#include <shlobj.h>


// Dropbox
//

Dropbox::Dropbox()
{
	Reset();
}

BOOL Dropbox::CheckForDropbox()
{
	Reset();

	// Dropbox only runs on Windows Vista or newer
	if (LFGetApp()->OSVersion<OS_Vista)
		return FALSE;

	// Scan for configuration file
	WCHAR Path[MAX_PATH];

	// Roaming
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, Path)))
		LoadDropboxSettings(Path);

	// Local
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, Path)))
		LoadDropboxSettings(Path);

	return IsDropboxAvailable();
}

BOOL Dropbox::Compare(LPCSTR Buffer, jsmntok_t& Token, LPCSTR Str)
{
	return (Token.type==JSMN_STRING) && ((INT)strlen(Str)==Token.end-Token.start) && (strncmp(Buffer+Token.start, Str, Token.end-Token.start)==0);
}

void Dropbox::UnescapeString(LPWSTR Dst, LPCSTR Src, SIZE_T Size)
{
	ASSERT(Dst);
	ASSERT(Src);

	if (Size>MAX_PATH-1)
		Size = MAX_PATH-1;

	BOOL InEscape = FALSE;

	// Copy Size CHARs from Src to Dst, and unescape the string
	while (Size>0)
	{
		if (InEscape)
		{
			if ((*Src=='\\') || (*Src=='"'))
				*(Dst++) = *Src;

			InEscape = FALSE;
		}
		else
		{
			if (*Src=='\\')
			{
				InEscape = TRUE;
			}
			else
			{
				*(Dst++) = *Src;
			}
		}

		Src++;
		Size--;
	}

	*Dst = L'\0';
}

BOOL Dropbox::ReadFile(LPWSTR Path, LPSTR Buffer, SIZE_T MaxSize, SIZE_T& Size)
{
	ASSERT(Path);

	BOOL Result = FALSE;

	HANDLE hFile = CreateFile(Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER FileSize;
		if (GetFileSizeEx(hFile, &FileSize))
			if ((FileSize.QuadPart>4) && (FileSize.QuadPart<(LONGLONG)MaxSize))
			{
				Size = (SIZE_T)FileSize.LowPart;

				DWORD wmRead;
				if (::ReadFile(hFile, Buffer, FileSize.LowPart, &wmRead, NULL))
					Result = (wmRead==FileSize.LowPart);
			}

		CloseHandle(hFile);
	}

	return Result;
}

void Dropbox::ExtractPath(LPWSTR Path, LPWSTR SubscriptionType, LPCSTR Buffer, jsmntok_t* pToken)
{
	ASSERT(Path);
	ASSERT(SubscriptionType);
	ASSERT(Buffer);
	ASSERT(pToken);

	if (pToken[0].type==JSMN_OBJECT)
		for (UINT b=1; b<=(UINT)pToken[0].size*2; b++)
			if (b & 1)
			{
				if (Compare(Buffer, pToken[b], "path"))
					UnescapeString(Path, &Buffer[pToken[b+1].start], pToken[b+1].end-pToken[b+1].start);

				if (Compare(Buffer, pToken[b], "subscription_type"))
					UnescapeString(SubscriptionType, &Buffer[pToken[b+1].start], pToken[b+1].end-pToken[b+1].start);
			}
}

void Dropbox::LoadDropboxSettings(LPWSTR Path)
{
	ASSERT(Path);

	// Read file
	wcscat_s(Path, MAX_PATH, L"\\Dropbox\\info.json");

	CHAR Buffer[16384];
	SIZE_T Size;
	if (!ReadFile(Path, Buffer, sizeof(Buffer), Size))
		return;

	// Parse
	jsmn_parser Parser;
	jsmn_init(&Parser);

	jsmntok_t Token[256];
	INT cToken = jsmn_parse(&Parser, Buffer, Size, Token, 256);
	if ((cToken<1) || (Token[0].type!=JSMN_OBJECT))
		return;

	// Extract Dropbox paths
	for (UINT a=1; a<(UINT)cToken; a++)
	{
		if (Compare(Buffer, Token[a], "personal"))
			ExtractPath(m_DropboxData.Paths[0], m_DropboxData.SubscriptionTypes[0], Buffer, &Token[a+1]);

		if (Compare(Buffer, Token[a], "business"))
			ExtractPath(m_DropboxData.Paths[1], m_DropboxData.SubscriptionTypes[1], Buffer, &Token[a+1]);
	}
}
