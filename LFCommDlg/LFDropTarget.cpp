
// LFDropTarget.cpp: Implementierung der Klasse LFDropTarget
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFDropTarget
//

LFDropTarget::LFDropTarget()
{
	CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_IDropTargetHelper, (void**)&m_pDropTargetHelper);

	p_OwnerWnd = NULL;
	m_StoreIDValid = m_SkipTemplate = m_AllowChooseStore = m_IsDragging = FALSE;
	p_Filter = NULL;
	p_SearchResult = NULL;
}

void LFDropTarget::SetDragging(BOOL IsDragging)
{
	m_IsDragging = IsDragging;
}

void LFDropTarget::SetOwner(CWnd* pOwnerWnd)
{
	p_OwnerWnd = pOwnerWnd;
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
	WorkerParameters wp;
	ZeroMemory(&wp, sizeof(wp));
	strcpy_s(wp.StoreID, LFKeySize, StoreID);
	wp.DeleteSource = (dwEffect & DROPEFFECT_MOVE)!=0;

	// Template füllen
	if (!m_SkipTemplate)
	{
		wp.Template = LFAllocItemDescriptor();

		LFItemTemplateDlg dlg(wp.Template, wp.StoreID, pWnd, m_AllowChooseStore, p_Filter);
		if (dlg.DoModal()==IDCANCEL)
		{
			LFFreeItemDescriptor(wp.Template);
			return E_ABORT;
		}

		if (m_AllowChooseStore)
			strcpy_s(wp.StoreID, LFKeySize, dlg.m_StoreID);
	}

	HDROP hDrop = (HDROP)GlobalLock(hgDrop);
	wp.FileImportList = LFAllocFileImportList(hDrop);
	GlobalUnlock(hgDrop);

	LFDoWithProgress(WorkerImportFromWindows, &wp.Hdr, pWnd);
	UINT Result = wp.FileImportList->m_LastError;
	LFErrorBox(pWnd, Result);

	LFFreeFileImportList(wp.FileImportList);

	if (wp.Template)
		LFFreeItemDescriptor(wp.Template);

	if (p_OwnerWnd)
		p_OwnerWnd->SendMessage(LFGetMessageIDs()->ItemsDropped);

	return (Result==LFOk) ? S_OK : E_INVALIDARG;
}

__forceinline HRESULT LFDropTarget::ImportFromStore(IDataObject* pDataObject, HGLOBAL hgLiquid, DWORD dwEffect, CHAR* StoreID, CWnd* pWnd)
{
	WorkerParameters wp;
	ZeroMemory(&wp, sizeof(wp));
	strcpy_s(wp.StoreID, LFKeySize, StoreID);
	wp.DeleteSource = (dwEffect & DROPEFFECT_MOVE)!=0;

	HLIQUID hLiquid = (HLIQUID)GlobalLock(hgLiquid);
	wp.TransactionList = LFAllocTransactionList(hLiquid);
	GlobalUnlock(hgLiquid);

	LFDoWithProgress(WorkerImportFromStore, &wp.Hdr, pWnd);
	UINT Result = wp.TransactionList->m_LastError;
	LFErrorBox(pWnd, Result);

	// CF_LIQUIDFILES neu setzen, um nicht veränderte Dateien (Fehler oder Drop auf denselben Store) zu entfernen
	FORMATETC fmt;
	ZeroMemory(&fmt, sizeof(fmt));
	fmt.cfFormat = LFGetApp()->CF_HLIQUID;
	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex = -1;
	fmt.tymed = TYMED_HGLOBAL;

	STGMEDIUM stg;
	ZeroMemory(&stg, sizeof(stg));
	stg.tymed = TYMED_HGLOBAL;
	stg.hGlobal = LFCreateLiquidFiles(wp.TransactionList);

	pDataObject->SetData(&fmt, &stg, FALSE);

	LFFreeTransactionList(wp.TransactionList);

	if (p_OwnerWnd)
		p_OwnerWnd->SendMessage(LFGetMessageIDs()->ItemsDropped);

	return (Result==LFOk) ? S_OK : E_INVALIDARG;
}

