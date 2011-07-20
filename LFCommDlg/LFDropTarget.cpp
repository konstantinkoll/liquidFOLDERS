
// LFDropTarget.cpp: Implementierung der Klasse LFDropTarget
//

#include "StdAfx.h"
#include "LFCommDlg.h"


// LFDropTarget
//

LFDropTarget::LFDropTarget()
{
	p_Owner = NULL;
	m_StoreIDValid = m_AllowChooseStore = m_SkipTemplate = FALSE;
	p_Filter = NULL;
}

void LFDropTarget::SetOwner(CWnd* pOwner)
{
	p_Owner = pOwner;
}

void LFDropTarget::SetStore(CHAR* StoreID, BOOL AllowChooseStore)
{
	p_Filter = NULL;
	strcpy_s(m_StoreID, LFKeySize, StoreID);
	m_StoreIDValid = TRUE;
	m_AllowChooseStore = AllowChooseStore;
}

void LFDropTarget::SetFilter(LFFilter* pFilter, BOOL AllowChooseStore)
{
	p_Filter = pFilter;
	m_StoreIDValid = FALSE;
	m_AllowChooseStore = AllowChooseStore;
}


STDMETHODIMP LFDropTarget::QueryInterface(REFIID iid, void** ppvObject)
{
	if ((iid==IID_IDropTarget) || (iid==IID_IUnknown))
	{
		AddRef();
		*ppvObject = this;
		return S_OK;
	}

	*ppvObject = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) STDMETHODCALLTYPE LFDropTarget::AddRef()
{
	return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(ULONG) STDMETHODCALLTYPE LFDropTarget::Release()
{
	LONG Count = InterlockedDecrement(&m_lRefCount);
	if (!Count)
	{
		delete this;
		return 0;
	}

	return Count;
}

STDMETHODIMP LFDropTarget::DragEnter(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	m_SkipTemplate = (grfKeyState & MK_SHIFT);

	COleDataObject dobj;
	dobj.Attach(pDataObject, FALSE);

	if ((dobj.GetGlobalData(CF_HDROP)==NULL) && (dobj.GetGlobalData(((LFApplication*)AfxGetApp())->CF_HLIQUID)==NULL))
	{
		*pdwEffect = DROPEFFECT_NONE;
		return E_INVALIDARG;
	}

	return DragOver(grfKeyState, pt, pdwEffect);
}

STDMETHODIMP LFDropTarget::DragOver(DWORD grfKeyState, POINTL /*pt*/, DWORD* pdwEffect)
{
	*pdwEffect &= (grfKeyState & MK_CONTROL) ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
	return S_OK;
}

STDMETHODIMP LFDropTarget::DragLeave()
{
	return S_OK;
}

STDMETHODIMP LFDropTarget::Drop(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	DragOver(grfKeyState, pt, pdwEffect);

	CHAR StoreID[LFKeySize];
	strcpy_s(StoreID, LFKeySize, p_Filter ? p_Filter->StoreID : m_StoreIDValid ? m_StoreID : "");

	// Wenn Default-Store gewünscht: verfügbar ?
	if (StoreID[0]=='\0')
		if (!LFDefaultStoreAvailable())
		{
			LFErrorBox(LFNoDefaultStore);
			return E_INVALIDARG;
		}

	// HDROP holen
	COleDataObject dobj;
	dobj.Attach(pDataObject, FALSE);

	HGLOBAL hG = dobj.GetGlobalData(CF_HDROP);
	if (!hG)
		return E_INVALIDARG;

	HDROP hDrop = (HDROP)GlobalLock(hG);
	LFFileImportList* il = LFAllocFileImportList(hDrop);
	GlobalUnlock(hG);

	if (!il->m_ItemCount)
	{
		LFFreeFileImportList(il);
		return E_INVALIDARG;
	}

	BOOL Success = TRUE;

	// Template füllen
	LFItemDescriptor* it = NULL;
	if (!m_SkipTemplate)
	{
		it = LFAllocItemDescriptor();

		LFItemTemplateDlg dlg(p_Owner ? p_Owner : CWnd::GetForegroundWindow(), it, StoreID, m_AllowChooseStore, p_Filter);
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
	UINT res = LFImportFiles(StoreID, il, it, true, (*pdwEffect & DROPEFFECT_MOVE)!=0);
	if (res!=LFOk)
	{
		LFErrorBox(res);
		Success = FALSE;
	}

	if (p_Owner)
		p_Owner->SendMessage(LFGetMessageIDs()->ItemsDropped, NULL, NULL);

Finish:
	LFFreeItemDescriptor(it);
	LFFreeFileImportList(il);
	return S_OK;
}
