
// LFDropTarget.cpp: Implementierung der Klasse LFDropTarget
//

#include "StdAfx.h"
#include "LFCommDlg.h"


// LFDropTarget
//

LFDropTarget::LFDropTarget()
{
	p_Owner = NULL;
	m_StoreIDValid = m_AllowChooseStore = m_SkipTemplate = m_IsDragging = FALSE;
	p_Filter = NULL;
	p_SearchResult = NULL;
}

void LFDropTarget::SetDragging(BOOL IsDragging)
{
	m_IsDragging = IsDragging;
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

void LFDropTarget::SetSearchResult(LFSearchResult* pSearchResult)
{
	p_SearchResult = pSearchResult;
}

__forceinline HRESULT LFDropTarget::ImportFromFS(HGLOBAL hgDrop, DWORD dwEffect, CHAR* StoreID, CWnd* pWnd)
{
	HDROP hDrop = (HDROP)GlobalLock(hgDrop);
	LFFileImportList* il = LFAllocFileImportList(hDrop);
	GlobalUnlock(hgDrop);

	// Template f�llen
	BOOL DoImport = TRUE;
	LFItemDescriptor* it = NULL;
	if (!m_SkipTemplate)
	{
		it = LFAllocItemDescriptor();

		LFItemTemplateDlg dlg(pWnd, it, StoreID, m_AllowChooseStore, p_Filter);
		switch (dlg.DoModal())
		{
		case IDCANCEL:
			DoImport = FALSE;
			break;
		case IDOK:
			strcpy_s(StoreID, LFKeySize, dlg.m_StoreID);
		}
	}

	// Import
	UINT res = LFOk;
	if (DoImport)
	{
		LFTransactionImport(StoreID, il, it, true, (dwEffect & DROPEFFECT_MOVE)!=0);
		res = il->m_ItemCount;
		LFErrorBox(res, pWnd->GetSafeHwnd());
	}

	LFFreeItemDescriptor(it);
	LFFreeFileImportList(il);

	if ((p_Owner) && (DoImport))
		p_Owner->SendMessage(LFGetMessageIDs()->ItemsDropped, NULL, NULL);

	return (res==LFOk) ? S_OK : E_INVALIDARG;
}

__forceinline HRESULT LFDropTarget::ImportFromStore(HGLOBAL hgLiquid, DWORD dwEffect, CHAR* StoreID, CWnd* pWnd)
{
	HLIQUID hLiquid = (HLIQUID)GlobalLock(hgLiquid);
	LFFileIDList* il = LFAllocFileIDList(hLiquid);
	GlobalUnlock(hgLiquid);

	LFTransactionImport(StoreID, il, (dwEffect & DROPEFFECT_MOVE)!=0);
	UINT res = il->m_LastError;
	LFErrorBox(res, pWnd->GetSafeHwnd());
	LFFreeFileIDList(il);

	if (p_Owner)
		p_Owner->SendMessage(LFGetMessageIDs()->ItemsDropped, NULL, NULL);

	return (res==LFOk) ? S_OK : E_INVALIDARG;
}

__forceinline HRESULT LFDropTarget::AddToClipboard(HGLOBAL hgLiquid, CWnd* pWnd)
{
	if (!hgLiquid)
		return E_INVALIDARG;

	HLIQUID hLiquid = (HLIQUID)GlobalLock(hgLiquid);
	LFFileIDList* il = LFAllocFileIDList(hLiquid);
	GlobalUnlock(hgLiquid);

	LFTransactionAddToSearchResult(il, p_SearchResult);
	UINT res = il->m_LastError;
	LFErrorBox(res, pWnd->GetSafeHwnd());
	LFFreeFileIDList(il);

	if (p_Owner)
		p_Owner->SendMessage(LFGetMessageIDs()->ItemsDropped, NULL, NULL);

	return (res==LFOk) ? S_OK : E_INVALIDARG;
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

	if (dobj.GetGlobalData(((LFApplication*)AfxGetApp())->CF_HLIQUID))
		goto Allowed;
	if ((!p_SearchResult) && (dobj.GetGlobalData(CF_HDROP)))
		goto Allowed;

	*pdwEffect = DROPEFFECT_NONE;
	return E_INVALIDARG;

Allowed:
	return DragOver(grfKeyState, pt, pdwEffect);
}

STDMETHODIMP LFDropTarget::DragOver(DWORD grfKeyState, POINTL /*pt*/, DWORD* pdwEffect)
{
	*pdwEffect &= m_IsDragging ? DROPEFFECT_NONE : p_SearchResult ? DROPEFFECT_COPY : (grfKeyState & MK_CONTROL) ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
	return S_OK;
}

STDMETHODIMP LFDropTarget::DragLeave()
{
	return S_OK;
}

STDMETHODIMP LFDropTarget::Drop(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	if ((DragOver(grfKeyState, pt, pdwEffect)!=S_OK) || (m_IsDragging))
		return E_INVALIDARG;

	// Data object
	COleDataObject dobj;
	dobj.Attach(pDataObject, FALSE);

	HGLOBAL hgDrop = dobj.GetGlobalData(CF_HDROP);
	HGLOBAL hgLiquid = dobj.GetGlobalData(((LFApplication*)AfxGetApp())->CF_HLIQUID);

	if ((hgDrop==NULL) && (hgLiquid==NULL))
		return E_INVALIDARG;

	// Fenster
	CWnd* pWnd = p_Owner ? p_Owner : CWnd::GetForegroundWindow();

	// Clipboard
	if (p_SearchResult)
		return AddToClipboard(hgLiquid, pWnd);

	// Ziel-Store
	CHAR StoreID[LFKeySize];
	strcpy_s(StoreID, LFKeySize, p_Filter ? p_Filter->StoreID : m_StoreIDValid ? m_StoreID : "");

	// Wenn Default-Store gew�nscht: verf�gbar ?
	if (StoreID[0]=='\0')
		if (!LFDefaultStoreAvailable())
		{
			LFErrorBox(LFNoDefaultStore, pWnd->GetSafeHwnd());
			return E_INVALIDARG;
		}

	return hgLiquid ? ImportFromStore(hgLiquid, *pdwEffect, StoreID, pWnd) : ImportFromFS(hgDrop, *pdwEffect, StoreID, pWnd);
}
