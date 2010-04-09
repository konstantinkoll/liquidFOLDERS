
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

	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);

	#ifdef _DEBUG
	void CListView::AssertValid() const
	{
	}
	#endif
};
