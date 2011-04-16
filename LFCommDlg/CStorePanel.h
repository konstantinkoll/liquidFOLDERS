
// CStorePanel.h: Schnittstelle der Klasse CStorePanel
//

#pragma once
#include "LFApplication.h"


// CStorePanel
//

class AFX_EXT_CLASS CStorePanel : public CWnd
{
public:
	CStorePanel();
	~CStorePanel();

	virtual void PreSubclassWindow();

	void SetStore(CHAR* Key);
	BOOL IsValidStore();

protected:
	LFApplication* p_App;
	INT m_IconSizeX;
	INT m_IconSizeY;
	CImageList* p_Icons;
	LFItemDescriptor* p_Item;

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
