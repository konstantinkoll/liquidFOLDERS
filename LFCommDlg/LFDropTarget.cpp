#include "StdAfx.h"
#include "LFCommDlg.h"
#include <iostream>


LFDropTarget::LFDropTarget()
{
	SkipTemplate = FALSE;
}

LFDropTarget::~LFDropTarget()
{
}

BOOL LFDropTarget::Register(CWnd* pWnd)
{
	return (m_hWnd!=pWnd->m_hWnd) ? COleDropTarget::Register(pWnd) : TRUE;
}

DROPEFFECT LFDropTarget::OnDragEnter(CWnd* /*pWnd*/, COleDataObject* /*pDataObject*/, DWORD /*dwKeyState*/, CPoint /*point*/)
{
	return DROPEFFECT_COPY;
}

DROPEFFECT LFDropTarget::OnDragOver(CWnd* /*pWnd*/, COleDataObject* /*pDataObject*/, DWORD dwKeyState, CPoint /*point*/)
{
	SkipTemplate = (dwKeyState & MK_SHIFT);

	return (dwKeyState & MK_CONTROL) ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
}

BOOL LFDropTarget::OnDrop(CWnd* /*pWnd*/, COleDataObject* pDataObject, DROPEFFECT /*dropEffect*/, CPoint /*point*/)
{
	// Store verfügbar ?
	if (!LFDefaultStoreAvailable())
	{
		LFErrorBox(LFNoDefaultStore);
		return FALSE;
	}

	// TODO: internes Format holen

	// HDROP holen
	HGLOBAL hG = pDataObject->GetGlobalData(CF_HDROP);
	if (!hG)
		return FALSE;

	HDROP hDrop = (HDROP)GlobalLock(hG);
	if (!hDrop)
	{
		GlobalUnlock(hG);
		return FALSE;
	}

	// Template füllen
	LFItemDescriptor* it = NULL;
	if (!SkipTemplate)
	{
		it = LFAllocItemDescriptor();

		LFItemTemplateDlg dlg(CWnd::FromHandle(m_hWnd), it);
		if (dlg.DoModal()!=IDOK)
		{
			GlobalUnlock(hG);
			return FALSE;
		}
	}

	// Dateien durchlaufen
	BOOL success = FALSE;
	UINT uNumFiles = DragQueryFile(hDrop, (UINT)-1, NULL, 0);
	TCHAR szNextFile [MAX_PATH];

	for (UINT uFile=0; uFile<uNumFiles; uFile++)
		if (DragQueryFile(hDrop, uFile, szNextFile, MAX_PATH )>0)
		{
			// TODO
			success = TRUE;
		}

	GlobalUnlock(hG);
	LFFreeItemDescriptor(it);
	return success;
}

void LFDropTarget::OnDragLeave (CWnd* /*pWnd*/)
{
}
