
// CInspectorPane.cpp: Implementierung der Klasse CInspectorPane
//

#include "stdafx.h"
#include "liquidFOLDERS.h"


// CFileSummary
//

#define STATUSUNUSED       0
#define STATUSUSED         1
#define STATUSMULTIPLE     2

CFileSummary::CFileSummary()
{
	Reset();
}

void CFileSummary::ResetAttribute(UINT Attr)
{
	ASSERT(Attr<AttrCount);

	m_AttributeSummary[Attr].Status = m_AttributeSummary[Attr].Visible = FALSE;

	LFInitVariantData(m_AttributeSummary[Attr].VData, Attr);
	LFInitVariantData(m_AttributeSummary[Attr].VDataRange, Attr);
}

void CFileSummary::Reset(UINT Context)
{
	m_Context = Context;
	p_LastItem = NULL;
	m_ItemCount = 0;

	// Icon, flags and store
	m_IconStatus = m_StoreStatus = STATUSUNUSED;
	m_FlagsFirst = TRUE;
	m_FlagsSet = m_FlagsMultiple = 0;
	m_AggregatedFiles = TRUE;

	// Attribute summaries
	for (UINT a=0; a<AttrCount; a++)
		ResetAttribute(a);
}

void CFileSummary::AddValueVirtual(UINT Attr, const LPCWSTR pStrValue)
{
	ASSERT(Attr<AttrCount);
	ASSERT(pStrValue);

	AttributeSummary* pAttributeSummary = &m_AttributeSummary[Attr];

	switch (pAttributeSummary->Status)
	{
	case STATUSUNUSED:
		wcsncpy_s(pAttributeSummary->VData.UnicodeString, 256, pStrValue, _TRUNCATE);
		pAttributeSummary->VData.IsNull = FALSE;

		pAttributeSummary->Status = STATUSUSED;
		pAttributeSummary->Visible = TRUE;

		break;

	case STATUSUSED:
		if (wcscmp(pAttributeSummary->VData.UnicodeString, pStrValue)!=0)
			pAttributeSummary->Status = STATUSMULTIPLE;

		break;
	}
}

void CFileSummary::AddValueVirtual(UINT Attr, const LPCSTR pStrValue)
{
	ASSERT(pStrValue);

	WCHAR tmpStr[256];
	MultiByteToWideChar(CP_ACP, 0, pStrValue, -1, tmpStr, 256);

	AddValueVirtual(Attr, tmpStr);
}

void CFileSummary::AddValueVirtual(UINT Attr, const INT64 Size)
{
	WCHAR tmpStr[256];
	LFSizeToString(Size, tmpStr, 256);

	AddValueVirtual(Attr, tmpStr);
}

void CFileSummary::AddValue(const LFItemDescriptor* pItemDescriptor, UINT Attr)
{
	ASSERT(pItemDescriptor);
	ASSERT(Attr<LFAttributeCount);

	if (theApp.IsAttributeAvailable(m_Context, Attr) && pItemDescriptor->AttributeValues[Attr])
	{
		AttributeSummary* pAttributeSummary = &m_AttributeSummary[Attr];

		LFVariantData Property;
		LFGetAttributeVariantDataEx(pItemDescriptor, Attr, Property);

		if (theApp.IsAttributeEditable(Attr) || !LFIsNullVariantData(Property))
			switch (pAttributeSummary->Status)
			{
			case STATUSUNUSED:
				pAttributeSummary->VData = Property;

				pAttributeSummary->Status = STATUSUSED;
				pAttributeSummary->Visible = TRUE;

				break;

			case STATUSUSED:
				if (LFCompareVariantData(pAttributeSummary->VData, Property)==0)
					return;

				// Set lower and upper range to first value
				pAttributeSummary->VDataRange = pAttributeSummary->VData;
				pAttributeSummary->Status = STATUSMULTIPLE;

				// Compare Property with range values

			case STATUSMULTIPLE:
				if (LFCompareVariantData(Property, pAttributeSummary->VData)<0)
					pAttributeSummary->VData = Property;

				if (LFCompareVariantData(Property, pAttributeSummary->VDataRange)>0)
					pAttributeSummary->VDataRange = Property;

				break;
			}
	}
}

