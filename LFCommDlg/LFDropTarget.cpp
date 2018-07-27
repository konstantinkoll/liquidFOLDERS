
// LFDropTarget.cpp: Implementierung der Klasse LFDropTarget
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFDropTarget
//

LFDropTarget::LFDropTarget()
{
	CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_IDropTargetHelper, (void**)&m_pDropTargetHelper);

	DEFAULTSTOREID(m_StoreID);
	p_OwnerWnd = NULL;
	m_SkipTemplate = m_AllowChooseStore = m_IsDragSource = m_IsDropAllowed = FALSE;
	p_SearchResult = NULL;
}

void LFDropTarget::SetStore(const ABSOLUTESTOREID& StoreID)
{
	m_StoreID = StoreID;
	m_AllowChooseStore = FALSE;
}

void LFDropTarget::SetStore(LFFilter* pFilter)
{
	m_StoreID = pFilter ? pFilter->Query.StoreID : DEFAULTSTOREID();
	m_AllowChooseStore = TRUE;
}

inline HRESULT LFDropTarget::ImportFromFS(HDROP hDrop, DWORD dwEffect, CWnd* pWnd) const
{
	if (!hDrop)
		return E_INVALIDARG;

	// Thread worker
	WorkerImportParameters Parameters;
	ZeroMemory(&Parameters, sizeof(Parameters));
	Parameters.StoreID = m_StoreID;
	Parameters.DeleteSource = (dwEffect & DROPEFFECT_MOVE)!=0;

	// Fill template
	if (!m_SkipTemplate)
	{
		LFItemTemplateDlg dlg(Parameters.pItemTemplate=LFAllocItemDescriptor(), m_StoreID, m_AllowChooseStore, pWnd);
		if (dlg.DoModal()==IDCANCEL)
		{
			LFFreeItemDescriptor(Parameters.pItemTemplate);
			return E_ABORT;
		}

		Parameters.StoreID = dlg.m_StoreID;
	}

	Parameters.pFileImportList = LFAllocFileImportList(hDrop);

	LFDoWithProgress(WorkerImport, &Parameters.Hdr, pWnd);
	LFErrorBox(pWnd, Parameters.Hdr.Result);

	LFFreeFileImportList(Parameters.pFileImportList);

	if (Parameters.pItemTemplate)
		LFFreeItemDescriptor(Parameters.pItemTemplate);

	if (p_OwnerWnd)
		p_OwnerWnd->SendMessage(LFGetMessageIDs()->ItemsDropped);

	return (Parameters.Hdr.Result==LFOk) ? S_OK : E_INVALIDARG;
}

inline HRESULT LFDropTarget::ImportFromStore(HLIQUIDFILES hLiquidFiles, IDataObject* pDataObject, CWnd* pWnd) const
{
	if (!hLiquidFiles)
		return E_INVALIDARG;

	// Thread worker
	WorkerSendToParameters Parameters;
	ZeroMemory(&Parameters, sizeof(Parameters));
	Parameters.pTransactionList = LFAllocTransactionListEx(hLiquidFiles);
	Parameters.StoreID = m_StoreID;

	LFDoWithProgress(WorkerSendTo, &Parameters.Hdr, pWnd);
	LFErrorBox(pWnd, Parameters.Hdr.Result);

	// Re-set CF_LIQUIDFILES to remove unaffected files (error or drop on same store)
	FORMATETC Format;
	ZeroMemory(&Format, sizeof(Format));
	Format.cfFormat = LFGetApp()->CF_LIQUIDFILES;
	Format.dwAspect = DVASPECT_CONTENT;
	Format.lindex = -1;
	Format.tymed = TYMED_HGLOBAL;

	STGMEDIUM Medium;
	ZeroMemory(&Medium, sizeof(Medium));
	Medium.tymed = TYMED_HGLOBAL;
	Medium.hGlobal = LFCreateLiquidFiles(Parameters.pTransactionList);

	pDataObject->SetData(&Format, &Medium, FALSE);

	LFFreeTransactionList(Parameters.pTransactionList);

	if (p_OwnerWnd)
		p_OwnerWnd->SendMessage(LFGetMessageIDs()->ItemsDropped);

	return (Parameters.Hdr.Result==LFOk) ? S_OK : E_INVALIDARG;
}

