
// LFDataObject.h: Schnittstelle der Klasse LFDataObject
//

#pragma once
#include "LFCore.h"


// LFDataObject
//

class AFX_EXT_CLASS LFDataObject : public IDataObject
{
public:
	LFDataObject(LFTransactionList* tl);

	BEGIN_INTERFACE

	// IUnknown members
	STDMETHOD(QueryInterface)(REFIID iid, void** ppvObject);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release());

	// IDataObject members
	STDMETHOD(GetData)(FORMATETC* pFormatEtc, STGMEDIUM* pMedium);
	STDMETHOD(GetDataHere)(FORMATETC* pFormatEtc, STGMEDIUM* pMedium);
	STDMETHOD(QueryGetData)(FORMATETC* pFormatEtc);
	STDMETHOD(GetCanonicalFormatEtc)(FORMATETC* pFormatEtcIn, FORMATETC* pFormatEtcOut);
	STDMETHOD(SetData)(FORMATETC* pFormatEtc, STGMEDIUM* pmedium, BOOL fRelease);
	STDMETHOD(EnumFormatEtc)(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc);
	STDMETHOD(DAdvise)(FORMATETC* pFormatEtc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection);
	STDMETHOD(DUnadvise)(DWORD dwConnection);
	STDMETHOD(EnumDAdvise)(IEnumSTATDATA** ppenumAdvise);

	END_INTERFACE

protected:
	LONG m_lRefCount;
	HGLOBAL m_hDropFiles;
	HGLOBAL m_hLiquidFiles;
};
