
// LFItemTemplateDlg.h: Schnittstelle der Klasse LFItemTemplateDlg
//

#pragma once
#include "LFCore.h"
#include "LFApplication.h"
#include "CInspectorGrid.h"
#include "CFrameCtrl.h"


// LFItemTemplateDlg
//

class AFX_EXT_CLASS LFItemTemplateDlg : public CDialog
{
public:
	LFItemTemplateDlg(CWnd* pParentWnd, LFItemDescriptor* pItem, CHAR* StoreID);

	virtual void DoDataExchange(CDataExchange* pDX);

	LFItemDescriptor* m_pItem;

protected:
	LFApplication* p_App;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnSortAlphabetically();
	afx_msg void OnReset();
	afx_msg void OnSkip();
	DECLARE_MESSAGE_MAP()

private:
	CHAR m_StoreID[LFKeySize];
	LFVariantData m_AttributeValues[LFAttributeCount];
	CInspectorGrid m_wndInspectorGrid;
	CFrameCtrl m_FrameCtrl;
};