void CFileSummary::AddFile(const LFItemDescriptor* pItemDescriptor)
{
	ASSERT(LFIsFile(pItemDescriptor));

	// Basic
	p_LastItem = pItemDescriptor;
	m_ItemCount++;

	// Flags
	if (m_FlagsFirst)
	{
		m_FlagsFirst = FALSE;
	}
	else
	{
		m_FlagsMultiple = m_FlagsSet ^ pItemDescriptor->CoreAttributes.Flags;
	}

	m_FlagsSet |= pItemDescriptor->CoreAttributes.Flags;

	// Color
	m_AttributeSummary[LFAttrColor].VData.ColorSet |= pItemDescriptor->AggregateColorSet;

	// Store
	if (m_StoreStatus<STATUSMULTIPLE)
		switch (m_StoreStatus)
		{
		case STATUSUNUSED:
			m_StoreStatus = STATUSUSED;
			m_StoreID = pItemDescriptor->StoreID;
			break;

		case STATUSUSED:
			if (m_StoreID!=pItemDescriptor->StoreID)
				m_StoreStatus = STATUSMULTIPLE;

			break;
		}

	// Attributes
	for (UINT a=0; a<LFAttributeCount; a++)
		AddValue(pItemDescriptor, a);

	// Virtual properties
	AddValueVirtual(AttrSource, theApp.m_SourceNames[pItemDescriptor->Type & LFTypeSourceMask][0]);
}

