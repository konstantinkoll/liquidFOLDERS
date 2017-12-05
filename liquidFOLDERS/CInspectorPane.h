
#pragma once
#include "LFCommDlg.h"


// CFileSummary
//

#define AttrFlags                  LFAttributeCount+0
#define AttrSource                 LFAttributeCount+1
#define AttrTotalBytes             LFAttributeCount+2
#define AttrTotalBytesFree         LFAttributeCount+3
#define AttrFreeBytesAvailable     LFAttributeCount+4
#define AttrMaintenanceTime        LFAttributeCount+5
#define AttrSynchronizeTime        LFAttributeCount+6
#define AttrLastSeen               LFAttributeCount+7
#define AttrIATAAirportName        LFAttributeCount+8
#define AttrIATAAirportCountry     LFAttributeCount+9

#define AttrCount                  LFAttributeCount+10

struct AttributeSummary
{
	BOOL Visible;
	UINT Status;
	LFVariantData VData;
	LFVariantData VDataRange;
};

class CFileSummary
{
public:
	CFileSummary();

	void ResetAttribute(UINT Attr);
	void Reset(UINT Context=LFContextAllFiles);
	void AddValueVirtual(UINT Attr, const LPCWSTR pStrValue);
	void AddValueVirtual(UINT Attr, const LPCSTR pStrValue);
	void AddValueVirtual(UINT Attr, const INT64 Size);
	void AddItem(const LFItemDescriptor* pItemDescriptor, const LFSearchResult* pRawFiles);
	void UpdateIATAAirport(BOOL AllowMultiple);

	AttributeSummary m_AttributeSummary[AttrCount];
	UINT m_Context;
	const LFItemDescriptor* p_LastItem;
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
	void AddValue(const LFItemDescriptor* pItemDescriptor, UINT Attr);

private:
	void AddFile(const LFItemDescriptor* pItemDescriptor);
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
	void SetPreview(const LFItemDescriptor* pItemDescriptor, const CString& Description=_T(""));
	BOOL UpdateThumbnailColor(const LFVariantData& VData);

protected:
	void FreeItem();

	static CString m_NoItemsSelected;
	CString m_Description;
	UINT m_Status;
	INT m_IconID;
	CHAR m_FileFormat[LFExtSize];
	LFItemDescriptor* m_pItemDesciptor;
};


// CInspectorPane
//

class CInspectorPane : public CFrontstagePane
{
public:
	CInspectorPane();

	virtual INT GetMinWidth(INT Height) const;
	virtual void AdjustLayout(CRect rectLayout);

	void AggregateInitialize(UINT Context);
	void AggregateAdd(const LFItemDescriptor* pItemDescriptor, const LFSearchResult* pRawFiles);
	void AggregateClose();

protected:
	void SaveSettings() const;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);

	afx_msg void OnToggleInternal();
	afx_msg void OnAlphabetic();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CFileSummary m_FileSummary;
	CIconHeader m_IconHeader;
	CInspectorGrid m_wndInspectorGrid;
	BOOL m_ShowInternal;
	BOOL m_SortAlphabetic;

private:
	void UpdatePropertyState(UINT Attr);
	void UpdateIATAAirport(BOOL AllowMultiple=TRUE);

	static CString m_AttributeVirtualNames[AttrCount-LFAttributeCount];
	CString m_TypeName;
};

inline void CInspectorPane::AggregateInitialize(UINT Context)
{
	m_FileSummary.Reset(Context);
	m_wndInspectorGrid.SetContext(Context);
}

inline void CInspectorPane::AggregateAdd(const LFItemDescriptor* pItemDescriptor, const LFSearchResult* pRawFiles)
{
	m_FileSummary.AddItem(pItemDescriptor, pRawFiles);
}
