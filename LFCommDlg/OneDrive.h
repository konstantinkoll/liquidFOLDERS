
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

	LFOneDrivePaths m_OneDriveData;

private:
	void Reset();
};


inline BOOL OneDrive::CheckForOneDrive()
{
	return LFGetOneDrivePaths(m_OneDriveData);
}

inline BOOL OneDrive::IsOneDriveAvailable() const
{
	return (m_OneDriveData.OneDrive[0]!=L'\0');
}

inline void OneDrive::Reset()
{
	ZeroMemory(&m_OneDriveData, sizeof(m_OneDriveData));
}
