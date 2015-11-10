
// LFDropTarget.h: Schnittstelle der Klasse LFDropTarget
//

#pragma once
#include "LFCore.h"


// LFDropTarget
//

class LFDropTarget : public IDropTarget
{
public:
	LFDropTarget();

	void SetDragging(BOOL IsDragging);
	void SetOwner(CWnd* pOwnerWnd);
	void SetFilter(LFFilter* pFilter, BOOL AllowChooseStore=TRUE);
	void SetStore(const CHAR* pStoreID, BOOL AllowChooseStore=TRUE);
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
	HRESULT ImportFromFS(HGLOBAL hgDrop, DWORD dwEffect, const CHAR* pStoreID, CWnd* pWnd) const;
	HRESULT ImportFromStore(IDataObject* pDataObject, HGLOBAL hgLiquid, DWORD dwEffect, const CHAR* pStoreID, CWnd* pWnd) const;
	HRESULT AddToClipboard(HGLOBAL hgLiquid, CWnd* pWnd) const;

	LONG m_lRefCount;
	IDropTargetHelper* m_pDropTargetHelper;
	LFFilter* p_Filter;
	LFSearchResult* p_SearchResult;
	CWnd* p_OwnerWnd;
	CHAR m_StoreID[LFKeySize];
	BOOL m_StoreIDValid;
	BOOL m_SkipTemplate;
	BOOL m_AllowChooseStore;
	BOOL m_IsDragging;
};
