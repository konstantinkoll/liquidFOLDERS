
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

void CFileSummary::Reset()
{
	p_LastItem = NULL;
	m_ItemCount = 0;

	// Icon and type
	m_IconStatus = STATUSUNUSED;

	// Properties
	ZeroMemory(m_AttributeSummary, sizeof(m_AttributeSummary));

	for (UINT a=0; a<AttrCount; a++)
	{
		LFInitVariantData(m_AttributeSummary[a].Value, a);
		LFInitVariantData(m_AttributeSummary[a].RangeFirst, a);
		LFInitVariantData(m_AttributeSummary[a].RangeSecond, a);
	}
}

void CFileSummary::AddValueVirtual(UINT Attr, const LPCWSTR pStrValue)
{
	ASSERT(Attr<AttrCount);
	ASSERT(pStrValue);

	AttributeSummary* pAttributeSummary = &m_AttributeSummary[Attr];

	switch (pAttributeSummary->Status)
	{
	case STATUSUNUSED:
		wcsncpy_s(pAttributeSummary->Value.UnicodeString, 256, pStrValue, _TRUNCATE);
		pAttributeSummary->Value.IsNull = FALSE;

		pAttributeSummary->Status = STATUSUSED;
		pAttributeSummary->Visible = TRUE;

		break;

	case STATUSUSED:
		if (wcscmp(pAttributeSummary->Value.UnicodeString, pStrValue)!=0)
			pAttributeSummary->Status = STATUSMULTIPLE;

		break;
	}
}

void CFileSummary::AddValueVirtual(UINT Attr, const LPCSTR pStrValue)
{
	ASSERT(pStrValue);

	WCHAR tmpStr[256];
	MultiByteToWideChar(CP_ACP, 0, pStrValue, -1, tmpStr, 256);

	AddValueVirtual(Attr, &tmpStr[0]);
}

void CFileSummary::AddValue(LFItemDescriptor* pItemDescriptor, UINT Attr)
{
	ASSERT(pItemDescriptor);
	ASSERT(Attr<LFAttributeCount);

	if (pItemDescriptor->AttributeValues[Attr])
	{
		AttributeSummary* pAttributeSummary = &m_AttributeSummary[Attr];

		LFVariantData Property;
		LFGetAttributeVariantDataEx(pItemDescriptor, Attr, Property);

		switch (pAttributeSummary->Status)
		{
		case STATUSUNUSED:
			if (!theApp.m_Attributes[Attr].AttrProperties.ReadOnly || !LFIsNullVariantData(Property))
			{
				pAttributeSummary->Value = Property;

				pAttributeSummary->Status = STATUSUSED;
				pAttributeSummary->Visible = TRUE;
			}

			break;

		case STATUSUSED:
			if (LFCompareVariantData(pAttributeSummary->Value, Property)==0)
				return;

			pAttributeSummary->RangeFirst = pAttributeSummary->RangeSecond = pAttributeSummary->Value;
			pAttributeSummary->Status = STATUSMULTIPLE;

		case STATUSMULTIPLE:
			if (!LFIsNullVariantData(Property))
			{
				if (LFCompareVariantData(Property, pAttributeSummary->RangeFirst)==-1)
					pAttributeSummary->RangeFirst = Property;

				if (LFCompareVariantData(Property, pAttributeSummary->RangeSecond)==1)
					pAttributeSummary->RangeSecond = Property;
			}

			break;
		}
	}
}

void CFileSummary::AddFile(LFItemDescriptor* pItemDescriptor)
{
	ASSERT(pItemDescriptor);
	ASSERT((pItemDescriptor->Type & LFTypeMask)==LFTypeFile);

	p_LastItem = pItemDescriptor;
	m_ItemCount++;

	for (UINT a=0; a<LFAttributeCount; a++)
		AddValue(pItemDescriptor, a);

	AddValueVirtual(AttrSource, theApp.m_SourceNames[pItemDescriptor->Type & LFTypeSourceMask][0]);
}

