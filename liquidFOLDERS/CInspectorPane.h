
#pragma once
#include "LFCommDlg.h"


// Virtuelle Attribute
//

#define AttrSource                  LFAttributeCount+0
#define AttrMaintenanceTime         LFAttributeCount+1
#define AttrSynchronizeTime         LFAttributeCount+2
#define AttrLastSeen                LFAttributeCount+3
#define AttrIATAAirportName         LFAttributeCount+4
#define AttrIATAAirportCountry      LFAttributeCount+5

#define AttrCount                   LFAttributeCount+6


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
	~CIconHeader();

	virtual INT GetPreferredHeight() const;
	virtual void DrawHeader(CDC& dc, Graphics& g, const CRect& rect, BOOL Themed) const;

	void SetEmpty();
	void SetMultiple(const CString& Description=_T(""));
	void SetCoreIcon(INT IconID, const CString& Description=_T(""));
	void SetFormatIcon(LPCSTR FileFormat, const CString& Description=_T(""));
	void SetPreview(LFItemDescriptor* pItemDescriptor, const CString& Description=_T(""));

protected:
	void FreeItem();

	static CString m_strNoItemsSelected;
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
	virtual void ScrollWindow(INT dx, INT dy, LPCRECT lpRect=NULL, LPCRECT lpClipRect=NULL);
};


// CInspectorPane
//

class CInspectorPane : public CFrontstagePane
{
public:
	CInspectorPane();

	virtual void AdjustLayout(CRect rectLayout);
	virtual void SaveSettings();

	void UpdateStart();
	void UpdateAdd(LFItemDescriptor* pItemDescriptor, LFSearchResult* pRawFiles);
	void UpdateFinish();

protected:
	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);

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
	void AddValue(LFItemDescriptor* pItemDescriptor, UINT Attr, BOOL Editable=FALSE);
	void AddValueVirtual(UINT Attr, const LPCSTR Value);
	void AddValueVirtual(UINT Attr, const LPCWSTR Value);

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
	static CString m_AttributeVirtualNames[AttrCount-LFAttributeCount];
	CString m_TypeName;
};
