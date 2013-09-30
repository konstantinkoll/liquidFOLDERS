
#pragma once
#include "LFCommDlg.h"
#include "liquidFOLDERS.h"


// Virtuelle Attribute
//

#define AttrSource                  LFAttributeCount+0
#define AttrDriveLetter             LFAttributeCount+1
#define AttrMaintenanceTime         LFAttributeCount+2
#define AttrLastSeen                LFAttributeCount+3
#define AttrGUID                    LFAttributeCount+4
#define AttrPathData                LFAttributeCount+5
#define AttrPathIdxMain             LFAttributeCount+6
#define AttrPathIdxAux              LFAttributeCount+7
#define AttrIndexVersion            LFAttributeCount+8
#define AttrIATAAirportName         LFAttributeCount+9
#define AttrIATAAirportCountry      LFAttributeCount+10

#define AttrCount                   LFAttributeCount+11


// CIconHeader
//

#define IconEmpty         0
#define IconMultiple      1
#define IconCore          2
#define IconExtension     3
#define IconPreview       4

class CIconHeader : public CInspectorHeader
{
public:
	CIconHeader();

	virtual INT GetPreferredHeight();
	virtual void DrawHeader(CDC& dc, CRect rect, BOOL Themed);

	void SetEmpty();
	void SetMultiple(CString Description=_T(""));
	void SetCoreIcon(INT IconID, CString Description=_T(""));
	void SetFormatIcon(CHAR* FileFormat, CString Description=_T(""));
	void SetPreview(LFItemDescriptor* i, CString Description=_T(""));

protected:
	void FreeItem();

	CGdiPlusBitmapResource m_Empty;
	CGdiPlusBitmapResource m_Multiple;
	CString m_strUnused;
	CString m_strDescription;
	UINT m_Status;
	INT m_IconID;
	CHAR m_FileFormat[LFExtSize];
	LFItemDescriptor* m_pItem;
};


// CInspectorWnd
//

class CInspectorWnd : public CGlassPane
{
public:
	CInspectorWnd();

	virtual void AdjustLayout();
	virtual void SaveSettings();

	void UpdateStart();
	void UpdateAdd(LFItemDescriptor* i, LFSearchResult* pRawFiles);
	void UpdateFinish();

protected:
	CIconHeader m_IconHeader;
	CInspectorGrid m_wndInspectorGrid;
	BOOL m_ShowPreview;
	BOOL m_ShowInternal;
	BOOL m_SortAlphabetic;
	LFItemDescriptor* p_LastItem;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg LRESULT OnPropertyChanged(WPARAM wparam, LPARAM lparam);

	afx_msg void OnTogglePreview();
	afx_msg void OnToggleInternal();
	afx_msg void OnAlphabetic();
	afx_msg void OnExportSummary();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	UINT m_Count;
	UINT m_IconID;
	UINT m_IconStatus;
	UINT m_TypeID;
	UINT m_TypeStatus;
	UINT m_AttributeVisible[AttrCount];
	UINT m_AttributeStatus[AttrCount];
	BOOL m_AttributeEditable[AttrCount];
	LFVariantData m_AttributeValues[AttrCount];
	CString m_AttributeVirtualNames[AttrCount-LFAttributeCount];
	CString m_TypeName;

	void AddValue(LFItemDescriptor* i, UINT Attr, BOOL Editable=TRUE);
	void AddValueVirtual(UINT Attr, CHAR* Value);
	void AddValueVirtual(UINT Attr, WCHAR* Value);
};
