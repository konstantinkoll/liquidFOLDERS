
// CListView.h: Schnittstelle der Klasse CListView
//

#pragma once
#include "liquidFOLDERS.h"
#include "StoreManager.h"
#include "CAbstractListView.h"


// CListView

class CListView : public CAbstractListView
{
public:
	CListView();
	virtual ~CListView();

	void Create(CWnd* pParentWnd, LFSearchResult* _result, UINT _ViewID);

protected:
	virtual void SetViewOptions(UINT _ViewID, BOOL Force);
	virtual void SetSearchResult(LFSearchResult* _result);
	virtual CMenu* GetContextMenu();

	void AdjustLayout();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};
