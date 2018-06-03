
// ICloud: Schnittstelle der Klasse ICloud
//

#pragma once
#include "LFCore.h"


// ICloud
//

class ICloud
{
public:
	ICloud();

	BOOL CheckForICloud();
	BOOL IsICloudAvailable() const;

	LFICloudPaths m_iCloudPaths;

private:
	void Reset();
};


inline BOOL ICloud::CheckForICloud()
{
	return LFGetICloudPaths(m_iCloudPaths);
}

inline BOOL ICloud::IsICloudAvailable() const
{
	return (m_iCloudPaths.Drive[0]!=L'\0') || (m_iCloudPaths.PhotoLibrary[0]!=L'\0');
}

inline void ICloud::Reset()
{
	ZeroMemory(&m_iCloudPaths, sizeof(m_iCloudPaths));
}