__forceinline HRESULT LFDropTarget::AddToClipboard(HGLOBAL hgLiquid, CWnd* pWnd)
{
	if (!hgLiquid)
		return E_INVALIDARG;

	CWaitCursor csr;

	HLIQUID hLiquid = (HLIQUID)GlobalLock(hgLiquid);
	LFTransactionList* tl = LFAllocTransactionList(hLiquid);
	GlobalUnlock(hgLiquid);

	LFTransactionAddToSearchResult(tl, p_SearchResult);
	UINT Result = tl->m_LastError;
	LFErrorBox(pWnd, Result);
	LFFreeTransactionList(tl);

	if (p_OwnerWnd)
		p_OwnerWnd->SendMessage(LFGetMessageIDs()->ItemsDropped);

	return (Result==LFOk) ? S_OK : E_INVALIDARG;
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
	return InterlockedDecrement(&m_lRefCount);
}

STDMETHODIMP LFDropTarget::DragEnter(IDataObject* pDataObject, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
	if (m_pDropTargetHelper)
	{
		POINT pt = { ptl.x, ptl.y };
		m_pDropTargetHelper->DragEnter(p_OwnerWnd->GetSafeHwnd(), pDataObject, &pt, *pdwEffect);
	}

	COleDataObject dobj;
	dobj.Attach(pDataObject, FALSE);

	if (dobj.GetGlobalData(LFGetApp()->CF_HLIQUID))
		goto Allowed;
	if ((!p_SearchResult) && (dobj.GetGlobalData(CF_HDROP)))
		goto Allowed;

	*pdwEffect = DROPEFFECT_NONE;
	return E_INVALIDARG;

Allowed:
	return DragOver(grfKeyState, ptl, pdwEffect);
}

STDMETHODIMP LFDropTarget::DragOver(DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
	if (m_pDropTargetHelper)
	{
		POINT pt = { ptl.x, ptl.y };
		m_pDropTargetHelper->DragOver(&pt, *pdwEffect);
	}

	m_SkipTemplate = (grfKeyState & MK_SHIFT);

	*pdwEffect &= m_IsDragging ? DROPEFFECT_NONE : p_SearchResult ? DROPEFFECT_COPY : (grfKeyState & MK_CONTROL) ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
	return S_OK;
}

STDMETHODIMP LFDropTarget::DragLeave()
{
	if (m_pDropTargetHelper)
		m_pDropTargetHelper->DragLeave();

	return S_OK;
}

STDMETHODIMP LFDropTarget::Drop(IDataObject* pDataObject, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
	if (p_OwnerWnd)
		p_OwnerWnd->ActivateTopParent();

	if (m_pDropTargetHelper)
	{
		POINT pt = { ptl.x, ptl.y };
		m_pDropTargetHelper->Drop(pDataObject, &pt, *pdwEffect);
	}

	if ((DragOver(grfKeyState, ptl, pdwEffect)!=S_OK) || (m_IsDragging))
		return E_INVALIDARG;

	// Allowed?
	if (LFGetApp()->ShowNagScreen(NAG_EXPIRED | NAG_FORCE, p_OwnerWnd, TRUE))
		return E_INVALIDARG;

	// Data object
	COleDataObject dobj;
	dobj.Attach(pDataObject, FALSE);

	HGLOBAL hgDrop = dobj.GetGlobalData(CF_HDROP);
	HGLOBAL hgLiquid = dobj.GetGlobalData(LFGetApp()->CF_HLIQUID);

	if ((hgDrop==NULL) && (hgLiquid==NULL))
		return E_INVALIDARG;

	// Fenster
	CWnd* pWnd = p_OwnerWnd ? p_OwnerWnd : CWnd::GetForegroundWindow();

	// Clipboard
	if (p_SearchResult)
	{
		CWaitCursor csr;

		return AddToClipboard(hgLiquid, pWnd);
	}

	// Ziel-Store
	CHAR StoreID[LFKeySize];
	strcpy_s(StoreID, LFKeySize, p_Filter ? p_Filter->StoreID : m_StoreIDValid ? m_StoreID : "");

	// Wenn Default-Store gewünscht: verfügbar ?
	if (StoreID[0]=='\0')
		if (!LFDefaultStoreAvailable())
		{
			LFErrorBox(pWnd, LFNoDefaultStore);
			return E_INVALIDARG;
		}

	return hgLiquid ? ImportFromStore(pDataObject, hgLiquid, *pdwEffect, StoreID, pWnd) : ImportFromFS(hgDrop, *pdwEffect, StoreID, pWnd);
}