void CFileSummary::AddItem(LFItemDescriptor* pItemDescriptor, LFSearchResult* pRawFiles)
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
	switch (m_Type=(pItemDescriptor->Type & LFTypeMask))
	{
	case LFTypeStore:
		m_ItemCount++;

		for (UINT a=0; a<=LFAttrFileTime; a++)
			AddValue(pItemDescriptor, a);

		AddValue(pItemDescriptor, LFAttrFileCount);
		AddValue(pItemDescriptor, LFAttrFileSize);

		LFStoreDescriptor Store;
		LFGetStoreSettings(pItemDescriptor->StoreID, &Store);

		AddValueVirtual(AttrSource, theApp.m_SourceNames[Store.Source][0]);

		WCHAR tmpStr[256];
		LFTimeToString(Store.MaintenanceTime, tmpStr, 256);
		AddValueVirtual(AttrMaintenanceTime, tmpStr);

		LFTimeToString(Store.SynchronizeTime, tmpStr, 256);
		if (tmpStr[0]!=L'\0')
			AddValueVirtual(AttrSynchronizeTime, tmpStr);

		if ((Store.Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal)
			AddValueVirtual(AttrLastSeen, Store.LastSeen);

		break;

	case LFTypeFile:
		AddFile(pItemDescriptor);
		break;

	case LFTypeFolder:
		if ((pItemDescriptor->FirstAggregate!=-1) && (pItemDescriptor->LastAggregate!=-1))
		{
			for (INT a=pItemDescriptor->FirstAggregate; a<=pItemDescriptor->LastAggregate; a++)
				AddFile((*pRawFiles)[a]);
		}
		else
		{
			m_ItemCount += pItemDescriptor->AggregateCount;

			for (UINT a=LFAttrFileName; a<LFAttributeCount; a++)
				AddValue(pItemDescriptor, a);
		}

		break;
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
	m_pItem = NULL;
}

CIconHeader::~CIconHeader()
{
	FreeItem();
}

INT CIconHeader::GetPreferredHeight() const
{
	return 128+24;
}

void CIconHeader::DrawHeader(CDC& dc, Graphics& g, const CRect& rect, BOOL Themed) const
{
	CPoint pt((rect.Width()-128)/2, rect.top+4);

	switch (m_Status)
	{
	case ICONMULTIPLE:
		g.DrawImage(LFGetApp()->GetCachedResourceImage(IDB_MULTIPLE), pt.x, pt.y);
		break;

	case ICONCORE:
		theApp.m_CoreImageListJumbo.DrawEx(&dc, m_IconID, pt, CSize(128, 128), CLR_NONE, 0xFFFFFF, ILD_TRANSPARENT);
		break;

	case ICONEXTENSION:
		theApp.m_IconFactory.DrawJumboFormatIcon(dc, pt, m_FileFormat);
		break;

	case ICONPREVIEW:
		theApp.m_IconFactory.DrawJumboIcon(dc, g, pt, m_pItem, NULL);
		break;
	}

	dc.SetTextColor(m_Status==ICONEMPTY ? Themed ? 0xBFB0A6 : GetSysColor(COLOR_3DSHADOW) : Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));

	CRect rectText(rect);
	rectText.top += (m_Status==ICONEMPTY) ? BACKSTAGEBORDER : 128+6;

	dc.DrawText(m_Description, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_TOP | DT_CENTER | DT_NOPREFIX);
}

