
// CExplorerList: Schnittstelle der Klasse CExplorerList
//

#pragma once
#include "liquidFOLDERS.h"
#include "LFApplication.h"


// CExplorerList
//

class AFX_EXT_CLASS CExplorerList : public CListCtrl
{
public:
	CExplorerList();
	~CExplorerList();

	void EnableTheming();

protected:
	LFApplication* p_App;
	HTHEME hTheme;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	DECLARE_MESSAGE_MAP()
};
