
// CTagcloudView.h: Schnittstelle der Klasse CTagcloudView
//

#pragma once
#include "liquidFOLDERS.h"
#include "StoreManager.h"
#include "CFileView.h"


struct OneTag
{
	wchar_t Text[256];
	UINT Count;
	LFFilter* NextFilter;
};


// CTagcloudView
//

class CTagcloudView : public CFileView
{
public:
	CTagcloudView();
	virtual ~CTagcloudView();

	void Create(CWnd* _pParentWnd, LFSearchResult* _result);

protected:

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
