
// LFTransactionDataObject.h: Schnittstelle der Klasse LFTransactionDataObject
//

#pragma once
#include "LFCore.h"


// LFTransactionDataObject
//

class LFTransactionDataObject : public IDataObject
{
public:
	LFTransactionDataObject(LFTransactionList* tl);

	LFFileIDList* GetFileIDList();

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
	STDMETHOD(SetData)(FORMATETC* pFormatEtc, STGMEDIUM* pMedium, BOOL fRelease);
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
