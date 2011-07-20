
// LFDropTarget.cpp: Implementierung der Klasse LFDropTarget
//

#include "StdAfx.h"
#include "LFCommDlg.h"
#include <iostream>


// LFDropTarget
//

LFDropTarget::LFDropTarget()
{
	m_StoreIDValid = m_AllowChooseStore = m_SkipTemplate = FALSE;
	p_Filter = NULL;
}

BOOL LFDropTarget::Register(CWnd* pWnd, CHAR* StoreID, BOOL AllowChooseStore)
{
	p_Filter = NULL;
	strcpy_s(m_StoreID, LFKeySize, StoreID);
	m_StoreIDValid = TRUE;
	m_AllowChooseStore = AllowChooseStore;

	return (m_hWnd!=pWnd->m_hWnd) ? COleDropTarget::Register(pWnd) : TRUE;
}

BOOL LFDropTarget::Register(CWnd* pWnd, LFFilter* pFilter, BOOL AllowChooseStore)
{
	p_Filter = pFilter;
	m_StoreIDValid = FALSE;
	m_AllowChooseStore = AllowChooseStore;

	return (m_hWnd!=pWnd->m_hWnd) ? COleDropTarget::Register(pWnd) : TRUE;
}

DROPEFFECT LFDropTarget::CheckDrop(COleDataObject* pDataObject, DWORD dwKeyState)
{
	m_SkipTemplate = (dwKeyState & MK_SHIFT);

	if ((pDataObject->GetGlobalData(CF_HDROP)==NULL) && pDataObject->GetGlobalData(((LFApplication*)AfxGetApp())->CF_HLIQUID)==NULL)
		return DROPEFFECT_NONE;

	return (dwKeyState & MK_CONTROL) ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
}

DROPEFFECT LFDropTarget::OnDragEnter(CWnd* /*pWnd*/, COleDataObject* pDataObject, DWORD dwKeyState, CPoint /*point*/)
{
	return CheckDrop(pDataObject, dwKeyState);
}

DROPEFFECT LFDropTarget::OnDragOver(CWnd* /*pWnd*/, COleDataObject* pDataObject, DWORD dwKeyState, CPoint /*point*/)
{
	return CheckDrop(pDataObject, dwKeyState);
}

BOOL LFDropTarget::OnDrop(CWnd* /*pWnd*/, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint /*point*/)
{
	CHAR StoreID[LFKeySize];
	strcpy_s(StoreID, LFKeySize, p_Filter ? p_Filter->StoreID : m_StoreIDValid ? m_StoreID : "");

	// Wenn Default-Store gewünscht: verfügbar ?
	if (StoreID[0]=='\0')
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
	LFFileImportList* il = LFAllocFileImportList(hDrop);
	GlobalUnlock(hG);

	if (!il->m_ItemCount)
	{
		LFFreeFileImportList(il);
		return FALSE;
	}

	BOOL Success = TRUE;

	// Template füllen
	LFItemDescriptor* it = NULL;
	if (!m_SkipTemplate)
	{
		it = LFAllocItemDescriptor();

		LFItemTemplateDlg dlg(CWnd::FromHandle(m_hWnd), it, StoreID, m_AllowChooseStore, p_Filter);
		switch (dlg.DoModal())
		{
		case IDCANCEL:
			Success = FALSE;
			goto Finish;
		case IDOK:
			strcpy_s(StoreID, LFKeySize, dlg.m_StoreID);
		}
	}

	// Import
	UINT res = LFImportFiles(StoreID, il, it, true, dropEffect==DROPEFFECT_MOVE);
	if (res!=LFOk)
	{
		LFErrorBox(res);
		Success = FALSE;
	}

	SendMessage(m_hWnd, LFGetMessageIDs()->ItemsDropped, NULL, NULL);

Finish:
	LFFreeItemDescriptor(it);
	LFFreeFileImportList(il);
	return Success;
}
