#pragma once
#include "LFCore.h"
#include "LFApplication.h"
#include "CAttributeProperties.h"
#include "CInspectorGrid.h"

class AFX_EXT_CLASS LFItemTemplateDlg : public CDialog
{
public:
	LFItemTemplateDlg(CWnd* pParentWnd, LFItemDescriptor* pItem);
	virtual ~LFItemTemplateDlg();

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
	LFVariantData AttributeValues[LFAttributeCount];
	CInspectorGrid m_Inspector;
};
