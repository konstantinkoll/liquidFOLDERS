
#pragma once
#include "LFCommDlg.h"


// CFileSummary
//

#define AttrFlags                  LFAttributeCount+0
#define AttrSource                 LFAttributeCount+1
#define AttrMaintenanceTime        LFAttributeCount+2
#define AttrSynchronizeTime        LFAttributeCount+3
#define AttrLastSeen               LFAttributeCount+4
#define AttrIATAAirportName        LFAttributeCount+5
#define AttrIATAAirportCountry     LFAttributeCount+6

#define AttrCount                  LFAttributeCount+7

struct AttributeSummary
{
	BOOL Visible;
	UINT Status;
	LFVariantData Value;
	LFVariantData RangeFirst;
	LFVariantData RangeSecond;
};

class CFileSummary
{
public:
	CFileSummary();

	void Reset(INT Context=LFContextAllFiles);
	void AddValueVirtual(UINT Attr, const LPCWSTR pStrValue);
	void AddValueVirtual(UINT Attr, const LPCSTR pStrValue);
	void AddItem(LFItemDescriptor* pItemDescriptor, LFSearchResult* pRawFiles);

	AttributeSummary m_AttributeSummary[AttrCount];
	INT m_Context;
	LFItemDescriptor* p_LastItem;
	UINT m_ItemCount;
	UINT m_IconStatus;
	UINT m_IconID;
	BOOL m_FlagsFirst;
	UINT m_FlagsSet;
	UINT m_FlagsMultiple;
	UINT m_StoreStatus;
	CHAR m_StoreID[LFKeySize];
	UINT m_Type;

protected:
	void AddValue(LFItemDescriptor* pItemDescriptor, UINT Attr);

private:
	void AddFile(LFItemDescriptor* pItemDescriptor);
};


// CIconHeader
//

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
	void SetFormatIcon(LPCSTR pFileFormat, const CString& Description=_T(""));
	void SetPreview(LFItemDescriptor* pItemDescriptor, const CString& Description=_T(""));

protected:
	void FreeItem();

	static CString m_NoItemsSelected;
	CString m_Description;
	UINT m_Status;
	INT m_IconID;
	CHAR m_FileFormat[LFExtSize];
	LFItemDescriptor* m_pItem;
};


// CInspectorPane
//

class CInspectorPane : public CFrontstagePane
{
public:
	CInspectorPane();

	virtual void AdjustLayout(CRect rectLayout);
	virtual void SaveSettings();

	void AggregateStart(INT Context);
	void AggregateAdd(LFItemDescriptor* pItemDescriptor, LFSearchResult* pRawFiles);
	void AggregateFinish();

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

	CFileSummary m_FileSummary;
	CIconHeader m_IconHeader;
	CInspectorGrid m_wndGrid;
	BOOL m_ShowInternal;
	BOOL m_SortAlphabetic;

private:
	static CString m_AttributeVirtualNames[AttrCount-LFAttributeCount];
	CString m_TypeName;
};

inline void CInspectorPane::AggregateStart(INT Context)
{
	m_FileSummary.Reset(Context);
}

inline void CInspectorPane::AggregateAdd(LFItemDescriptor* pItemDescriptor, LFSearchResult* pRawFiles)
{
	m_FileSummary.AddItem(pItemDescriptor, pRawFiles);
}
