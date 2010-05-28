
#pragma once
#include "CIconCtrl.h"
#include "LFCommDlg.h"
#include "liquidFOLDERS.h"


// CInspectorToolBar
//

class CInspectorToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*)GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList()
	{
		return FALSE;
	}
};


// Virtuelle Attribute
//

#define AttrDriveLetter             LFAttributeCount+0
#define AttrMaintenanceTime         LFAttributeCount+1
#define AttrLastSeen                LFAttributeCount+2
#define AttrGUID                    LFAttributeCount+3
#define AttrPathData                LFAttributeCount+4
#define AttrPathIdxMain             LFAttributeCount+5
#define AttrPathIdxAux              LFAttributeCount+6
#define AttrIndexVersion            LFAttributeCount+7
#define AttrIATAAirportName         LFAttributeCount+8
#define AttrIATAAirportCountry      LFAttributeCount+9

#define AttrCount                   LFAttributeCount+10


// CInspectorWnd
//

class CInspectorWnd : public CDockablePane
{
public:
	CInspectorWnd();
	virtual ~CInspectorWnd();

	virtual void AdjustLayout();
	virtual void SaveSettings();

	void UpdateStart(BOOL Reset=TRUE);
	void UpdateAdd(LFItemDescriptor* i);
	void UpdateFinish();

protected:
	CInspectorToolBar m_wndToolBar;
	CIconCtrl m_wndIconCtrl;
	CInspectorGrid m_wndPropList;
	CMFCPropertyGridProperty* pGroups[LFAttrCategoryCount];
	CMFCPropertyGridProperty* pAttributes[AttrCount];
	BOOL m_ShowIcon;
	BOOL m_Alphabetic;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnPaint();
	afx_msg LRESULT OnPropertyChanged(WPARAM wparam, LPARAM lparam);
	afx_msg void OnToggleIcon();
	afx_msg void OnTreeView();
	afx_msg void OnAlphabetic();
	afx_msg void OnExpandAll();
	afx_msg void OnExport();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	UINT Count;

	UINT IconID;
	UINT IconStatus;
	
	UINT TypeID;
	UINT TypeStatus;

	UINT AttributeVisible[AttrCount];
	UINT AttributeStatus[AttrCount];
	UINT AttributeCategory[AttrCount];
	BOOL AttributeEditable[AttrCount];
	LFVariantData AttributeValues[AttrCount];
	CString AttributeVirtualNames[AttrCount-LFAttributeCount];

	CString TypeName;

	void AddValue(LFItemDescriptor* i, UINT Attr, BOOL Editable=TRUE);
	void AddValueVirtual(UINT Attr, char* Value, BOOL Editable=TRUE);
	void AddValueVirtual(UINT Attr, wchar_t* Value, BOOL Editable=TRUE);
};