void CIconHeader::FreeItem()
{
	if (m_pItem)
	{
		LFFreeItemDescriptor(m_pItem);
		m_pItem = NULL;
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

void CIconHeader::SetPreview(LFItemDescriptor* pItemDescriptor, const CString& Description)
{
	ASSERT(pItemDescriptor);

	m_Status = ICONPREVIEW;
	m_Description = Description;
	strcpy_s(m_FileFormat, LFExtSize, pItemDescriptor->CoreAttributes.FileFormat);

	FreeItem();
	m_pItem = LFCloneItemDescriptor(pItemDescriptor);
}


// CInspectorPane
//

CString CInspectorPane::m_AttributeVirtualNames[AttrCount-LFAttributeCount];

CInspectorPane::CInspectorPane()
	: CFrontstagePane()
{
	if (m_AttributeVirtualNames[0].IsEmpty())
		for (UINT a=0; a<AttrCount-LFAttributeCount; a++)
			ENSURE(m_AttributeVirtualNames[a].LoadString(a+IDS_VATTR_FIRST));
}

void CInspectorPane::AdjustLayout(CRect rectLayout)
{
	const INT BorderLeft = BACKSTAGEBORDER-PANEGRIPPER;

	m_wndGrid.SetWindowPos(NULL, rectLayout.left+BorderLeft, rectLayout.top, rectLayout.Width()-BorderLeft, rectLayout.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CInspectorPane::SaveSettings()
{
	theApp.WriteInt(_T("InspectorShowInternal"), m_ShowInternal);
	theApp.WriteInt(_T("InspectorSortAlphabetic"), m_SortAlphabetic);
}

void CInspectorPane::AggregateFinish()
{
	// Type
	UINT SID = (m_FileSummary.m_Type==LFTypeStore) ? IDS_STORES_SINGULAR : IDS_FILES_SINGULAR;

	if (SID)
	{
		if (m_FileSummary.m_ItemCount!=1)
			SID++;

		m_TypeName.Format(CString((LPCSTR)SID), m_FileSummary.m_ItemCount);
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
			if (m_FileSummary.m_AttributeSummary[LFAttrFileFormat].Status==STATUSMULTIPLE)
			{
				m_IconHeader.SetMultiple(m_TypeName);
			}
			else
				if ((m_FileSummary.m_AttributeSummary[LFAttrStoreID].Status==STATUSMULTIPLE) || (m_FileSummary.m_AttributeSummary[LFAttrFileID].Status==STATUSMULTIPLE))
				{
					m_IconHeader.SetFormatIcon(m_FileSummary.m_AttributeSummary[LFAttrFileFormat].Value.AnsiString, m_TypeName);
				}
				else
				{
					m_IconHeader.SetPreview(m_FileSummary.p_LastItem, m_TypeName);
				}

			break;

		default:
			m_IconHeader.SetCoreIcon(m_FileSummary.m_IconID-1, m_TypeName);
		}
	}

	// Airport name and country
	if ((m_FileSummary.m_AttributeSummary[LFAttrLocationIATA].Status==STATUSUSED) && (m_FileSummary.m_AttributeSummary[LFAttrLocationIATA].Value.AnsiString[0]!='\0'))
	{
		LFAirport* pAirport;
		if (LFIATAGetAirportByCode(m_FileSummary.m_AttributeSummary[LFAttrLocationIATA].Value.AnsiString, &pAirport))
		{
			m_FileSummary.AddValueVirtual(AttrIATAAirportName, pAirport->Name);
			m_FileSummary.AddValueVirtual(AttrIATAAirportCountry, LFIATAGetCountry(pAirport->CountryID)->Name);
		}
		else
		{
			m_FileSummary.AddValueVirtual(AttrIATAAirportName, L"?");
			m_FileSummary.AddValueVirtual(AttrIATAAirportCountry, L"?");
		}
	}
	else
	{
		m_FileSummary.m_AttributeSummary[AttrIATAAirportName].Status = m_FileSummary.m_AttributeSummary[AttrIATAAirportCountry].Status = m_FileSummary.m_AttributeSummary[LFAttrLocationIATA].Status;
		m_FileSummary.m_AttributeSummary[AttrIATAAirportName].Visible = m_FileSummary.m_AttributeSummary[AttrIATAAirportCountry].Visible = (m_FileSummary.m_AttributeSummary[LFAttrLocationIATA].Status==STATUSMULTIPLE);
	}

	// Update properties
	for (UINT a=0; a<AttrCount; a++)
	{
		const AttributeSummary* pAttributeSummary = &m_FileSummary.m_AttributeSummary[a];

		m_wndGrid.UpdatePropertyState(a, pAttributeSummary->Status==STATUSMULTIPLE,
		a<LFAttributeCount ? !theApp.m_Attributes[a].AttrProperties.ReadOnly : FALSE,
			pAttributeSummary->Visible & (m_ShowInternal ? TRUE : a<LFAttributeCount ? (theApp.m_Attributes[a].AttrProperties.Category!=LFAttrCategoryInternal) : FALSE),
			&pAttributeSummary->RangeFirst, &pAttributeSummary->RangeSecond);
	}

	// Store
	ASSERT(theApp.m_Attributes[LFAttrStoreID].AttrProperties.Type==LFTypeAnsiString);
	m_wndGrid.SetStore(m_FileSummary.m_AttributeSummary[LFAttrStoreID].Status==STATUSUSED ? m_FileSummary.m_AttributeSummary[LFAttrStoreID].Value.AnsiString : "");

	m_wndGrid.AdjustLayout();
}


BEGIN_MESSAGE_MAP(CInspectorPane, CFrontstagePane)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE(WM_PROPERTYCHANGED, OnPropertyChanged)

	ON_COMMAND(IDM_INSPECTOR_SHOWINTERNAL, OnToggleInternal)
	ON_COMMAND(IDM_INSPECTOR_SORTALPHABETIC, OnAlphabetic)
	ON_COMMAND(IDM_INSPECTOR_EXPORTSUMMARY, OnExportSummary)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_INSPECTOR_SHOWINTERNAL, IDM_INSPECTOR_EXPORTMETADATA, OnUpdateCommands)
END_MESSAGE_MAP()

INT CInspectorPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstagePane::OnCreate(lpCreateStruct)==-1)
		return -1;

	m_ShowInternal = theApp.GetInt(_T("InspectorShowInternal"), FALSE);
	m_SortAlphabetic = theApp.GetInt(_T("InspectorSortAlphabetic"), FALSE);

	if (!m_wndGrid.Create(this, 1, &m_IconHeader))
		return -1;

	m_wndGrid.SetAlphabeticMode(m_SortAlphabetic);

	// Add attribute properties
	for (UINT a=0; a<LFAttributeCount; a++)
	{
		CProperty* pProperty = m_wndGrid.AddAttributeProperty(&m_FileSummary.m_AttributeSummary[a].Value);

		if (a==LFAttrLocationIATA)
			((CPropertyIATA*)pProperty)->SetAdditionalData(&m_FileSummary.m_AttributeSummary[LFAttrLocationName].Value, &m_FileSummary.m_AttributeSummary[LFAttrLocationGPS].Value);
	}

	// Add virtual attribute properties
	for (UINT a=LFAttributeCount; a<AttrCount; a++)
		m_wndGrid.AddProperty(new CProperty(&m_FileSummary.m_AttributeSummary[a].Value), LFAttrCategoryInternal, m_AttributeVirtualNames[a-LFAttributeCount].GetBuffer());

	return 0;
}

