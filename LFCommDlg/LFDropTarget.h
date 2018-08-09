
// LFDropTarget.h: Schnittstelle der Klasse LFDropTarget
//

#pragma once
#include "CBackstageDropTarget.h"
#include "LFCore.h"


// LFDropTarget
//

class LFDropTarget : public CBackstageDropTarget
{
public:
	LFDropTarget();

	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT DropEffect, CPoint point);

	void SetDragSource(BOOL IsDragSource);
	void SetStore(const ABSOLUTESTOREID& StoreID);
	void SetStore(LFFilter* pFilter);
	void SetSearchResult(LFSearchResult* pSearchResult);

protected:
	BOOL IsClipboard() const;
	static LRESULT SendDroppedMessage(CWnd* pWnd);
	BOOL ImportFromFS(CWnd* pWnd, HDROP hDrop, DROPEFFECT DropEffect) const;
	BOOL ImportFromStore(CWnd* pWnd, HLIQUIDFILES hLiquidFiles, COleDataObject* pDataObject) const;
	BOOL AddToClipboard(CWnd* pWnd, HGLOBAL hgLiquidFiles) const;

	LFSearchResult* p_SearchResult;
	STOREID m_StoreID;
	BOOL m_SkipTemplate;
	BOOL m_AllowChooseStore;
	BOOL m_IsDragSource;
	BOOL m_IsDropAllowed;

private:
	WCHAR m_StoreName[256];
	static CString m_strDefaultStore;
	static CString m_strImportCopy;
	static CString m_strImportMove;
	static CString m_strRemember;
	static CString m_strClipboard;
};

inline void LFDropTarget::SetDragSource(BOOL IsDragSource)
{
	m_IsDragSource = IsDragSource;
}

inline void LFDropTarget::SetSearchResult(LFSearchResult* pSearchResult)
{
	p_SearchResult = pSearchResult;
}

inline BOOL LFDropTarget::IsClipboard() const
{
	return p_SearchResult!=NULL;
}

inline LRESULT LFDropTarget::SendDroppedMessage(CWnd* pWnd)
{
	return pWnd ? pWnd->GetTopLevelParent()->SendMessage(LFGetMessageIDs()->ItemsDropped) : NULL;
}
