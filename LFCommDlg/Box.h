
// Box: Schnittstelle der Klasse Box
//

#pragma once
#include "LFCore.h"


// Box
//

class Box
{
public:
	Box();

	BOOL CheckForBox();
	BOOL IsBoxAvailable() const;

	WCHAR m_Path[MAX_PATH];

private:
	void Reset();
};


inline BOOL Box::CheckForBox()
{
	return LFGetBoxPath(m_Path);
}

inline BOOL Box::IsBoxAvailable() const
{
	return (m_Path[0]!=L'\0');
}

inline void Box::Reset()
{
	m_Path[0] = L'\0';
}
