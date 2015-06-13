
// LFStoreDataObject.h: Schnittstelle der Klasse LFStoreDataObject
//

#pragma once
#include "LF.h"


// LFStoreDataObject
//

class LFStoreDataObject : public IDataObject
{
public:
	LFStoreDataObject(LFItemDescriptor* i);
	LFStoreDataObject(LFStoreDescriptor* s);

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
	HGLOBAL m_hDescriptor;
	HGLOBAL m_hShellLink;

private:
	void CreateGlobals(IShellLink* pShellLink, WCHAR* Name);
};