void CFileSummary::AddItem(const LFItemDescriptor* pItemDescriptor, const LFSearchResult* pRawFiles)
{
	ASSERT(pItemDescriptor);
	ASSERT(pRawFiles);

	p_LastItem = pItemDescriptor;

	// Icon
	if (m_IconStatus<STATUSMULTIPLE)
		switch (m_IconStatus)
		{
		case STATUSUNUSED:
			m_IconStatus = STATUSUSED;
			m_IconID = pItemDescriptor->IconID;

			break;

		case STATUSUSED:
			if (m_IconID!=pItemDescriptor->IconID)
				m_IconStatus = STATUSMULTIPLE;

			break;
		}

	// Type and properties
	switch (m_Type=LFGetItemType(pItemDescriptor))
	{
	case LFTypeFile:
		AddFile(pItemDescriptor);

		break;

	case LFTypeFolder:
		if ((pItemDescriptor->AggregateFirst!=-1) && (pItemDescriptor->AggregateLast!=-1))
		{
			for (INT a=pItemDescriptor->AggregateFirst; a<=pItemDescriptor->AggregateLast; a++)
				AddFile((*pRawFiles)[a]);
		}
		else
		{
			m_ItemCount += pItemDescriptor->AggregateCount;
			m_AggregatedFiles = FALSE;
		}

		break;

	case LFTypeStore:
		m_ItemCount++;
		m_AggregatedFiles = FALSE;

		for (UINT a=0; a<=LFLastCoreAttribute; a++)
			AddValue(pItemDescriptor, a);

		// Source
		AddValueVirtual(AttrSource, theApp.m_SourceNames[pItemDescriptor->StoreDescriptor.Source][0]);

		// Maintenance time
		WCHAR tmpStr[256];
		LFTimeToString(pItemDescriptor->StoreDescriptor.MaintenanceTime, tmpStr, 256);
		AddValueVirtual(AttrMaintenanceTime, tmpStr);

		// Synchronize time
		LFTimeToString(pItemDescriptor->StoreDescriptor.SynchronizeTime, tmpStr, 256);
		if (tmpStr[0]!=L'\0')
			AddValueVirtual(AttrSynchronizeTime, tmpStr);

		// Last seen
		if ((pItemDescriptor->StoreDescriptor.Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal)
			AddValueVirtual(AttrLastSeen, pItemDescriptor->StoreDescriptor.LastSeen);

		// Volume space
		AddValueVirtual(AttrTotalBytes, pItemDescriptor->StoreDescriptor.TotalNumberOfBytes.QuadPart);
		AddValueVirtual(AttrTotalBytesFree, pItemDescriptor->StoreDescriptor.TotalNumberOfBytesFree.QuadPart);

		if (pItemDescriptor->StoreDescriptor.TotalNumberOfBytesFree.QuadPart!=pItemDescriptor->StoreDescriptor.FreeBytesAvailable.QuadPart)
			AddValueVirtual(AttrFreeBytesAvailable, pItemDescriptor->StoreDescriptor.FreeBytesAvailable.QuadPart);

		break;
	}
}

void CFileSummary::UpdateIATAAirport(BOOL AllowMultiple)
{
	ResetAttribute(AttrIATAAirportName);
	ResetAttribute(AttrIATAAirportCountry);

	const BOOL Multiple = AllowMultiple && (m_AttributeSummary[LFAttrLocationIATA].Status==STATUSMULTIPLE);
	const BOOL Visible = m_AttributeSummary[LFAttrLocationIATA].Visible && !LFIsNullVariantData(m_AttributeSummary[LFAttrLocationIATA].VData);

	if (Visible)
		if (Multiple)
		{
			m_AttributeSummary[AttrIATAAirportName].Status = m_AttributeSummary[AttrIATAAirportCountry].Status = STATUSMULTIPLE;
			m_AttributeSummary[AttrIATAAirportName].Visible = m_AttributeSummary[AttrIATAAirportCountry].Visible = TRUE;
		}
		else
		{
			LPCAIRPORT lpcAirport;
			if (LFIATAGetAirportByCode(m_AttributeSummary[LFAttrLocationIATA].VData.IATACode, lpcAirport))
			{
				AddValueVirtual(AttrIATAAirportName, lpcAirport->Name);
				AddValueVirtual(AttrIATAAirportCountry, LFIATAGetCountry(lpcAirport->CountryID)->Name);
			}
			else
			{
				AddValueVirtual(AttrIATAAirportName, L"?");
				AddValueVirtual(AttrIATAAirportCountry, L"?");
			}
		}
}


// CIconHeader
//

#define ICONEMPTY         0
#define ICONMULTIPLE      1
#define ICONCORE          2
#define ICONEXTENSION     3
#define ICONPREVIEW       4

CString CIconHeader::m_NoItemsSelected;

CIconHeader::CIconHeader()
	: CInspectorHeader()
{
	if (m_NoItemsSelected.IsEmpty())
		ENSURE(m_NoItemsSelected.LoadString(IDS_NOITEMSSELECTED));

	m_Description = m_NoItemsSelected;
	m_Status = ICONEMPTY;
	m_pItemDesciptor = NULL;
}

CIconHeader::~CIconHeader()
{
	FreeItem();
}

INT CIconHeader::GetPreferredHeight() const
{
	return BACKSTAGEBORDER+128+BACKSTAGEBORDER/2+LFGetApp()->m_DefaultFont.GetFontHeight();
}

void CIconHeader::DrawHeader(CDC& dc, Graphics& g, const CRect& rect, BOOL Themed) const
{
	CPoint pt((rect.Width()-128)/2, rect.top+BACKSTAGEBORDER);

	switch (m_Status)
	{
	case ICONMULTIPLE:
		g.DrawImage(theApp.GetCachedResourceImage(IDB_MULTIPLE_128), pt.x, pt.y);
		break;

	case ICONCORE:
		theApp.m_CoreImageListJumbo.DrawEx(&dc, m_IconID-1, pt, CSize(128, 128), CLR_NONE, 0xFFFFFF, ILD_TRANSPARENT);
		DrawStoreIconShadow(g, pt, m_IconID);

		break;

	case ICONEXTENSION:
		theApp.m_IconFactory.DrawJumboFormatIcon(dc, pt, m_FileFormat);
		break;

	case ICONPREVIEW:
		theApp.m_IconFactory.DrawJumboIcon(dc, g, pt, m_pItemDesciptor, NULL, FALSE);
		break;
	}

	dc.SetTextColor(m_Status==ICONEMPTY ? Themed ? 0xBFB0A6 : GetSysColor(COLOR_3DSHADOW) : Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));

	CRect rectText(rect);
	rectText.top += (m_Status==ICONEMPTY) ? BACKSTAGEBORDER : BACKSTAGEBORDER+128+BACKSTAGEBORDER/2;

	dc.DrawText(m_Description, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_TOP | DT_CENTER | DT_NOPREFIX);
}

