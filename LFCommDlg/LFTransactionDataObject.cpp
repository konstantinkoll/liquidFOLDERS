
// LFTransactionDataObject.cpp: Implementierung der Klasse LFTransactionDataObject
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFTransactionDataObject
//

LFTransactionDataObject::LFTransactionDataObject(LFTransactionList* pTransactionList)
{
	m_lRefCount = 1;
	m_hDropFiles = LFCreateDropFiles(pTransactionList);
	m_hLiquidFiles = LFCreateLiquidFiles(pTransactionList);
}

LFTransactionList* LFTransactionDataObject::GetTransactionList()
{
	return LFAllocTransactionList(m_hLiquidFiles);
}

STDMETHODIMP LFTransactionDataObject::QueryInterface(REFIID iid, void** ppvObject)
{
	if ((iid==IID_IDataObject) || (iid==IID_IUnknown))
	{
		AddRef();
		*ppvObject = this;
		return S_OK;
	}

	*ppvObject = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) STDMETHODCALLTYPE LFTransactionDataObject::AddRef()
{
	return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(ULONG) STDMETHODCALLTYPE LFTransactionDataObject::Release()
{
	LONG Count = InterlockedDecrement(&m_lRefCount);
	if (!Count)
	{
		if (m_hDropFiles)
			GlobalFree(m_hDropFiles);

		if (m_hLiquidFiles)
			GlobalFree(m_hLiquidFiles);

		delete this;
		return 0;
	}

	return Count;
}

STDMETHODIMP LFTransactionDataObject::GetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium)
{
	if ((!pFormatEtc) || (!pMedium))
		return DV_E_FORMATETC;

	if ((pFormatEtc->tymed & TYMED_HGLOBAL)==0)
		return DV_E_FORMATETC;

	if (pFormatEtc->cfFormat==CF_HDROP)
	{
		if (!DuplicateGlobalMemory(m_hDropFiles, pMedium->hGlobal))
			return STG_E_MEDIUMFULL;

		pMedium->tymed = TYMED_HGLOBAL;
		pMedium->pUnkForRelease = NULL;
		return S_OK;
	}

	if (pFormatEtc->cfFormat==LFGetApp()->CF_HLIQUID)
	{
		if (!DuplicateGlobalMemory(m_hLiquidFiles, pMedium->hGlobal))
			return STG_E_MEDIUMFULL;

		pMedium->tymed = TYMED_HGLOBAL;
		pMedium->pUnkForRelease = NULL;
		return S_OK;
	}

	return DV_E_FORMATETC;
}

STDMETHODIMP LFTransactionDataObject::GetDataHere(FORMATETC* /*pFormatEtc*/, STGMEDIUM* /*pMedium*/)
{
	return DATA_E_FORMATETC;
}

STDMETHODIMP LFTransactionDataObject::QueryGetData(FORMATETC* pFormatEtc)
{
	return ((pFormatEtc->cfFormat==CF_HDROP) || (pFormatEtc->cfFormat==LFGetApp()->CF_HLIQUID)) &&
		(pFormatEtc->tymed & TYMED_HGLOBAL) ? S_OK : DV_E_FORMATETC;
}

STDMETHODIMP LFTransactionDataObject::GetCanonicalFormatEtc(FORMATETC* /*pFormatEtcIn*/, FORMATETC* pFormatEtcOut)
{
	pFormatEtcOut->ptd = NULL;

	return E_NOTIMPL;
}

STDMETHODIMP LFTransactionDataObject::SetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium, BOOL /*fRelease*/)
{
	if ((!pFormatEtc) || (!pMedium))
		return E_INVALIDARG;
	if ((pFormatEtc->tymed!=TYMED_HGLOBAL) || (pMedium->tymed!=TYMED_HGLOBAL))
		return DV_E_TYMED;
	if (!pMedium->hGlobal)
		return E_INVALIDARG;

	if (pFormatEtc->cfFormat==CF_HDROP)
	{
		if (m_hDropFiles)
			GlobalFree(m_hDropFiles);

		m_hDropFiles = pMedium->hGlobal;
		return S_OK;
	}

	if (pFormatEtc->cfFormat==LFGetApp()->CF_HLIQUID)
	{
		if (m_hLiquidFiles)
			GlobalFree(m_hLiquidFiles);

		m_hLiquidFiles = pMedium->hGlobal;
		return S_OK;
	}

	return DV_E_FORMATETC;
}

STDMETHODIMP LFTransactionDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
{
	if (dwDirection==DATADIR_GET)
	{
		FORMATETC fmt[2];
		ZeroMemory(&fmt, sizeof(fmt));

		fmt[0].cfFormat = CF_HDROP;
		fmt[0].dwAspect = DVASPECT_CONTENT;
		fmt[0].lindex = -1;
		fmt[0].tymed = TYMED_HGLOBAL;

		fmt[1].cfFormat = LFGetApp()->CF_HLIQUID;
		fmt[1].dwAspect = DVASPECT_CONTENT;
		fmt[1].lindex = -1;
		fmt[1].tymed = TYMED_HGLOBAL;

		return SHCreateStdEnumFmtEtc(2, fmt, ppenumFormatEtc);
	}

	return E_NOTIMPL;
}

STDMETHODIMP LFTransactionDataObject::DAdvise(FORMATETC* /*pFormatEtc*/, DWORD /*advf*/, IAdviseSink* /*pAdvSink*/, DWORD* /*pdwConnection*/)
{
	return OLE_E_ADVISENOTSUPPORTED;;
}

STDMETHODIMP LFTransactionDataObject::DUnadvise(DWORD /*dwConnection*/)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP LFTransactionDataObject::EnumDAdvise(IEnumSTATDATA** /*ppenumAdvise*/)
{
	return OLE_E_ADVISENOTSUPPORTED;
}
