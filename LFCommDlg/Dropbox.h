
// Dropbox: Schnittstelle der Klasse Dropbox
//

#pragma once
#include "jsmn.h"


// Dropbox
//

struct DropboxData
{
	WCHAR Paths[2][MAX_PATH];
	WCHAR SubscriptionTypes[2][MAX_PATH];
};

class Dropbox
{
public:
	Dropbox();

	void CheckForDropbox();
	BOOL IsDropboxAvailable() const;

	DropboxData m_Data;

protected:
	void LoadDropboxSettings(LPWSTR Path);

private:
	static BOOL Compare(LPCSTR Buffer, jsmntok_t& Token, LPCSTR Str);
	static void UnescapeString(LPWSTR Dst, LPCSTR Src, SIZE_T Size);
	BOOL ReadFile(LPWSTR Path, LPSTR Buffer, SIZE_T MaxSize, SIZE_T& Size);
	void ExtractPath(LPWSTR Path, LPWSTR SubscriptionType, LPCSTR Buffer, jsmntok_t* pToken);
};

inline BOOL Dropbox::IsDropboxAvailable() const
{
	return (m_Data.Paths[0][0]!=L'\0') || (m_Data.Paths[1][0]!=L'\0');
}
