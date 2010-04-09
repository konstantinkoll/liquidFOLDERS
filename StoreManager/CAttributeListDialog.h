#pragma once
#include "StoreManager.h"

#define ALD_Mode_ShowAttributes   0
#define ALD_Mode_SortAttribute    1

class CAttributeListDialog : public CDialog
{
public:
	CAttributeListDialog(UINT nIdTemplate, CWnd* pParentWnd);
	virtual ~CAttributeListDialog();

protected:
	CImageListTransparent* m_pAttributeIcons;

	void PopulateListCtrl(int nId, UINT mode, UINT context, LFViewParameters* vp);
};
