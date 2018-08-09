
// LFDropSource.cpp: Implementierung der Klasse LFDropSource
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFDropSource
//

#define DDWM_SETCURSOR        WM_USER+2
#define DDWM_UPDATEWINDOW     WM_USER+3

LFDropSource::LFDropSource(LFDataSource* pDataSource)
	: COleDropSource()
{
	ASSERT(pDataSource);

	p_DataObject = pDataSource->GetDataObject();
	m_SetCursor = TRUE;
}

BOOL LFDropSource::GetGlobalData(LPCWSTR lpszFormat, FORMATETC& FormatEtc, STGMEDIUM& StgMedium) const
{
	ASSERT(p_DataObject);
	ASSERT(lpszFormat);

	FormatEtc.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(lpszFormat);
	FormatEtc.ptd = NULL;
	FormatEtc.dwAspect = DVASPECT_CONTENT;
	FormatEtc.lindex = -1;
	FormatEtc.tymed = TYMED_HGLOBAL;

	if (SUCCEEDED(p_DataObject->QueryGetData(&FormatEtc)))
		if (SUCCEEDED(p_DataObject->GetData(&FormatEtc, &StgMedium)))
		{
			if (StgMedium.tymed==TYMED_HGLOBAL)
				return TRUE;

			ReleaseStgMedium(&StgMedium);
		}

	return FALSE;
}

DWORD LFDropSource::GetGlobalDataDWord(LPCWSTR lpszFormat) const
{
	FORMATETC FormatEtc;
	STGMEDIUM StgMedium;

	DWORD dwData = 0;

	if (GetGlobalData(lpszFormat, FormatEtc, StgMedium))
	{
		ASSERT(GlobalSize(StgMedium.hGlobal)>=sizeof(DWORD));

		dwData = *((LPDWORD)(GlobalLock(StgMedium.hGlobal)));
		GlobalUnlock(StgMedium.hGlobal);

		ReleaseStgMedium(&StgMedium);
	}

	return dwData;
}

BOOL LFDropSource::SetDragImageCursor(DROPEFFECT DropEffect) const
{
	// At least Windows Vista?
	if (LFGetApp()->OSVersion<OS_Vista)
		return FALSE;

	// Is showing layered?
	if (!IsCtrlThemed() || !GetGlobalDataDWord(_T("IsShowingLayered")))
		return FALSE;

	// Drag window handle
	HWND hWnd = (HWND)ULongToHandle(GetGlobalDataDWord(_T("DragWindow")));
	if (!hWnd)
		return FALSE;

	// Set cursor
	WPARAM wParam;

	switch (DropEffect & ~DROPEFFECT_SCROLL)
	{
	case DROPEFFECT_NONE:
		wParam = 1;
		break;

	case DROPEFFECT_COPY:
		wParam = 3;
		break;

	case DROPEFFECT_MOVE:
		wParam = 2;
		break;

	case DROPEFFECT_LINK:
		wParam = 4;
		break;

	default:
		wParam = 0;
	}

	// Undocumented window message
	SendMessage(hWnd, DDWM_SETCURSOR, wParam, 0);

	return TRUE;
}

SCODE LFDropSource::QueryContinueDrag(BOOL bEscapePressed, DWORD dwKeyState)
{
	if (bEscapePressed)
		return DRAGDROP_S_CANCEL;

	if ((dwKeyState & MK_LBUTTON)==0)
		return DRAGDROP_S_DROP;

	return S_OK;
}

SCODE LFDropSource::GiveFeedback(DROPEFFECT DropEffect)
{
	if (SetDragImageCursor(DropEffect))
	{
		// Set default arrow cursor if neccessary
		if (m_SetCursor)
		{
			SetCursor((HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_NORMAL), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));

			m_SetCursor = FALSE;
		}

		return S_OK;
	}

	// When the old style drag cursor is used, the default cursor must be set the next time.
	// If the new style cursor is actually shown, the cursor has been set above.
	m_SetCursor = TRUE;

	return DRAGDROP_S_USEDEFAULTCURSORS;
}
