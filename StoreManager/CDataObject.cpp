
// CDataObject.cpp: Implementierung der Klasse CDataObject
//

#include "stdafx.h"
#include "CDataObject.h"


// CDataObject
//

CDataObject::CDataObject(LFPhysicalLocationList* ll)
{
	m_lRefCount = 1;
	m_IsReset = TRUE;
	m_pPhysicalLocationList = ll;
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
		delete this;
		return 0;
	}

	return Count;
}

STDMETHODIMP CDataObject::GetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium)
{
	ASSERT(m_pPhysicalLocationList);

	if ((pFormatEtc->cfFormat!=CF_HDROP) || (pFormatEtc->tymed!=TYMED_HGLOBAL) || (!pMedium))
		return DV_E_FORMATETC;

	pMedium->tymed = TYMED_HGLOBAL;
	pMedium->hGlobal = LFCreateDropFiles(m_pPhysicalLocationList);
	pMedium->pUnkForRelease = NULL;

	return S_OK;
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
