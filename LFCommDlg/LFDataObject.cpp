
// LFDataObject.cpp: Implementierung der Klasse LFDataObject
//

#include "stdafx.h"
#include "LFDataObject.h"
#include "LFApplication.h"


BOOL DuplicateGlobalMemory(const HGLOBAL hSrc, HGLOBAL& hDst)
{
	if (!hSrc)
	{
		hDst = NULL;
		return FALSE;
	}

	SIZE_T sz = GlobalSize(hSrc);
	hDst = GlobalAlloc(GMEM_MOVEABLE, sz);
	if (!hDst)
		return FALSE;

	void* pSrc = GlobalLock(hSrc);
	void* pDst = GlobalLock(hDst);
	memcpy(pDst, pSrc, sz);
	GlobalUnlock(hSrc);
	GlobalUnlock(hDst);

	return TRUE;
}


// LFDataObject
//

LFDataObject::LFDataObject(LFTransactionList* tl)
{
	m_lRefCount = 1;
	m_hDropFiles = LFCreateDropFiles(tl);
	m_hLiquidFiles = LFCreateLiquidFiles(tl);
}

LFFileIDList* LFDataObject::GetFileIDList()
{
	return LFAllocFileIDList(m_hLiquidFiles);
}

STDMETHODIMP LFDataObject::QueryInterface(REFIID iid, void** ppvObject)
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

STDMETHODIMP_(ULONG) STDMETHODCALLTYPE LFDataObject::AddRef()
{
	return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(ULONG) STDMETHODCALLTYPE LFDataObject::Release()
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

STDMETHODIMP LFDataObject::GetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium)
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

	if (pFormatEtc->cfFormat==((LFApplication*)AfxGetApp())->CF_HLIQUID)
	{
		if (!DuplicateGlobalMemory(m_hLiquidFiles, pMedium->hGlobal))
			return STG_E_MEDIUMFULL;

		pMedium->tymed = TYMED_HGLOBAL;
		pMedium->pUnkForRelease = NULL;
		return S_OK;
	}

	return DV_E_FORMATETC;
}

STDMETHODIMP LFDataObject::GetDataHere(FORMATETC* /*pFormatEtc*/, STGMEDIUM* /*pMedium*/)
{
	return DATA_E_FORMATETC;
}

STDMETHODIMP LFDataObject::QueryGetData(FORMATETC* pFormatEtc)
{
	return ((pFormatEtc->cfFormat==CF_HDROP) || (pFormatEtc->cfFormat==((LFApplication*)AfxGetApp())->CF_HLIQUID)) &&
		(pFormatEtc->tymed & TYMED_HGLOBAL) ? S_OK : DV_E_FORMATETC;
}

STDMETHODIMP LFDataObject::GetCanonicalFormatEtc(FORMATETC* /*pFormatEtcIn*/, FORMATETC* pFormatEtcOut)
{
	pFormatEtcOut->ptd = NULL;

	return E_NOTIMPL;
}

STDMETHODIMP LFDataObject::SetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium, BOOL /*fRelease*/)
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

	if (pFormatEtc->cfFormat==((LFApplication*)AfxGetApp())->CF_HLIQUID)
	{
		if (m_hLiquidFiles)
			GlobalFree(m_hLiquidFiles);
		m_hLiquidFiles = pMedium->hGlobal;
		return S_OK;
	}

	return DV_E_FORMATETC;
}

STDMETHODIMP LFDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
{
	if (dwDirection==DATADIR_GET)
	{
		FORMATETC fmt[2];
		ZeroMemory(&fmt, sizeof(fmt));

		fmt[0].cfFormat = CF_HDROP;
		fmt[0].dwAspect = DVASPECT_CONTENT;
		fmt[0].lindex = -1;
		fmt[0].tymed = TYMED_HGLOBAL;

		fmt[1].cfFormat = ((LFApplication*)AfxGetApp())->CF_HLIQUID;
		fmt[1].dwAspect = DVASPECT_CONTENT;
		fmt[1].lindex = -1;
		fmt[1].tymed = TYMED_HGLOBAL;

		return SHCreateStdEnumFmtEtc(2, fmt, ppenumFormatEtc);
	}

	return E_NOTIMPL;
}

STDMETHODIMP LFDataObject::DAdvise(FORMATETC* /*pFormatEtc*/, DWORD /*advf*/, IAdviseSink* /*pAdvSink*/, DWORD* /*pdwConnection*/)
{
	return OLE_E_ADVISENOTSUPPORTED;;
}

STDMETHODIMP LFDataObject::DUnadvise(DWORD /*dwConnection*/)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP LFDataObject::EnumDAdvise(IEnumSTATDATA** /*ppenumAdvise*/)
{
	return OLE_E_ADVISENOTSUPPORTED;
}
