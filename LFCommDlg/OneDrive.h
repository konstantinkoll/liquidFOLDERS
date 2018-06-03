
// OneDrive: Schnittstelle der Klasse OneDrive
//

#pragma once
#include "LFCore.h"


// OneDrive
//

class OneDrive
{
public:
	OneDrive();

	BOOL CheckForOneDrive();
	BOOL IsOneDriveAvailable() const;

	LFOneDrivePaths m_OneDrivePaths;

private:
	void Reset();
};


inline BOOL OneDrive::CheckForOneDrive()
{
	return LFGetOneDrivePaths(m_OneDrivePaths);
}

inline BOOL OneDrive::IsOneDriveAvailable() const
{
	return (m_OneDrivePaths.OneDrive[0]!=L'\0');
}

inline void OneDrive::Reset()
{
	ZeroMemory(&m_OneDrivePaths, sizeof(m_OneDrivePaths));
}
