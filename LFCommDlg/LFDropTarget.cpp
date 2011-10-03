
// LFDropTarget.cpp: Implementierung der Klasse LFDropTarget
//

#include "StdAfx.h"
#include "LFCommDlg.h"


// Thread workers
//

struct WorkerParameters
{
	LFWorkerParameters Hdr;
	CHAR StoreID[LFKeySize];
	BOOL Move;
	LFItemDescriptor* Template;
	union
	{
		LFFileIDList* FileIDList;
		LFFileImportList* FileImportList;
	};
};

DWORD WINAPI WorkerImportFromFS(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	LFTransactionImport(wp->StoreID, wp->FileImportList, wp->Template, true, wp->Move==TRUE, &p);

	LF_WORKERTHREAD_FINISH();
}

DWORD WINAPI WorkerImportFromStore(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	LFTransactionImport(wp->StoreID, wp->FileIDList, wp->Move==TRUE, &p);

	LF_WORKERTHREAD_FINISH();
}


// LFDropTarget
//

LFDropTarget::LFDropTarget()
{
	CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_IDropTargetHelper, (void**)&m_pDropTargetHelper);

	p_Owner = NULL;
	m_StoreIDValid = m_AllowChooseStore = m_IsDragging = FALSE;
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
	WorkerParameters wp;
	ZeroMemory(&wp, sizeof(wp));
	strcpy_s(wp.StoreID, LFKeySize, StoreID);
	wp.Move = (dwEffect & DROPEFFECT_MOVE)!=0;

	if (p_Filter)
	{
		wp.Template = LFAllocItemDescriptor();

		LFFilterCondition* pCondition = p_Filter->ConditionList;
		while (pCondition)
		{
			if (pCondition->Compare==LFFilterCompareSubfolder)
			{
				UINT Attr = pCondition->AttrData.Attr;
				if ((!((LFApplication*)AfxGetApp())->m_Attributes[Attr]->ReadOnly) && (Attr!=LFAttrFileName))
					LFSetAttributeVariantData(wp.Template, &pCondition->AttrData);
			}

			pCondition = pCondition->Next;
		}
	}

	HDROP hDrop = (HDROP)GlobalLock(hgDrop);
	wp.FileImportList = LFAllocFileImportList(hDrop);
	GlobalUnlock(hgDrop);

	LFDoWithProgress(WorkerImportFromFS, &wp.Hdr, pWnd);
	UINT res = wp.FileImportList->m_LastError;
	LFErrorBox(res, pWnd->GetSafeHwnd());

	LFFreeFileImportList(wp.FileImportList);
	LFFreeItemDescriptor(wp.Template);

	if (p_Owner)
		p_Owner->SendMessage(LFGetMessageIDs()->ItemsDropped, NULL, NULL);

	return (res==LFOk) ? S_OK : E_INVALIDARG;
}

__forceinline HRESULT LFDropTarget::ImportFromStore(IDataObject* pDataObject, HGLOBAL hgLiquid, DWORD dwEffect, CHAR* StoreID, CWnd* pWnd)
{
	WorkerParameters wp;
	ZeroMemory(&wp, sizeof(wp));
	strcpy_s(wp.StoreID, LFKeySize, StoreID);
	wp.Move = (dwEffect & DROPEFFECT_MOVE)!=0;

	HLIQUID hLiquid = (HLIQUID)GlobalLock(hgLiquid);
	wp.FileIDList = LFAllocFileIDList(hLiquid);
	GlobalUnlock(hgLiquid);

	LFDoWithProgress(WorkerImportFromStore, &wp.Hdr, pWnd);
	UINT res = wp.FileIDList->m_LastError;
	LFErrorBox(res, pWnd->GetSafeHwnd());

	// CF_LIQUIDFILES neu setzen, um nicht veränderte Dateien (Fehler oder Drop auf denselben Store) zu entfernen
	FORMATETC fmt;
	ZeroMemory(&fmt, sizeof(fmt));
	fmt.cfFormat = ((LFApplication*)AfxGetApp())->CF_HLIQUID;
	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex = -1;
	fmt.tymed = TYMED_HGLOBAL;

	STGMEDIUM stg;
	ZeroMemory(&stg, sizeof(stg));
	stg.tymed = TYMED_HGLOBAL;
	stg.hGlobal = LFCreateLiquidFiles(wp.FileIDList);

	pDataObject->SetData(&fmt, &stg, FALSE);

	LFFreeFileIDList(wp.FileIDList);

	if (p_Owner)
		p_Owner->SendMessage(LFGetMessageIDs()->ItemsDropped, NULL, NULL);

	return (res==LFOk) ? S_OK : E_INVALIDARG;
}

__forceinline HRESULT LFDropTarget::AddToClipboard(HGLOBAL hgLiquid, CWnd* pWnd)
{
	if (!hgLiquid)
		return E_INVALIDARG;

	CWaitCursor wait;

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
	return InterlockedDecrement(&m_lRefCount);
}

STDMETHODIMP LFDropTarget::DragEnter(IDataObject* pDataObject, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
	if (m_pDropTargetHelper)
	{
		POINT pt = { ptl.x, ptl.y };
		m_pDropTargetHelper->DragEnter(p_Owner ? p_Owner->GetSafeHwnd() : NULL, pDataObject, &pt, *pdwEffect);
	}

	COleDataObject dobj;
	dobj.Attach(pDataObject, FALSE);

	if (dobj.GetGlobalData(((LFApplication*)AfxGetApp())->CF_HLIQUID))
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
	if (p_Owner)
		p_Owner->ActivateTopParent();

	if (m_pDropTargetHelper)
	{
		POINT pt = { ptl.x, ptl.y };
		m_pDropTargetHelper->Drop(pDataObject, &pt, *pdwEffect);
	}

	if ((DragOver(grfKeyState, ptl, pdwEffect)!=S_OK) || (m_IsDragging))
		return E_INVALIDARG;

	// Allowed?
	if (((LFApplication*)AfxGetApp())->ShowNagScreen(NAG_EXPIRED | NAG_FORCE, p_Owner, TRUE))
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
	{
		CWaitCursor wait;

		return AddToClipboard(hgLiquid, pWnd);
	}

	// Ziel-Store
	CHAR StoreID[LFKeySize];
	strcpy_s(StoreID, LFKeySize, p_Filter ? p_Filter->StoreID : m_StoreIDValid ? m_StoreID : "");

	// Wenn Default-Store gewünscht: verfügbar ?
	if (StoreID[0]=='\0')
		if (!LFDefaultStoreAvailable())
		{
			LFErrorBox(LFNoDefaultStore, pWnd->GetSafeHwnd());
			return E_INVALIDARG;
		}

	return hgLiquid ? ImportFromStore(pDataObject, hgLiquid, *pdwEffect, StoreID, pWnd) : ImportFromFS(hgDrop, *pdwEffect, StoreID, pWnd);
}
