
// LFDropTarget.h: Schnittstelle der Klasse LFDropTarget
//

#pragma once
#include "LF.h"


// LFDropTarget
//

class LFDropTarget : public IDropTarget
{
public:
	LFDropTarget();

	void SetDragging(BOOL IsDragging);
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
	STDMETHOD(DragEnter)(IDataObject* pDataObject, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect);
	STDMETHOD(DragOver)(DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect);
	STDMETHOD(DragLeave)();
	STDMETHOD(Drop)(IDataObject* pDataObject, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect);

	END_INTERFACE

protected:
	HRESULT ImportFromFS(HGLOBAL hgDrop, DWORD dwEffect, CHAR* StoreID, CWnd* pWnd);
	HRESULT ImportFromStore(IDataObject* pDataObject, HGLOBAL hgLiquid, DWORD dwEffect, CHAR* StoreID, CWnd* pWnd);
	HRESULT AddToClipboard(HGLOBAL hgLiquid, CWnd* pWnd);

	LONG m_lRefCount;
	IDropTargetHelper* m_pDropTargetHelper;
	LFFilter* p_Filter;
	LFSearchResult* p_SearchResult;
	CWnd* p_Owner;
	CHAR m_StoreID[LFKeySize];
	BOOL m_StoreIDValid;
	BOOL m_SkipTemplate;
	BOOL m_AllowChooseStore;
	BOOL m_IsDragging;
};
