
// LFItemTemplateDlg.h: Schnittstelle der Klasse LFItemTemplateDlg
//

#pragma once
#include "LFCore.h"
#include "LFApplication.h"
#include "CAttributeProperties.h"
#include "CInspectorGrid.h"


// LFItemTemplateDlg
//

class AFX_EXT_CLASS LFItemTemplateDlg : public CDialog
{
public:
	LFItemTemplateDlg(CWnd* pParentWnd, LFItemDescriptor* pItem, char* _StoreID);

	virtual void DoDataExchange(CDataExchange* pDX);

	LFItemDescriptor* m_pItem;

protected:
	LFApplication* p_App;
	CMFCPropertyGridProperty* pGroups[LFAttrCategoryCount];
	CAttributeProperty* pAttributes[LFAttributeCount];

	afx_msg BOOL OnInitDialog();
	afx_msg void OnSortAlphabetically();
	afx_msg void OnReset();
	afx_msg void OnSkip();
	DECLARE_MESSAGE_MAP()

private:
	char StoreID[LFKeySize];
	LFVariantData AttributeValues[LFAttributeCount];
	CInspectorGrid m_Inspector;
};
