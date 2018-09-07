
#include "stdafx.h"
#include "Progress.h"


extern LFMessageIDs LFMessages;


DWORD_PTR UpdateProgress(LFProgress* pProgress)
{
	assert(pProgress);

	DWORD_PTR Result;
	return SendMessageTimeout(pProgress->hWnd, LFMessages.UpdateProgress, (WPARAM)pProgress, NULL, SMTO_ABORTIFHUNG, 5000, &Result) ? Result : TRUE;
}

BOOL ProgressMajorNext(LFProgress* pProgress)
{
	if (!pProgress)
		return FALSE;
	
	pProgress->MajorCurrent++;

	return pProgress->UserAbort;
}

BOOL ProgressMinorStart(LFProgress* pProgress, UINT MinorCount, LPCWSTR pObject, BOOL NoMinorCounter)
{
	assert(pObject);

	if (!pProgress)
		return FALSE;

	wcscpy_s(pProgress->Object, 256, pObject);
	pProgress->MinorCount = MinorCount;
	pProgress->MinorCurrent = 0;
	pProgress->NoMinorCounter = NoMinorCounter;

	return UpdateProgress(pProgress)!=0;
}

BOOL ProgressMinorNext(LFProgress* pProgress)
{
	if (!pProgress)
		return FALSE;

	pProgress->MinorCurrent++;

	return UpdateProgress(pProgress)!=0;
}

BOOL ProgressMinorNext(LFProgress* pProgress, BOOL Error)
{
	if (!pProgress)
		return FALSE;

	pProgress->MinorCurrent++;

	if (Error)
		pProgress->ProgressState = LFProgressError;

	return UpdateProgress(pProgress)!=0;
}

BOOL ProgressMinorNext(LFProgress* pProgress, BOOL Error, LPCWSTR pObject)
{
	assert(pObject);

	if (!pProgress)
		return FALSE;

	wcscpy_s(pProgress->Object, 256, pObject);
	pProgress->MinorCurrent++;

	if (Error)
		pProgress->ProgressState = LFProgressError;

	return UpdateProgress(pProgress)!=0;
}

BOOL SetProgressError(LFProgress* pProgress, BOOL Error)
{
	if (!pProgress)
		return FALSE;

	if (Error)
		pProgress->ProgressState = LFProgressError;

	return UpdateProgress(pProgress)!=0;
}

BOOL SetProgressObject(LFProgress* pProgress, LPCWSTR pObject)
{
	assert(pObject);

	if (!pProgress)
		return FALSE;

	wcscpy_s(pProgress->Object, 256, pObject);

	return UpdateProgress(pProgress)!=0;
}
