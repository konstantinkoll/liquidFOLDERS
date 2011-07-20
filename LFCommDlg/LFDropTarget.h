
// LFDropTarget.h: Schnittstelle der Klasse LFDropTarget
//

#pragma once
#include "afxole.h"
#include "liquidFOLDERS.h"


// LFDropTarget
//

class AFX_EXT_CLASS LFDropTarget : public IDropTarget
{
public:
	LFDropTarget();

	void SetOwner(CWnd* pOwner);
	void SetFilter(LFFilter* pFilter, BOOL AllowChooseStore=TRUE);
	void SetStore(CHAR* StoreID, BOOL AllowChooseStore=TRUE);
	void SetSearchResult(LFSearchResult* pSearchResult);

	BEGIN_INTERFACE

	// IUnknown members
	STDMETHOD(QueryInterface)(REFIID iid, void** ppvObject);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release());

	// IDropTarget members
	STDMETHOD(DragEnter)(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	STDMETHOD(DragLeave)();
	STDMETHOD(Drop)(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);

	END_INTERFACE

protected:
	LONG m_lRefCount;
	LFFilter* p_Filter;
	LFSearchResult* p_SearchResult;
	CWnd* p_Owner;
	CHAR m_StoreID[LFKeySize];
	BOOL m_StoreIDValid;
	BOOL m_AllowChooseStore;
	BOOL m_SkipTemplate;
};
