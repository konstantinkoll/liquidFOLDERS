
// LFDropTarget.h: Schnittstelle der Klasse LFDropTarget
//

#pragma once
#include "afxole.h"


// LFDropTarget
//

class AFX_EXT_CLASS LFDropTarget : public COleDropTarget
{
public:
	LFDropTarget();

	virtual BOOL Register(CWnd* pWnd, CHAR* StoreID, BOOL AllowChooseStore);
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);

protected:
	CHAR m_StoreID[LFKeySize];
	BOOL m_AllowChooseStore;
	BOOL m_SkipTemplate;
};
