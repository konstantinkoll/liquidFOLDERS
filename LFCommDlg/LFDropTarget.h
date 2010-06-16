#pragma once
#include "afxole.h"

class AFX_EXT_CLASS LFDropTarget : public COleDropTarget
{
public:
	LFDropTarget();
	~LFDropTarget();

	virtual BOOL Register(CWnd* pWnd, char* StoreID);
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);

protected:
	char m_StoreID[LFKeySize];
	BOOL m_SkipTemplate;
};
