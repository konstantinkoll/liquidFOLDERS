#pragma once
#include "afxole.h"

class AFX_EXT_CLASS LFDropTarget : public COleDropTarget
{
public:
	LFDropTarget();
	~LFDropTarget();

	virtual BOOL Register(CWnd* pWnd);
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);

protected:
	BOOL SkipTemplate;
};
