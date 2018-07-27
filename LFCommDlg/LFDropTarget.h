
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

	void SetDragSource(BOOL IsDragSource);
	void SetOwner(CWnd* pOwnerWnd);
	void SetStore(const ABSOLUTESTOREID& StoreID);
	void SetStore(LFFilter* pFilter);
	void SetSearchResult(LFSearchResult* pSearchResult);

	BEGIN_INTERFACE

	// IUnknown members
	STDMETHOD(QueryInterface)(REFIID iid, void** ppvObject);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release());

	// IDropTarget members
	STDMETHOD(DragEnter)(IDataObject* pDataObject, DWORD grfKeyState, POINTL Point, LPDWORD pdwEffect);
	STDMETHOD(DragOver)(DWORD grfKeyState, POINTL Point, LPDWORD pdwEffect);
	STDMETHOD(DragLeave)();
	STDMETHOD(Drop)(IDataObject* pDataObject, DWORD grfKeyState, POINTL Point, LPDWORD pdwEffect);

	END_INTERFACE

protected:
	HRESULT ImportFromFS(HDROP hDrop, DWORD dwEffect, CWnd* pWnd=NULL) const;
	HRESULT ImportFromStore(HLIQUIDFILES hLiquidFiles, IDataObject* pDataObject, CWnd* pWnd=NULL) const;
	HRESULT AddToClipboard(HGLOBAL hgLiquidFiles, CWnd* pWnd=NULL) const;

	LONG m_lRefCount;
	IDropTargetHelper* m_pDropTargetHelper;
	CWnd* p_OwnerWnd;
	LFSearchResult* p_SearchResult;
	STOREID m_StoreID;
	BOOL m_SkipTemplate;
	BOOL m_AllowChooseStore;
	BOOL m_IsDragSource;
	BOOL m_IsDropAllowed;
};

inline void LFDropTarget::SetDragSource(BOOL IsDragSource)
{
	m_IsDragSource = IsDragSource;
}

inline void LFDropTarget::SetOwner(CWnd* pOwnerWnd)
{
	p_OwnerWnd = pOwnerWnd;
}

inline void LFDropTarget::SetSearchResult(LFSearchResult* pSearchResult)
{
	p_SearchResult = pSearchResult;
}
