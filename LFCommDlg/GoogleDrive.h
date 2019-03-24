
// GoogleDrive: Schnittstelle der Klasse GoogleDrive
//

#pragma once
#include "LFCore.h"


// GoogleDrive
//

class GoogleDrive
{
public:
	GoogleDrive();

	BOOL CheckForGoogleDrive();
	BOOL IsGoogleDriveAvailable() const;

	WCHAR m_Path[MAX_PATH];

private:
	void Reset();
};


inline BOOL GoogleDrive::CheckForGoogleDrive()
{
	return LFGetGoogleDrivePath(m_Path);
}

inline BOOL GoogleDrive::IsGoogleDriveAvailable() const
{
	return (m_Path[0]!=L'\0');
}

inline void GoogleDrive::Reset()
{
	m_Path[0] = L'\0';
}
