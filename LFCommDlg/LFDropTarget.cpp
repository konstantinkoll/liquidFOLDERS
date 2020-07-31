
// LFDropTarget.cpp: Implementierung der Klasse LFDropTarget
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFDropTarget
//

CString LFDropTarget::m_strDefaultStore;
CString LFDropTarget::m_strImportCopy;
CString LFDropTarget::m_strImportMove;
CString LFDropTarget::m_strRemember;
CString LFDropTarget::m_strClipboard;

LFDropTarget::LFDropTarget()
	: CBackstageDropTarget()
{
	DEFAULTSTOREID(m_StoreID);
	m_SkipTemplate = m_AllowChooseStore = m_IsDragSource = m_IsDropAllowed = FALSE;
	p_SearchResult = NULL;
	m_StoreName[0] = L'\0';

	if (m_strDefaultStore.IsEmpty())
	{
		ENSURE(m_strDefaultStore.LoadString(IDS_DEFAULTSTORE));
		ENSURE(m_strImportCopy.LoadString(IDS_DROPTARGET_IMPORTCOPY));
		ENSURE(m_strImportMove.LoadString(IDS_DROPTARGET_IMPORTMOVE));
		ENSURE(m_strRemember.LoadString(IDS_DROPTARGET_REMEMBER));
		ENSURE(m_strClipboard.LoadString(IDR_CLIPBOARD));
	}
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

inline BOOL LFDropTarget::ImportFromFS(CWnd* pWnd, HDROP hDrop, DROPEFFECT DropEffect) const
{
	if (!hDrop)
		return FALSE;

	// Thread worker
	WorkerImportParameters Parameters;
	ZeroMemory(&Parameters, sizeof(Parameters));
	Parameters.StoreID = m_StoreID;
	Parameters.DeleteSource = (DropEffect & DROPEFFECT_MOVE)!=0;

	// Fill template
	if (!m_SkipTemplate)
	{
		LFItemTemplateDlg dlg(Parameters.pItemTemplate=LFAllocItemDescriptor(), m_StoreID, m_AllowChooseStore, pWnd);
		if (dlg.DoModal()==IDCANCEL)
		{
			LFFreeItemDescriptor(Parameters.pItemTemplate);

			return FALSE;
		}

		Parameters.StoreID = dlg.m_StoreID;
	}

	Parameters.pFileImportList = LFAllocFileImportList(hDrop);

	LFDoWithProgress(WorkerImport, &Parameters.Hdr, pWnd);
	LFErrorBox(pWnd, Parameters.Hdr.Result);

	LFFreeFileImportList(Parameters.pFileImportList);

	if (Parameters.pItemTemplate)
		LFFreeItemDescriptor(Parameters.pItemTemplate);

	return Parameters.Hdr.Result==LFOk;
}

inline BOOL LFDropTarget::ImportFromStore(CWnd* pWnd, HLIQUIDFILES hLiquidFiles, COleDataObject* pDataObject) const
{
	if (!hLiquidFiles)
		return FALSE;

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

	pDataObject->m_lpDataObject->SetData(&Format, &Medium, FALSE);

	LFFreeTransactionList(Parameters.pTransactionList);

	return Parameters.Hdr.Result==LFOk;
}

inline BOOL LFDropTarget::AddToClipboard(CWnd* pWnd, HLIQUIDFILES hLiquidFiles) const
{
	if (!hLiquidFiles)
		return FALSE;

	CWaitCursor WaitCursor;

	// Deselect items
	LFTransactionList* pTransactionList = LFAllocTransactionListEx(hLiquidFiles);

	if (pTransactionList->m_ItemCount)
		for (UINT a=0; a<p_SearchResult->m_ItemCount; a++)
			(*p_SearchResult)[a]->Flags &= ~LFFlagsItemSelected;

	// Do work
	const UINT Result = LFDoTransaction(pTransactionList, LFTransactionAddToSearchResult, NULL, (UINT_PTR)p_SearchResult);
	LFErrorBox(pWnd, Result);

	LFFreeTransactionList(pTransactionList);

	return Result==LFOk;
}

DROPEFFECT LFDropTarget::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	// Store name
	if (!IsClipboard())
	{
		LFStoreDescriptor StoreDescriptor;
		LPCWSTR StoreName = LFGetStoreSettings(m_StoreID, StoreDescriptor)==LFOk ? StoreDescriptor.StoreName : L"?";

		if (LFIsDefaultStoreID(m_StoreID))
		{
			wcscpy_s(m_StoreName, 256, m_strDefaultStore);
			wcsncat_s(m_StoreName, 256, L" (", _TRUNCATE);
			wcsncat_s(m_StoreName, 256, StoreName, _TRUNCATE);
			wcsncat_s(m_StoreName, 256, L")", _TRUNCATE);
		}
		else
		{
			wcscpy_s(m_StoreName, 256, StoreName);
		}
	}

	// Is drop allowed?
	const BOOL HasLiquidFiles = pDataObject->IsDataAvailable(LFGetApp()->CF_LIQUIDFILES);
	const BOOL HasFiles = pDataObject->IsDataAvailable(CF_HDROP);

	const DROPEFFECT DropEffect = ((m_IsDropAllowed=HasLiquidFiles || (HasFiles && !IsClipboard()))==TRUE) ? OnDragOver(pWnd, pDataObject, dwKeyState, point) : DROPEFFECT_NONE;

	// Drop target helper
	if (m_pDropTargetHelper)
		m_pDropTargetHelper->DragEnter(pWnd->GetSafeHwnd(), pDataObject->m_lpDataObject, &point, DropEffect);

	return DropEffect;
}

