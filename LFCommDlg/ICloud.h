
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

	WCHAR m_Path[MAX_PATH];

private:
	void Reset();
};


inline BOOL ICloud::CheckForICloud()
{
	return LFGetICloudPath(m_Path);
}

inline BOOL ICloud::IsICloudAvailable() const
{
	return (m_Path[0]!=L'\0');
}

inline void ICloud::Reset()
{
	m_Path[0] = L'\0';
}