inline HRESULT LFDropTarget::AddToClipboard(HLIQUIDFILES hLiquidFiles, CWnd* pWnd) const
{
	if (!hLiquidFiles)
		return E_INVALIDARG;

	CWaitCursor WaitCursor;

	LFTransactionList* pTransactionList = LFAllocTransactionListEx(hLiquidFiles);

	const UINT Result = LFDoTransaction(pTransactionList, LFTransactionAddToSearchResult, NULL, (UINT_PTR)p_SearchResult);
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

STDMETHODIMP LFDropTarget::DragEnter(IDataObject* pDataObject, DWORD grfKeyState, POINTL Point, LPDWORD pdwEffect)
{
	// Drop target helper
	if (m_pDropTargetHelper)
		m_pDropTargetHelper->DragEnter(p_OwnerWnd->GetSafeHwnd(), pDataObject, (LPPOINT)&Point, *pdwEffect);

	// Is drop allowed?
	COleDataObject DataObject;
	DataObject.Attach(pDataObject, FALSE);

	if ((m_IsDropAllowed=DataObject.GetGlobalData(LFGetApp()->CF_LIQUIDFILES) || (DataObject.GetGlobalData(CF_HDROP) && !p_SearchResult))==TRUE)
	{
		return DragOver(grfKeyState, Point, pdwEffect);
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;

		return S_OK;
	}
}

STDMETHODIMP LFDropTarget::DragOver(DWORD grfKeyState, POINTL Point, LPDWORD pdwEffect)
{
	// Drop target helper
	if (m_pDropTargetHelper)
		m_pDropTargetHelper->DragOver((LPPOINT)&Point, *pdwEffect);

	// Skip template
	m_SkipTemplate = (grfKeyState & MK_SHIFT);

	// Drop effect
	*pdwEffect &= m_IsDragSource || !m_IsDropAllowed ? DROPEFFECT_NONE : p_SearchResult ? DROPEFFECT_COPY : (grfKeyState & MK_CONTROL) ? DROPEFFECT_MOVE : DROPEFFECT_COPY;

	return S_OK;
}

STDMETHODIMP LFDropTarget::DragLeave()
{
	// Drop target helper
	if (m_pDropTargetHelper)
		m_pDropTargetHelper->DragLeave();

	return S_OK;
}

STDMETHODIMP LFDropTarget::Drop(IDataObject* pDataObject, DWORD grfKeyState, POINTL Point, LPDWORD pdwEffect)
{
	DragOver(grfKeyState, Point, pdwEffect);

	// Drop target helper
	if (m_pDropTargetHelper)
		m_pDropTargetHelper->Drop(pDataObject, (LPPOINT)&Point, *pdwEffect);

	if (m_IsDragSource || !m_IsDropAllowed)
		return E_INVALIDARG;

	// Bring window to front
	CWnd* pWnd;
	if (p_OwnerWnd)
	{
		pWnd = p_OwnerWnd;

		p_OwnerWnd->ActivateTopParent();
	}
	else
	{
		pWnd = CWnd::GetForegroundWindow();
	}

	// Allowed?
	if (!LFNagScreen(pWnd))
		return E_INVALIDARG;

	// Data object
	COleDataObject DataObject;
	DataObject.Attach(pDataObject, FALSE);

	HDROP hDrop = (HDROP)DataObject.GetGlobalData(CF_HDROP);
	HLIQUIDFILES hLiquidFiles = (HLIQUIDFILES)DataObject.GetGlobalData(LFGetApp()->CF_LIQUIDFILES);

	return p_SearchResult ? AddToClipboard(hLiquidFiles, pWnd) :
		hLiquidFiles ? ImportFromStore(hLiquidFiles, pDataObject, pWnd) : ImportFromFS(hDrop, *pdwEffect, pWnd);
}
