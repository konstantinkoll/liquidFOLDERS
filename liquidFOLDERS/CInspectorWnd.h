
#pragma once
#include "LFCommDlg.h"


// Virtuelle Attribute
//

#define AttrSource                  LFAttributeCount+0
#define AttrDriveLetter             LFAttributeCount+1
#define AttrMaintenanceTime         LFAttributeCount+2
#define AttrSynchronizeTime         LFAttributeCount+3
#define AttrLastSeen                LFAttributeCount+4
#define AttrIATAAirportName         LFAttributeCount+5
#define AttrIATAAirportCountry      LFAttributeCount+6

#define AttrCount                   LFAttributeCount+7


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


// CStoreManagerGrid
//

class CStoreManagerGrid : public CInspectorGrid
{
protected:
	virtual void ScrollWindow(INT dx, INT dy);
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
	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg LRESULT OnPropertyChanged(WPARAM wparam, LPARAM lparam);

	afx_msg void OnToggleInternal();
	afx_msg void OnAlphabetic();
	afx_msg void OnExportSummary();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CIconHeader m_IconHeader;
	CStoreManagerGrid m_wndGrid;
	BOOL m_ShowInternal;
	BOOL m_SortAlphabetic;
	LFItemDescriptor* p_LastItem;

private:
	void AddValue(LFItemDescriptor* i, UINT Attr, BOOL Editable=FALSE);
	void AddValueVirtual(UINT Attr, CHAR* Value);
	void AddValueVirtual(UINT Attr, WCHAR* Value);

	UINT m_Count;
	UINT m_IconID;
	UINT m_IconStatus;
	UINT m_TypeID;
	UINT m_AttributeVisible[AttrCount];
	UINT m_AttributeStatus[AttrCount];
	BOOL m_AttributeEditable[AttrCount];
	LFVariantData m_AttributeValues[AttrCount];
	LFVariantData m_AttributeRangeFirst[AttrCount];
	LFVariantData m_AttributeRangeSecond[AttrCount];
	CString m_AttributeVirtualNames[AttrCount-LFAttributeCount];
	CString m_TypeName;
};