void CInspectorPane::OnSetFocus(CWnd* /*pOldWnd*/)
{
	m_wndGrid.SetFocus();
}

void CInspectorPane::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if ((point.x<0) || (point.y<0))
	{
		CRect rect;
		GetClientRect(rect);

		point.x = (rect.left+rect.right)/2;
		point.y = (rect.top+rect.bottom)/2;
		ClientToScreen(&point);
	}

	CMenu Menu;
	ENSURE(Menu.LoadMenu(IDM_INSPECTOR));

	CMenu* pPopup = Menu.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, GetOwner(), NULL);
}

LRESULT CInspectorPane::OnPropertyChanged(WPARAM wParam, LPARAM lParam)
{
	SHORT Attr1 = LOWORD(wParam);
	SHORT Attr2 = LOWORD(lParam);
	SHORT Attr3 = HIWORD(lParam);
	SHORT AttrIATA = (Attr1==LFAttrLocationIATA) ? Attr1 : (Attr2==LFAttrLocationIATA) ? Attr2 : (Attr3==LFAttrLocationIATA) ? Attr3 : -1;

	LFVariantData* pValue1 = (Attr1==-1) ? NULL : &m_FileSummary.m_AttributeSummary[Attr1].Value;
	LFVariantData* pValue2 = (Attr2==-1) ? NULL : &m_FileSummary.m_AttributeSummary[Attr2].Value;
	LFVariantData* pValue3 = (Attr3==-1) ? NULL : &m_FileSummary.m_AttributeSummary[Attr3].Value;

	// Update of IATA airport code and country (internal properties)
	if (AttrIATA!=-1)
	{
		BOOL Visible;
		if ((Visible=!LFIsNullVariantData(m_FileSummary.m_AttributeSummary[AttrIATA].Value))==TRUE)
		{
			LFAirport* pAirport;
			if (LFIATAGetAirportByCode(m_FileSummary.m_AttributeSummary[AttrIATA].Value.IATAString, &pAirport))
			{
				MultiByteToWideChar(CP_ACP, 0, pAirport->Name, -1, m_FileSummary.m_AttributeSummary[AttrIATAAirportName].Value.UnicodeString, 256);
				MultiByteToWideChar(CP_ACP, 0, LFIATAGetCountry(pAirport->CountryID)->Name, -1, m_FileSummary.m_AttributeSummary[AttrIATAAirportCountry].Value.UnicodeString, 256);
			}
			else
			{
				wcscpy_s(m_FileSummary.m_AttributeSummary[AttrIATAAirportName].Value.UnicodeString, 256, L"?");
				wcscpy_s(m_FileSummary.m_AttributeSummary[AttrIATAAirportCountry].Value.UnicodeString, 256, L"?");
			}

			m_FileSummary.m_AttributeSummary[AttrIATAAirportName].Value.IsNull = m_FileSummary.m_AttributeSummary[AttrIATAAirportCountry].Value.IsNull = FALSE;
		}

		m_wndGrid.UpdatePropertyState(AttrIATAAirportName, FALSE, FALSE, Visible && m_ShowInternal);
		m_wndGrid.UpdatePropertyState(AttrIATAAirportCountry, FALSE, FALSE, Visible && m_ShowInternal);
		m_wndGrid.AdjustLayout();
	}

	// Update items
	((CMainView*)GetParent())->UpdateItems(pValue1, pValue2, pValue3);

	return NULL;
}


