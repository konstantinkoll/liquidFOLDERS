
// CDataObject.cpp: Implementierung der Klasse CDataObject
//

#include "stdafx.h"
#include "CDataObject.h"
#include "LFApplication.h"


BOOL DuplicateGlobalMemory(const HGLOBAL hSrc, HGLOBAL& hDst)
{
	if (!hSrc)
	{
		hDst = NULL;
		return FALSE;
	}

	DWORD sz = GlobalSize(hSrc);
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


// CDataObject
//

CDataObject::CDataObject(LFPhysicalLocationList* ll)
{
	m_lRefCount = 1;
	m_IsReset = TRUE;
	m_hDropFiles = LFCreateDropFiles(ll);
	m_hLiquidFiles = LFCreateLiquidFiles(ll);
}

STDMETHODIMP CDataObject::QueryInterface(REFIID iid, void** ppvObject)
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

STDMETHODIMP_(ULONG) STDMETHODCALLTYPE CDataObject::AddRef()
{
	return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(ULONG) STDMETHODCALLTYPE CDataObject::Release()
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

STDMETHODIMP CDataObject::GetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium)
{
	if ((!pFormatEtc) || (!pMedium))
		return DV_E_FORMATETC;

	if ((pFormatEtc->cfFormat==CF_HDROP) && (pFormatEtc->tymed==TYMED_HGLOBAL))
	{
		if (!DuplicateGlobalMemory(m_hDropFiles, pMedium->hGlobal))
			return STG_E_MEDIUMFULL;

		pMedium->tymed = TYMED_HGLOBAL;
		pMedium->pUnkForRelease = NULL;
		return S_OK;
	}

	if ((pFormatEtc->cfFormat==((LFApplication*)AfxGetApp())->CF_HLIQUID) && (pFormatEtc->tymed==TYMED_HGLOBAL))
	{
		if (!DuplicateGlobalMemory(m_hLiquidFiles, pMedium->hGlobal))
			return STG_E_MEDIUMFULL;

		pMedium->tymed = TYMED_HGLOBAL;
		pMedium->pUnkForRelease = NULL;
		return S_OK;
	}

	return DV_E_FORMATETC;
}

STDMETHODIMP CDataObject::GetDataHere(FORMATETC* /*pFormatEtc*/, STGMEDIUM* /*pMedium*/)
{
	return DATA_E_FORMATETC;
}

STDMETHODIMP CDataObject::QueryGetData(FORMATETC* pFormatEtc)
{
	return (pFormatEtc->cfFormat==CF_HDROP) && (pFormatEtc->tymed==TYMED_HGLOBAL) ? S_OK : DV_E_FORMATETC;
}

STDMETHODIMP CDataObject::GetCanonicalFormatEtc(FORMATETC* /*pFormatEtcIn*/, FORMATETC* pFormatEtcOut)
{
	pFormatEtcOut->ptd = NULL;

	return E_NOTIMPL;
}

STDMETHODIMP CDataObject::SetData(FORMATETC* /*pFormatEtc*/, STGMEDIUM* /*pmedium*/, BOOL /*fRelease*/)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
{
	if (dwDirection==DATADIR_GET)
	{
		FORMATETC fmt;
		ZeroMemory(&fmt, sizeof(fmt));
		fmt.cfFormat = CF_HDROP;
		fmt.dwAspect = DVASPECT_CONTENT;
		fmt.lindex = -1;
		fmt.tymed = TYMED_HGLOBAL;

		return SHCreateStdEnumFmtEtc(1, &fmt, ppenumFormatEtc);
	}

	return E_NOTIMPL;
}

STDMETHODIMP CDataObject::DAdvise(FORMATETC* /*pFormatEtc*/, DWORD /*advf*/, IAdviseSink* /*pAdvSink*/, DWORD* /*pdwConnection*/)
{
	return OLE_E_ADVISENOTSUPPORTED;;
}

STDMETHODIMP CDataObject::DUnadvise(DWORD /*dwConnection*/)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP CDataObject::EnumDAdvise(IEnumSTATDATA** /*ppenumAdvise*/)
{
	return OLE_E_ADVISENOTSUPPORTED;
}