void CIconHeader::FreeItem()
{
	if (m_pItemDesciptor)
	{
		LFFreeItemDescriptor(m_pItemDesciptor);
		m_pItemDesciptor = NULL;
	}
}

void CIconHeader::SetEmpty()
{
	m_Status = ICONEMPTY;
	m_Description = m_NoItemsSelected;

	FreeItem();
}

void CIconHeader::SetMultiple(const CString& Description)
{
	m_Status = ICONMULTIPLE;
	m_Description = Description;

	FreeItem();
}

void CIconHeader::SetCoreIcon(INT IconID, const CString& Description)
{
	m_Status = ICONCORE;
	m_IconID = IconID;
	m_Description = Description;

	FreeItem();
}

void CIconHeader::SetFormatIcon(LPCSTR pFileFormat, const CString& Description)
{
	ASSERT(pFileFormat);

	m_Status = ICONEXTENSION;
	strcpy_s(m_FileFormat, LFExtSize, pFileFormat);
	m_Description = Description;

	FreeItem();
}

void CIconHeader::SetPreview(const LFItemDescriptor* pItemDescriptor, const CString& Description)
{
	ASSERT(pItemDescriptor);

	m_Status = ICONPREVIEW;
	m_Description = Description;
	strcpy_s(m_FileFormat, LFExtSize, pItemDescriptor->CoreAttributes.FileFormat);

	FreeItem();
	m_pItemDesciptor = LFCloneItemDescriptor(pItemDescriptor);
}

BOOL CIconHeader::UpdateThumbnailColor(const LFVariantData& VData)
{
	ASSERT(VData.Type==LFTypeColor);

	switch (m_Status)
	{
	case ICONCORE:
		if ((m_IconID>=IDI_FLD_DEFAULT) && (m_IconID<IDI_FLD_DEFAULT+LFItemColorCount))
		{
			m_IconID = IDI_FLD_DEFAULT+VData.Color;

			return TRUE;
		}

		break;

	case ICONPREVIEW:
		LFSetAttributeVariantData(m_pItemDesciptor, VData);

		return TRUE;
	}

	return FALSE;
}


// CInspectorPane
//

CString CInspectorPane::m_AttributeVirtualNames[AttrCount-LFAttributeCount];

CInspectorPane::CInspectorPane()
	: CFrontstagePane()
{
	ASSERT(AttrIATAAirportName>=AttrCount-2);
	ASSERT(AttrIATAAirportCountry>=AttrCount-2);

	if (m_AttributeVirtualNames[0].IsEmpty())
		for (UINT a=0; a<AttrCount-LFAttributeCount; a++)
			ENSURE(m_AttributeVirtualNames[a].LoadString(a+IDS_VATTR_FIRST));
}

INT CInspectorPane::GetMinWidth(INT Height) const
{
	return m_wndInspectorGrid.GetMinWidth(Height)+BACKSTAGEBORDER;
}

