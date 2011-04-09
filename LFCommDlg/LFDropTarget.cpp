
// LFDropTarget.cpp: Implementierung der Klasse LFDropTarget
//

#include "StdAfx.h"
#include "LFCommDlg.h"
#include <iostream>


// LFDropTarget
//

LFDropTarget::LFDropTarget()
{
	m_SkipTemplate = FALSE;
}

BOOL LFDropTarget::Register(CWnd* pWnd, CHAR* StoreID, BOOL AllowChooseStore)
{
	strcpy_s(m_StoreID, LFKeySize, StoreID);
	m_AllowChooseStore = AllowChooseStore;

	return (m_hWnd!=pWnd->m_hWnd) ? COleDropTarget::Register(pWnd) : TRUE;
}

DROPEFFECT LFDropTarget::OnDragEnter(CWnd* /*pWnd*/, COleDataObject* /*pDataObject*/, DWORD dwKeyState, CPoint /*point*/)
{
	m_SkipTemplate = (dwKeyState & MK_SHIFT);

	return (dwKeyState & MK_CONTROL) ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
}

DROPEFFECT LFDropTarget::OnDragOver(CWnd* /*pWnd*/, COleDataObject* /*pDataObject*/, DWORD dwKeyState, CPoint /*point*/)
{
	m_SkipTemplate = (dwKeyState & MK_SHIFT);

	return (dwKeyState & MK_CONTROL) ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
}

BOOL LFDropTarget::OnDrop(CWnd* /*pWnd*/, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint /*point*/)
{
	// Wenn Default-Store gewünscht: verfügbar ?
	if (m_StoreID[0]=='\0')
		if (!LFDefaultStoreAvailable())
		{
			LFErrorBox(LFNoDefaultStore);
			return FALSE;
		}

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
	CHAR StoreID[LFKeySize];
	strcpy_s(StoreID, LFKeySize, m_StoreID);

	LFItemDescriptor* it = NULL;
	if (!m_SkipTemplate)
	{
		it = LFAllocItemDescriptor();

		LFItemTemplateDlg dlg(CWnd::FromHandle(m_hWnd), it, StoreID, m_AllowChooseStore);
		switch (dlg.DoModal())
		{
		case IDCANCEL:
			GlobalUnlock(hG);
			return FALSE;
		case IDOK:
			strcpy_s(StoreID, LFKeySize, dlg.m_StoreID);
		}
	}

	// Dateien durchlaufen
	BOOL success = TRUE;
	UINT uNumFiles = DragQueryFile(hDrop, (UINT)-1, NULL, 0);
	WCHAR szNextFile[MAX_PATH];

	LFFileImportList* il = LFAllocFileImportList();
	for (UINT uFile=0; uFile<uNumFiles; uFile++)
		if (DragQueryFile(hDrop, uFile, szNextFile, MAX_PATH))
			if (!LFAddImportPath(il, &szNextFile[0]))
				success = FALSE;

	GlobalUnlock(hG);

	// Import
	if (success)
	{
		UINT res = LFImportFiles(StoreID, il, it, dropEffect==DROPEFFECT_MOVE);
		if (res!=LFOk)
		{
			LFErrorBox(res);
			success = FALSE;
		}

		SendMessage(m_hWnd, LFGetMessageIDs()->ItemsDropped, NULL, NULL);
	}

	LFFreeItemDescriptor(it);
	LFFreeFileImportList(il);
	return success;
}