DROPEFFECT LFDropTarget::OnDragOver(CWnd* /*pWnd*/, COleDataObject* /*pDataObject*/, DWORD dwKeyState, CPoint point)
{
	// Skip template
	m_SkipTemplate = (dwKeyState & MK_CONTROL);

	// Drop effect
	const DROPEFFECT DropEffect = m_IsDragSource || !m_IsDropAllowed ? DROPEFFECT_NONE : p_SearchResult ? DROPEFFECT_LINK : (dwKeyState & MK_SHIFT) ? DROPEFFECT_MOVE : DROPEFFECT_COPY;

	// Drop description
	SetDropDescription(m_IsDragSource ? DROPIMAGE_INVALID : DropEffectToDropImage(DropEffect),
		IsClipboard() ? m_strRemember : DropEffect==DROPEFFECT_MOVE ? m_strImportMove : m_strImportCopy,
		IsClipboard() ? m_strClipboard : m_StoreName);

	// Drop target helper
	if (m_pDropTargetHelper)
		m_pDropTargetHelper->DragOver(&point, DropEffect);

	return DropEffect;
}

BOOL LFDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT DropEffect, CPoint point)
{
	// Drop target helper
	if (m_pDropTargetHelper)
		m_pDropTargetHelper->Drop(pDataObject->m_lpDataObject, &point, DropEffect);

	if (m_IsDragSource || !m_IsDropAllowed)
		return FALSE;

	// Allowed?
	if (!LFNagScreen(pWnd))
		return FALSE;

	// Bring window to front
	if (!pWnd)
		pWnd = CWnd::GetForegroundWindow();

	pWnd->ActivateTopParent();

	// Drop
	HDROP hDrop = (HDROP)pDataObject->GetGlobalData(CF_HDROP);
	HLIQUIDFILES hLiquidFiles = (HLIQUIDFILES)pDataObject->GetGlobalData(LFGetApp()->CF_LIQUIDFILES);

	const BOOL Result = p_SearchResult ? AddToClipboard(pWnd, hLiquidFiles) :
		hLiquidFiles ? ImportFromStore(pWnd, hLiquidFiles, pDataObject) : ImportFromFS(pWnd, hDrop, DropEffect);

	if (Result)
		SendDroppedMessage(pWnd);

	return Result;
}