void CInspectorPane::AdjustLayout(CRect rectLayout)
{
	const INT BorderLeft = BACKSTAGEBORDER-PANEGRIPPER;

	m_wndInspectorGrid.SetWindowPos(NULL, rectLayout.left+BorderLeft, rectLayout.top, rectLayout.Width()-BorderLeft, rectLayout.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CInspectorPane::SaveSettings() const
{
	theApp.WriteInt(_T("InspectorShowInternal"), m_ShowInternal);
	theApp.WriteInt(_T("InspectorSortAlphabetic"), m_SortAlphabetic);
}

void CInspectorPane::UpdatePropertyState(UINT Attr)
{
	ASSERT(Attr<AttrCount);

	const AttributeSummary* pAttributeSummary = &m_FileSummary.m_AttributeSummary[Attr];

	m_wndInspectorGrid.UpdatePropertyState(Attr, pAttributeSummary->Status==STATUSMULTIPLE,
		Attr<LFAttributeCount ? theApp.IsAttributeEditable(Attr) && (m_FileSummary.m_Context!=LFContextArchive) && (m_FileSummary.m_Context!=LFContextTrash) : FALSE,
		pAttributeSummary->Visible && (m_ShowInternal ? TRUE : Attr<LFAttributeCount ? (theApp.m_Attributes[Attr].AttrProperties.Category!=LFAttrCategoryInternal) : FALSE),
		&pAttributeSummary->VDataRange);
}

void CInspectorPane::UpdateIATAAirport(BOOL AllowMultiple)
{
	m_FileSummary.UpdateIATAAirport(AllowMultiple);

	// Update properties
	UpdatePropertyState(AttrIATAAirportName);
	UpdatePropertyState(AttrIATAAirportCountry);

	// Done
	m_wndInspectorGrid.AdjustLayout();
}

void CInspectorPane::AggregateClose()
{
	// Type
	if (m_FileSummary.m_Type==LFTypeStore)
	{
		ENSURE(m_TypeName.LoadString(IDS_STORE));
	}
	else
	{
		WCHAR tmpStr[256];
		LFGetFileSummary(tmpStr, 256, m_FileSummary.m_ItemCount);

		m_TypeName = tmpStr;
	}

	// Icon
	switch (m_FileSummary.m_IconStatus)
	{
	case STATUSUNUSED:
		m_IconHeader.SetEmpty();
		break;

	case STATUSMULTIPLE:
		m_IconHeader.SetMultiple(m_TypeName);
		break;

	default:
		switch (m_FileSummary.m_Type)
		{
		case LFTypeFile:
			if (m_FileSummary.m_ItemCount==1)
			{
				m_IconHeader.SetPreview(m_FileSummary.p_LastItem, m_TypeName);
			}
			else
				if (m_FileSummary.m_AttributeSummary[LFAttrFileFormat].Status==STATUSMULTIPLE)
				{
					m_IconHeader.SetMultiple(m_TypeName);
				}
				else
				{
					m_IconHeader.SetFormatIcon(m_FileSummary.m_AttributeSummary[LFAttrFileFormat].VData.AnsiString, m_TypeName);
				}

			break;

		default:
			m_IconHeader.SetCoreIcon(m_FileSummary.m_IconID, m_TypeName);
		}
	}

	// Flags
	if (!m_FileSummary.m_FlagsFirst)
	{
		// Flags
		WCHAR tmpStr[7] = L"AMTND";
		WCHAR* pChar = tmpStr;

		for (UINT Flag=LFFlagArchive; Flag>0; Flag>>=1, pChar++)
			if (m_FileSummary.m_FlagsMultiple & Flag)
			{
				*pChar = L'?';
			}
			else
				if ((m_FileSummary.m_FlagsSet & Flag)==0)
				{
					*pChar = L'-';
				}

		m_FileSummary.AddValueVirtual(AttrFlags, tmpStr);
	}

	// Store
	m_wndInspectorGrid.SetStore(m_FileSummary.m_StoreStatus==STATUSUSED ? m_FileSummary.m_StoreID : DEFAULTSTOREID());

	// Update properties, except for AttrIATAAirportName and AttrIATAAirportCounty (must be last)
	for (UINT a=0; a<AttrCount-2; a++)
		UpdatePropertyState(a);

	// Update AttrIATAAirportName and AttrIATAAirportCounty; includes call to AdjustLayout() of CInpsectorGrid
	UpdateIATAAirport();
}


BEGIN_MESSAGE_MAP(CInspectorPane, CFrontstagePane)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_PROPERTYCHANGED, OnPropertyChanged)

	ON_COMMAND(IDM_INSPECTOR_SHOWINTERNAL, OnToggleInternal)
	ON_COMMAND(IDM_INSPECTOR_SORTALPHABETIC, OnAlphabetic)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_INSPECTOR_SHOWINTERNAL, IDM_INSPECTOR_SORTALPHABETIC, OnUpdateCommands)
