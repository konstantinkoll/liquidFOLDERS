#include "StdAfx.h"
#include "LFCommDlg.h"
#include <iostream>


LFDropTarget::LFDropTarget()
{
}

LFDropTarget::~LFDropTarget()
{
}

BOOL LFDropTarget::Register(CWnd* pWnd)
{
	if (m_hWnd!=pWnd->m_hWnd)
	{
		return COleDropTarget::Register(pWnd);
	}
	else
	{
		return TRUE;
	}
}

DROPEFFECT LFDropTarget::OnDragEnter(CWnd* /*pWnd*/, COleDataObject* /*pDataObject*/, DWORD /*dwKeyState*/, CPoint /*point*/)
{
	return DROPEFFECT_COPY;
}

DROPEFFECT LFDropTarget::OnDragOver(CWnd* /*pWnd*/, COleDataObject* /*pDataObject*/, DWORD dwKeyState, CPoint /*point*/)
{
	return ((MK_SHIFT | MK_CONTROL) & dwKeyState) ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
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
	LFItemDescriptor* it = LFAllocItemDescriptor();

	LFItemTemplateDlg dlg(CWnd::FromHandle(m_hWnd), it);
	if (dlg.DoModal()!=IDOK)
	{
		GlobalUnlock(hG);
		return FALSE;
	}
	LFFreeItemDescriptor(it);

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
	return success;
}

void LFDropTarget::OnDragLeave (CWnd* /*pWnd*/)
{
}
