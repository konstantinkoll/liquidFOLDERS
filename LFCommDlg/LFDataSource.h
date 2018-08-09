
// LFDataSource.h: Schnittstelle der Klasse LFDataSource
//

#pragma once


// LFDataSource
//

class LFDataSource : public COleDataSource
{
public:
	LFDataSource(LFItemDescriptor* pItemDescriptor);
	LFDataSource(LFTransactionList* pTransactionList);

	void CacheGlobalData(CLIPFORMAT cfFormat, LPVOID pData, SIZE_T szData, LPFORMATETC lpFormatEtc=NULL);
	LPDATAOBJECT GetDataObject();

	BEGIN_INTERFACE_PART(LFDataObject, IDataObject)
		INIT_INTERFACE_PART(LFDataSource, LFDataObject)

		// IDataObject members
		STDMETHOD(GetData)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(GetDataHere)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(QueryGetData)(LPFORMATETC);
		STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC, LPFORMATETC);
		STDMETHOD(SetData)(LPFORMATETC, LPSTGMEDIUM, BOOL);
		STDMETHOD(EnumFormatEtc)(DWORD, LPENUMFORMATETC*);
		STDMETHOD(DAdvise)(LPFORMATETC, DWORD, LPADVISESINK, LPDWORD);
		STDMETHOD(DUnadvise)(DWORD);
		STDMETHOD(EnumDAdvise)(LPENUMSTATDATA*);
	END_INTERFACE_PART(LFDataObject)

	DECLARE_INTERFACE_MAP()

protected:
	void InitDragSourceHelper(HBITMAP hBitmap);
	void InitDragSourceHelper(LFItemDescriptor* pItemDescriptor);
	void InitDragSourceHelper(UINT nID);

	IDragSourceHelper* m_pDragSourceHelper;
	IDragSourceHelper2* m_pDragSourceHelper2;
};

inline LPDATAOBJECT LFDataSource::GetDataObject()
{
	return (LPDATAOBJECT)GetInterface(&IID_IDataObject);
}
