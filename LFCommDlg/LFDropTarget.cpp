
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

void LFDropTarget::SetStore(const LPCSTR pStoreID, BOOL AllowChooseStore)
{
	ASSERT(pStoreID);

	p_Filter = NULL;
	strcpy_s(m_StoreID, LFKeySize, pStoreID);
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

__forceinline HRESULT LFDropTarget::ImportFromFS(HGLOBAL hgDrop, DWORD dwEffect, const LPCSTR pStoreID, CWnd* pWnd) const
{
	ASSERT(pStoreID);

	WorkerParameters wp;
	ZeroMemory(&wp, sizeof(wp));
	strcpy_s(wp.StoreID, LFKeySize, pStoreID);
	wp.DeleteSource = (dwEffect & DROPEFFECT_MOVE)!=0;

	// Template füllen
	if (!m_SkipTemplate)
	{
		wp.pItemTemplate = LFAllocItemDescriptor();

		LFItemTemplateDlg dlg(wp.pItemTemplate, wp.StoreID, pWnd, m_AllowChooseStore, p_Filter);
		if (dlg.DoModal()==IDCANCEL)
		{
			LFFreeItemDescriptor(wp.pItemTemplate);
			return E_ABORT;
		}

		if (m_AllowChooseStore)
			strcpy_s(wp.StoreID, LFKeySize, dlg.m_StoreID);
	}

	HDROP hDrop = (HDROP)GlobalLock(hgDrop);
	wp.pFileImportList = LFAllocFileImportList(hDrop);
	GlobalUnlock(hgDrop);

	LFDoWithProgress(WorkerImport, &wp.Hdr, pWnd);
	UINT Result = wp.pFileImportList->m_LastError;
	LFErrorBox(pWnd, Result);

	LFFreeFileImportList(wp.pFileImportList);

	if (wp.pItemTemplate)
		LFFreeItemDescriptor(wp.pItemTemplate);

	if (p_OwnerWnd)
		p_OwnerWnd->SendMessage(LFGetMessageIDs()->ItemsDropped);

	return (Result==LFOk) ? S_OK : E_INVALIDARG;
}

__forceinline HRESULT LFDropTarget::ImportFromStore(IDataObject* pDataObject, HGLOBAL hgLiquid, DWORD dwEffect, const LPCSTR pStoreID, CWnd* pWnd) const
{
	ASSERT(pStoreID);

	WorkerParameters wp;
	ZeroMemory(&wp, sizeof(wp));
	strcpy_s(wp.StoreID, LFKeySize, pStoreID);
	wp.DeleteSource = (dwEffect & DROPEFFECT_MOVE)!=0;

	HLIQUID hLiquid = (HLIQUID)GlobalLock(hgLiquid);
	wp.pTransactionList = LFAllocTransactionListEx(hLiquid);
	GlobalUnlock(hgLiquid);

	LFDoWithProgress(WorkerSendTo, &wp.Hdr, pWnd);
	UINT Result = wp.pTransactionList->m_LastError;
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
	stg.hGlobal = LFCreateLiquidFiles(wp.pTransactionList);

	pDataObject->SetData(&fmt, &stg, FALSE);

	LFFreeTransactionList(wp.pTransactionList);

	if (p_OwnerWnd)
		p_OwnerWnd->SendMessage(LFGetMessageIDs()->ItemsDropped);

	return (Result==LFOk) ? S_OK : E_INVALIDARG;
}

__forceinline HRESULT LFDropTarget::AddToClipboard(HGLOBAL hgLiquid, CWnd* pWnd) const
{
	if (!hgLiquid)
		return E_INVALIDARG;

	CWaitCursor csr;

	HLIQUID hLiquid = (HLIQUID)GlobalLock(hgLiquid);
	LFTransactionList* pTransactionList = LFAllocTransactionListEx(hLiquid);
	GlobalUnlock(hgLiquid);

	LFDoTransaction(pTransactionList, LFTransactionTypeAddToSearchResult, NULL, (UINT_PTR)p_SearchResult);
	UINT Result = pTransactionList->m_LastError;
	LFErrorBox(pWnd, Result);
	LFFreeTransactionList(pTransactionList);

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
	if (!LFNagScreen(p_OwnerWnd))
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
	UINT Result;
	if (StoreID[0]=='\0')
		if ((Result=LFGetDefaultStore())!=LFOk)
		{
			LFErrorBox(pWnd, Result);
			return E_INVALIDARG;
		}

	return hgLiquid ? ImportFromStore(pDataObject, hgLiquid, *pdwEffect, StoreID, pWnd) : ImportFromFS(hgDrop, *pdwEffect, StoreID, pWnd);
}
