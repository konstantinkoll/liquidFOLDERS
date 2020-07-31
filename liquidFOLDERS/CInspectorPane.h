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

	void ResetAttribute(ATTRIBUTE Attr);
	void Reset(ITEMCONTEXT OriginalContext=LFContextAllFiles);
	void AddValueVirtual(ATTRIBUTE Attr, const LPCWSTR pStrValue);
	void AddValueVirtual(ATTRIBUTE Attr, const LPCSTR pStrValue);
	void AddValueVirtual(ATTRIBUTE Attr, const INT64 Size);
	void AddItem(const LFItemDescriptor* pItemDescriptor, const LFSearchResult* pRawFiles);
	void UpdateIATAAirport(BOOL AllowMultiple);
	UINT GetFileCount() const;
	UINT GetCompressibleFileCount() const;

	AttributeSummary m_AttributeSummary[AttrCount];
	const LFItemDescriptor* p_LastItem;
	UINT m_ItemCount;
	UINT m_CompressibleItemCount;
	UINT m_ContextStatus;
	ITEMCONTEXT m_OriginalContext;
	ITEMCONTEXT m_ItemContext;
	UINT m_IconStatus;
	UINT m_IconID;
	BOOL m_StateFirst;
	BYTE m_StateSet;
	BYTE m_MultipleStates;
	BYTE m_Type;
	UINT m_StoreStatus;
	STOREID m_StoreID;
	BOOL m_AggregatedFiles;

protected:
	void AddValue(const LFItemDescriptor* pItemDescriptor, ATTRIBUTE Attr);

private:
	void AddFile(const LFItemDescriptor* pItemDescriptor);
};

inline UINT CFileSummary::GetFileCount() const
{
	return m_AggregatedFiles ? m_ItemCount : 0;
}

inline UINT CFileSummary::GetCompressibleFileCount() const
{
	return m_AggregatedFiles ? m_CompressibleItemCount : 0;
}


// CIconHeader
//

class CIconHeader sealed : public CInspectorHeader
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

	virtual INT GetMinWidth() const;
	virtual void AdjustLayout(CRect rectLayout);

	void AggregateInitialize(ITEMCONTEXT Context);
	void AggregateAdd(const LFItemDescriptor* pItemDescriptor, const LFSearchResult* pRawFiles);
	void AggregateClose();
	UINT GetFileCount() const;
	UINT GetCompressibleFileCount() const;

protected:
	void SaveSettings() const;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
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
	void UpdatePropertyState(ATTRIBUTE Attr);
	void UpdateIATAAirport(BOOL AllowMultiple=TRUE);

	static CString m_AttributeVirtualNames[AttrCount-LFAttributeCount];
	CString m_TypeName;
};

inline void CInspectorPane::AggregateInitialize(ITEMCONTEXT Context)
{
	m_FileSummary.Reset(Context);
}

inline void CInspectorPane::AggregateAdd(const LFItemDescriptor* pItemDescriptor, const LFSearchResult* pRawFiles)
{
	m_FileSummary.AddItem(pItemDescriptor, pRawFiles);
}

inline UINT CInspectorPane::GetFileCount() const
{
	return m_FileSummary.GetFileCount();
}

inline UINT CInspectorPane::GetCompressibleFileCount() const
{
	return m_FileSummary.GetCompressibleFileCount();
}
