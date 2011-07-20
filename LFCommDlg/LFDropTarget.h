
// LFDropTarget.h: Schnittstelle der Klasse LFDropTarget
//

#pragma once
#include "afxole.h"
#include "liquidFOLDERS.h"


// LFDropTarget
//

class AFX_EXT_CLASS LFDropTarget : public COleDropTarget
{
public:
	LFDropTarget();

	BOOL Register(CWnd* pWnd, CHAR* StoreID, BOOL AllowChooseStore);
	BOOL Register(CWnd* pWnd, LFFilter* pFilter, BOOL AllowChooseStore);
	DROPEFFECT CheckDrop(COleDataObject* pDataObject, DWORD dwKeyState);

	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);

protected:
	LFFilter* p_Filter;
	CHAR m_StoreID[LFKeySize];
	BOOL m_StoreIDValid;
	BOOL m_AllowChooseStore;
	BOOL m_SkipTemplate;
};
