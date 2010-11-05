
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

	void Create(CWnd* pParentWnd, LFSearchResult* _result, UINT _ViewID, INT _FocusItem);

protected:
	virtual void SetViewOptions(UINT _ViewID, BOOL Force);
	virtual void SetSearchResult(LFSearchResult* _result);

	void AdjustLayout();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	DECLARE_MESSAGE_MAP()

	DECLARE_INTERFACE_MAP()
	BEGIN_INTERFACE_PART(FooterCallback, IListViewFooterCallback)
		STDMETHOD(OnButtonClicked)(INT, LPARAM, PINT);
		STDMETHOD(OnDestroyButton)(INT, LPARAM);
	END_INTERFACE_PART(FooterCallback)

	BOOL m_HasCategories;
};
