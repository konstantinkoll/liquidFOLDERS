
#pragma once
#include "CInspectorIconCtrl.h"
#include "LFCommDlg.h"
#include "liquidFOLDERS.h"


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

class CInspectorWnd : public CGlasPane
{
public:
	CInspectorWnd();

	virtual void AdjustLayout();
	virtual void SaveSettings();

	void UpdateStart(CHAR* StoreID);
	void UpdateAdd(LFItemDescriptor* i, LFSearchResult* raw);
	void UpdateFinish();

protected:
	CInspectorIconCtrl m_wndIconCtrl;
	CInspectorGrid m_wndPropList;
	CMFCPropertyGridProperty* pGroups[LFAttrCategoryCount];
	CMFCPropertyGridProperty* pAttributes[AttrCount];
	BOOL m_ShowIcon;
	BOOL m_Alphabetic;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
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
	void AddValueVirtual(UINT Attr, CHAR* Value);
	void AddValueVirtual(UINT Attr, WCHAR* Value);
};