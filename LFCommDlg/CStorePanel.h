
// CStorePanel.h: Schnittstelle der Klasse CStorePanel
//

#pragma once
#include "LFCore.h"


// CStorePanel
//

class CStorePanel : public CWnd
{
public:
	CStorePanel();
	~CStorePanel();

	virtual void PreSubclassWindow();

	void SetStore(const CHAR* StoreID);
	BOOL IsValidStore() const;

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

	INT m_IconSizeX;
	INT m_IconSizeY;
	CImageList* p_Icons;
	LFItemDescriptor* p_Item;
};