void CInspectorPane::OnToggleInternal()
{
	m_ShowInternal = !m_ShowInternal;

	SaveSettings();
	AggregateFinish();
}

void CInspectorPane::OnAlphabetic()
{
	m_wndGrid.SetAlphabeticMode(m_SortAlphabetic=!m_SortAlphabetic);

	SaveSettings();
}

void CInspectorPane::OnExportSummary()
{
	CString Extensions((LPCSTR)IDS_TXTFILEFILTER);
	Extensions += _T(" (*.txt)|*.txt||");

	CFileDialog dlg(FALSE, _T(".txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
	{
		FILE* pStream;
		if (_tfopen_s(&pStream, dlg.GetPathName(), _T("wt,ccs=UTF-8")))
		{
			LFErrorBox(this, LFDriveNotReady);
		}
		else
		{
			CStdioFile f(pStream);

			try
			{
				f.WriteString(m_TypeName+_T("\n\n"));

				for (UINT a=0; a<AttrCount; a++)
					if (m_FileSummary.m_AttributeSummary[a].Visible)
						f.WriteString(m_wndGrid.GetName(a)+_T(": ")+m_wndGrid.GetValue(a)+_T("\n"));
			}
			catch(CFileException ex)
			{
				LFErrorBox(this, LFDriveNotReady);
			}

			f.Close();
		}
	}
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

	case IDM_INSPECTOR_EXPORTSUMMARY:
	case IDM_INSPECTOR_EXPORTMETADATA:
		bEnable = m_FileSummary.m_ItemCount;
		break;
	}

	pCmdUI->Enable(bEnable);
}
