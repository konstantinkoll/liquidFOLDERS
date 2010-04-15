#include "StdAfx.h"
#include "LFDropTarget.h"

#include <iostream>

CLFDropTarget::CLFDropTarget(CDialog* parent)
:m_pParent(parent)
{
}

CLFDropTarget::~CLFDropTarget(void)
{
}

DROPEFFECT CLFDropTarget::OnDragEnter ( CWnd* pWnd, COleDataObject* pDataObject,
						DWORD dwKeyState, CPoint point ) 
{
	return DROPEFFECT_MOVE;
}

DROPEFFECT CLFDropTarget::OnDragOver ( CWnd* pWnd, COleDataObject* pDataObject,
					   DWORD dwKeyState, CPoint point ) 
{
    if (MK_SHIFT && dwKeyState) return DROPEFFECT_MOVE;
    if (MK_CONTROL && dwKeyState) return DROPEFFECT_COPY;
    if (MK_SHIFT & MK_CONTROL && dwKeyState) return DROPEFFECT_LINK;

	return DROPEFFECT_MOVE;
}


BOOL CLFDropTarget::OnDrop ( CWnd* pWnd, COleDataObject* pDataObject,
							DROPEFFECT dropEffect, CPoint point )
{
	HGLOBAL hg;
	HDROP   hdrop;
	UINT    uNumFiles;
	TCHAR   szNextFile [MAX_PATH];
	BOOL	success = false;

	// Get the HDROP data from the data object.

	hg = pDataObject->GetGlobalData ( CF_HDROP );

	if ( NULL == hg )
		return FALSE;

	hdrop = (HDROP) GlobalLock ( hg );

	if ( NULL == hdrop )
	{
		GlobalUnlock ( hg );
		return FALSE;
	}

    uNumFiles = DragQueryFile ( hdrop, -1, NULL, 0 );

	// Read in the list of files here...
    for ( UINT uFile = 0; uFile < uNumFiles; uFile++ ) {
        // Get the next filename from the HDROP info.

        if ( DragQueryFile ( hdrop, uFile, szNextFile, MAX_PATH ) > 0 ) {
			//Hier steht die Datei oder das Verzeichnis in szNextFile

			success = true;
		}
	}

	GlobalUnlock ( hg );

	return success;
}


void CLFDropTarget::OnDragLeave ( CWnd* pWnd ) {
}
