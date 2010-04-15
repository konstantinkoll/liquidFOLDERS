#pragma once
#include "afxole.h"

class CLFDropTarget : public COleDropTarget
{
public:
    DROPEFFECT OnDragEnter ( CWnd* pWnd, COleDataObject* pDataObject,
                             DWORD dwKeyState, CPoint point );

    DROPEFFECT OnDragOver ( CWnd* pWnd, COleDataObject* pDataObject,
                            DWORD dwKeyState, CPoint point );

    BOOL OnDrop ( CWnd* pWnd, COleDataObject* pDataObject,
                  DROPEFFECT dropEffect, CPoint point );

    void OnDragLeave ( CWnd* pWnd );

	CLFDropTarget(CDialog* parent);
	~CLFDropTarget(void);

protected:
    CDialog* m_pParent;  // initialized in constructor
};
