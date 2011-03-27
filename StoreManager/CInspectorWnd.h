
#pragma once
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
	//void SetPreview(, CString Description=_T(""));		TODO

protected:
	CGdiPlusBitmapResource m_Empty;
	CGdiPlusBitmapResource m_Multiple;
	CString m_strUnused;
	CString m_strDescription;
	UINT m_Status;
	INT m_IconID;
	CHAR m_FileFormat[LFExtSize];
};


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
	CIconHeader m_IconHeader;
	CInspectorGrid m_wndInspectorGrid;
	BOOL m_ShowPreview;
	BOOL m_SortAlphabetic;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg LRESULT OnPropertyChanged(WPARAM wparam, LPARAM lparam);

	afx_msg void OnTogglePreview();
	afx_msg void OnAlphabetic();
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
	BOOL AttributeEditable[AttrCount];
	LFVariantData AttributeValues[AttrCount];
	CString AttributeVirtualNames[AttrCount-LFAttributeCount];

	CString TypeName;

	void AddValue(LFItemDescriptor* i, UINT Attr, BOOL Editable=TRUE);
	void AddValueVirtual(UINT Attr, CHAR* Value);
	void AddValueVirtual(UINT Attr, WCHAR* Value);
};
