
#pragma once
#include "LF.h"


DWORD_PTR UpdateProgress(LFProgress* pProgress);
BOOL ProgressMajorNext(LFProgress* pProgress);
BOOL ProgressMinorStart(LFProgress* pProgress, UINT MinorCount, LPCWSTR pObject=L"", BOOL NoMinorCounter=FALSE);
BOOL ProgressMinorNext(LFProgress* pProgress);
BOOL ProgressMinorNext(LFProgress* pProgress, BOOL Error);
BOOL ProgressMinorNext(LFProgress* pProgress, BOOL Error, LPCWSTR pObject);
BOOL SetProgressError(LFProgress* pProgress, BOOL Error);
BOOL SetProgressObject(LFProgress* pProgress, LPCWSTR pObject=L"");

inline BOOL AbortProgress(LFProgress* pProgress)
{
	return pProgress ? pProgress->UserAbort : FALSE;
}

inline void ProgressMajorStart(LFProgress* pProgress, UINT MajorCount)
{
	if (pProgress)
		pProgress->MajorCount = MajorCount;
}

inline void ProgressMinorSkip(LFProgress* pProgress)
{
	if (pProgress)
	{
		pProgress->Object[0] = L'\0';
		pProgress->MinorCurrent++;
	}
}

inline BOOL ProgressMinorNext(LFProgress* pProgress, UINT Result)
{
	return ProgressMinorNext(pProgress, Result>LFCancel);
}

inline BOOL ProgressMinorNext(LFProgress* pProgress, UINT Result, LPCWSTR pObject)
{
	return ProgressMinorNext(pProgress, Result>LFCancel, pObject);
}

inline BOOL SetProgressError(LFProgress* pProgress, UINT Result)
{
	return SetProgressError(pProgress, Result>LFCancel);
}