END_MESSAGE_MAP()

INT CInspectorPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstagePane::OnCreate(lpCreateStruct)==-1)
		return -1;

	m_ShowInternal = theApp.GetInt(_T("InspectorShowInternal"), FALSE);
	m_SortAlphabetic = theApp.GetInt(_T("InspectorSortAlphabetic"), FALSE);

	if (!m_wndInspectorGrid.Create(this, 1, IDM_INSPECTOR, &m_IconHeader))
		return -1;

	// Add attribute properties
	m_wndInspectorGrid.AddAttributeProperties(&m_FileSummary.m_AttributeSummary[0].VData, sizeof(AttributeSummary));

	// Add virtual attribute properties
	for (UINT a=LFAttributeCount; a<AttrCount; a++)
		m_wndInspectorGrid.AddProperty(&m_FileSummary.m_AttributeSummary[a].VData, m_AttributeVirtualNames[a-LFAttributeCount]);

	// Sort inspector items
	m_wndInspectorGrid.SetAlphabeticMode(m_SortAlphabetic);

	return 0;
}

void CInspectorPane::OnSetFocus(CWnd* /*pOldWnd*/)
{
	m_wndInspectorGrid.SetFocus();
}

LRESULT CInspectorPane::OnPropertyChanged(WPARAM wParam, LPARAM lParam)
{
	const SHORT Attr1 = LOWORD(wParam);
	const SHORT Attr2 = LOWORD(lParam);
	const SHORT Attr3 = HIWORD(lParam);

	// Update icon color in header
	if ((Attr1==LFAttrColor) || (Attr2==LFAttrColor) || (Attr3==LFAttrColor))
		if (m_IconHeader.UpdateThumbnailColor(m_FileSummary.m_AttributeSummary[LFAttrColor].VData))
			m_wndInspectorGrid.Invalidate();

	// Update of IATA airport code and country (internal properties)
	if ((Attr1==LFAttrLocationIATA) || (Attr2==LFAttrLocationIATA) || (Attr3==LFAttrLocationIATA))
		UpdateIATAAirport(FALSE);

	// Update items
	const LFVariantData* pValue1 = (Attr1==-1) ? NULL : &m_FileSummary.m_AttributeSummary[Attr1].VData;
	const LFVariantData* pValue2 = (Attr2==-1) ? NULL : &m_FileSummary.m_AttributeSummary[Attr2].VData;
	const LFVariantData* pValue3 = (Attr3==-1) ? NULL : &m_FileSummary.m_AttributeSummary[Attr3].VData;

	((CMainView*)GetParent())->UpdateItems(pValue1, pValue2, pValue3);

	return NULL;
}


void CInspectorPane::OnToggleInternal()
{
	m_ShowInternal = !m_ShowInternal;

	SaveSettings();
	AggregateClose();
}

void CInspectorPane::OnAlphabetic()
{
	m_wndInspectorGrid.SetAlphabeticMode(m_SortAlphabetic=!m_SortAlphabetic);

	SaveSettings();
}

void CInspectorPane::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_INSPECTOR_SHOWINTERNAL:
		pCmdUI->SetCheck(m_ShowInternal);
		break;

	case IDM_INSPECTOR_SORTALPHABETIC:
		pCmdUI->SetCheck(m_SortAlphabetic);
		break;
	}

	pCmdUI->Enable(bEnable);
}
