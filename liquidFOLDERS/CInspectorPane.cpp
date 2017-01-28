
// CInspectorPane.cpp: Implementierung der Klasse CInspectorPane
//

#include "stdafx.h"
#include "liquidFOLDERS.h"


// CIconHeader
//

CString CIconHeader::m_strNoItemsSelected;

CIconHeader::CIconHeader()
	: CInspectorHeader()
{
	if (m_strNoItemsSelected.IsEmpty())
		ENSURE(m_strNoItemsSelected.LoadString(IDS_NOITEMSSELECTED));

	m_strDescription = m_strNoItemsSelected;
	m_Status = IconEmpty;
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

void CIconHeader::DrawHeader(CDC& dc, const CRect& rect, BOOL Themed)
{
	const INT cx = (rect.Width()-128)/2;
	const INT cy = rect.top+4;
	CRect rectIcon(cx, cy, cx+128, cy+128);

	Graphics g(dc);

	switch (m_Status)
	{
	case IconMultiple:
		g.DrawImage(LFGetApp()->GetCachedResourceImage(IDB_MULTIPLE), cx, cy);
		break;

	case IconCore:
		theApp.m_CoreImageListJumbo.DrawEx(&dc, m_IconID, CPoint(cx, cy), CSize(128, 128), CLR_NONE, 0xFFFFFF, ILD_TRANSPARENT);
		break;

	case IconPreview:
		if (theApp.m_ThumbnailCache.DrawJumboThumbnail(dc, rectIcon, m_pItem))
			break;

	case IconExtension:
		theApp.m_FileFormats.DrawJumboIcon(dc, rectIcon, m_FileFormat);
		break;
	}

	dc.SetTextColor(m_Status==IconEmpty ? Themed ? 0xBFB0A6 : GetSysColor(COLOR_3DSHADOW) : Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));

	CRect rectText(rect);
	rectText.top += 128+6;

	if (m_Status==IconEmpty)
		rectText.OffsetRect(0, -128);

	dc.DrawText(m_strDescription, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER | DT_CENTER | DT_NOPREFIX);

	if (Themed)
		CTaskbar::DrawTaskbarShadow(g, rect);
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
	m_Status = IconEmpty;
	m_strDescription = m_strNoItemsSelected;

	FreeItem();
}

void CIconHeader::SetMultiple(const CString& Description)
{
	m_Status = IconMultiple;
	m_strDescription = Description;

	FreeItem();
}

void CIconHeader::SetCoreIcon(INT IconID, const CString& Description)
{
	m_Status = IconCore;
	m_IconID = IconID;
	m_strDescription = Description;

	FreeItem();
}

void CIconHeader::SetFormatIcon(CHAR* FileFormat, const CString& Description)
{
	ASSERT(FileFormat);

	m_Status = IconExtension;
	strcpy_s(m_FileFormat, LFExtSize, FileFormat);
	m_strDescription = Description;

	FreeItem();
}

void CIconHeader::SetPreview(LFItemDescriptor* pItemDescriptor, const CString& Description)
{
	ASSERT(pItemDescriptor);

	m_Status = IconPreview;
	m_strDescription = Description;
	strcpy_s(m_FileFormat, LFExtSize, pItemDescriptor->CoreAttributes.FileFormat);

	FreeItem();
	m_pItem = LFCloneItemDescriptor(pItemDescriptor);
}


// CStoreManagerGrid
//

void CStoreManagerGrid::ScrollWindow(INT dx, INT dy, LPCRECT lpRect, LPCRECT lpClipRect)
{
	if (IsCtrlThemed())
	{
		CRect rect;
		GetClientRect(rect);

		rect.top += 2;

		ScrollWindowEx(dx, dy, rect, rect, NULL, NULL, SW_INVALIDATE);
		RedrawWindow(CRect(rect.left, 0, rect.right, 2), NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else
	{
		CInspectorGrid::ScrollWindow(dx, dy, lpRect, lpClipRect);
	}
}


// CInspectorPane
//

#define StatusUnused       0
#define StatusUsed         1
#define StatusMultiple     2

CString CInspectorPane::m_AttributeVirtualNames[AttrCount-LFAttributeCount];

CInspectorPane::CInspectorPane()
	: CFrontstagePane()
{
	m_Count = 0;
	p_LastItem = NULL;

	if (m_AttributeVirtualNames[0].IsEmpty())
		for (UINT a=0; a<AttrCount-LFAttributeCount; a++)
			ENSURE(m_AttributeVirtualNames[a].LoadString(a+IDS_VATTR_FIRST));

	for (UINT a=0; a<AttrCount; a++)
	{
		LFInitVariantData(m_AttributeValues[a], a);
		LFInitVariantData(m_AttributeRangeFirst[a], a);
		LFInitVariantData(m_AttributeRangeSecond[a], a);
	}
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

void CInspectorPane::AddValue(LFItemDescriptor* pItemDescriptor, UINT Attr, BOOL Editable)
{
	m_AttributeEditable[Attr] |= Editable;

	if (pItemDescriptor->AttributeValues[Attr])
	{
		LFVariantData v;
		LFGetAttributeVariantDataEx(pItemDescriptor, Attr, v);

		INT Cmp;

		switch (m_AttributeStatus[Attr])
		{
		case StatusUnused:
			m_AttributeValues[Attr] = v;
			if ((Editable) || (!LFIsNullVariantData(v)))
			{
				m_AttributeStatus[Attr] = StatusUsed;
				m_AttributeVisible[Attr] = TRUE;
			}

			break;

		case StatusUsed:
			Cmp = LFCompareVariantData(m_AttributeValues[Attr], v);

			if (Cmp!=0)
			{
				m_AttributeStatus[Attr] = StatusMultiple;

				if (Cmp<0)
				{
					m_AttributeRangeFirst[Attr] = m_AttributeValues[Attr];
					m_AttributeRangeSecond[Attr] = v;
				}
				else
				{
					m_AttributeRangeFirst[Attr] = v;
					m_AttributeRangeSecond[Attr] = m_AttributeValues[Attr];
				}
			}

			break;

		case StatusMultiple:
			if (!LFIsNullVariantData(v))
			{
				if (LFCompareVariantData(v, m_AttributeRangeFirst[Attr])==-1)
					m_AttributeRangeFirst[Attr] = v;

				if (LFCompareVariantData(v, m_AttributeRangeSecond[Attr])==1)
					m_AttributeRangeSecond[Attr] = v;
			}

			break;
		}
	}
}

void CInspectorPane::AddValueVirtual(UINT Attr, const CHAR* Value)
{
	WCHAR tmpStr[256];
	MultiByteToWideChar(CP_ACP, 0, Value, -1, tmpStr, 256);

	AddValueVirtual(Attr, &tmpStr[0]);
}

void CInspectorPane::AddValueVirtual(UINT Attr, const WCHAR* Value)
{
	switch (m_AttributeStatus[Attr])
	{
	case StatusUnused:
		m_AttributeStatus[Attr] = StatusUsed;
		m_AttributeVisible[Attr] = TRUE;
		wcsncpy_s(m_AttributeValues[Attr].UnicodeString, 256, Value, 255);

		break;

	case StatusUsed:
		if (wcscmp(m_AttributeValues[Attr].UnicodeString, Value)!=0)
			m_AttributeStatus[Attr] = StatusMultiple;

		break;
	}
}

void CInspectorPane::UpdateStart()
{
	m_Count = 0;
	p_LastItem = NULL;

	// Icon und Typ
	m_IconStatus = StatusUnused;

	// Properties
	ZeroMemory(m_AttributeVisible, sizeof(m_AttributeVisible));
	ZeroMemory(m_AttributeStatus, sizeof(m_AttributeStatus));
	ZeroMemory(m_AttributeEditable, sizeof(m_AttributeEditable));

	for (UINT a=0; a<AttrCount; a++)
	{
		LFClearVariantData(m_AttributeValues[a]);
		LFClearVariantData(m_AttributeRangeFirst[a]);
		LFClearVariantData(m_AttributeRangeSecond[a]);
	}
}

void CInspectorPane::UpdateAdd(LFItemDescriptor* pItemDescriptor, LFSearchResult* pRawFiles)
{
	p_LastItem = pItemDescriptor;

	// Icon
	if (m_IconStatus<StatusMultiple)
		switch (m_IconStatus)
		{
		case StatusUnused:
			m_IconStatus = StatusUsed;
			m_IconID = pItemDescriptor->IconID;
			break;

		case StatusUsed:
			if (m_IconID!=pItemDescriptor->IconID)
				m_IconStatus = StatusMultiple;

			break;
		}

	// Typ
	m_TypeID = (pItemDescriptor->Type & LFTypeMask);

	// Attribute
	switch (m_TypeID)
	{
	case LFTypeStore:
		m_Count++;

		for (UINT a=0; a<=LFAttrFileTime; a++)
			AddValue(pItemDescriptor, a, !theApp.m_Attributes[a].ReadOnly);

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
		m_Count++;

		for (UINT a=0; a<LFAttributeCount; a++)
			if (a!=LFAttrDescription)
				AddValue(pItemDescriptor, a, !theApp.m_Attributes[a].ReadOnly);
		AddValueVirtual(AttrSource, theApp.m_SourceNames[pItemDescriptor->Type & LFTypeSourceMask][0]);

		break;

	case LFTypeFolder:
		m_Count += pItemDescriptor->AggregateCount;

		if ((pItemDescriptor->FirstAggregate!=-1) && (pItemDescriptor->LastAggregate!=-1))
		{
			for (INT a=pItemDescriptor->FirstAggregate; a<=pItemDescriptor->LastAggregate; a++)
			{
				for (UINT b=0; b<LFAttributeCount; b++)
					AddValue((*pRawFiles)[a], b, !theApp.m_Attributes[b].ReadOnly);
				AddValueVirtual(AttrSource, theApp.m_SourceNames[(*pRawFiles)[a]->Type & LFTypeSourceMask][0]);
			}
		}
		else
		{
			for (UINT a=LFAttrFileName; a<LFAttributeCount; a++)
				AddValue(pItemDescriptor, a);
		}

		break;
	}
}

void CInspectorPane::UpdateFinish()
{
	// Icon & Typ
	UINT SID = 0;
	switch (m_TypeID)
	{
	case LFTypeStore:
		SID = IDS_STORES_SINGULAR;
		break;

	default:
		SID = IDS_FILES_SINGULAR;
	}

	if (SID)
	{
		if (m_Count!=1)
			SID++;

		CString tmpStr((LPCSTR)SID);
		m_TypeName.Format(tmpStr, m_Count);
	}

	switch (m_IconStatus)
	{
	case StatusUnused:
		m_IconHeader.SetEmpty();
		break;

	case StatusMultiple:
		m_IconHeader.SetMultiple(m_TypeName);
		break;

	default:
		switch (m_TypeID)
		{
		case LFTypeFile:
			if (m_AttributeStatus[LFAttrFileFormat]==StatusMultiple)
			{
				m_IconHeader.SetMultiple(m_TypeName);
			}
			else
				if ((m_AttributeStatus[LFAttrStoreID]==StatusMultiple) || (m_AttributeStatus[LFAttrFileID]==StatusMultiple))
				{
					m_IconHeader.SetFormatIcon(m_AttributeValues[LFAttrFileFormat].AnsiString, m_TypeName);
				}
				else
				{
					m_IconHeader.SetPreview(p_LastItem, m_TypeName);
				}

			break;

		default:
			m_IconHeader.SetCoreIcon(m_IconID-1, m_TypeName);
		}
	}

	// Flughafen-Name und -Land
	if ((m_AttributeStatus[LFAttrLocationIATA]==StatusUsed) && (m_AttributeValues[LFAttrLocationIATA].AnsiString[0]!='\0'))
	{
		LFAirport* pAirport;
		if (LFIATAGetAirportByCode(m_AttributeValues[LFAttrLocationIATA].AnsiString, &pAirport))
		{
			AddValueVirtual(AttrIATAAirportName, pAirport->Name);
			AddValueVirtual(AttrIATAAirportCountry, LFIATAGetCountry(pAirport->CountryID)->Name);
		}
		else
		{
			AddValueVirtual(AttrIATAAirportName, L"?");
			AddValueVirtual(AttrIATAAirportCountry, L"?");
		}
	}
	else
	{
		m_AttributeStatus[AttrIATAAirportName] = m_AttributeStatus[AttrIATAAirportCountry] = m_AttributeStatus[LFAttrLocationIATA];
		m_AttributeVisible[AttrIATAAirportName] = m_AttributeVisible[AttrIATAAirportCountry] = (m_AttributeStatus[LFAttrLocationIATA]==StatusMultiple);
	}

	// Werte aktualisieren
	for (UINT a=0; a<AttrCount; a++)
		m_wndGrid.UpdatePropertyState(a, m_AttributeStatus[a]==StatusMultiple,
			a<LFAttributeCount ? (!theApp.m_Attributes[a].ReadOnly) && m_AttributeEditable[a] : FALSE,
			m_AttributeVisible[a] & (m_ShowInternal ? TRUE : a<LFAttributeCount ? (theApp.m_Attributes[a].Category!=LFAttrCategoryInternal) : FALSE),
			&m_AttributeRangeFirst[a], &m_AttributeRangeSecond[a]);

	// Store
	m_wndGrid.SetStore(m_AttributeStatus[LFAttrStoreID]==StatusUsed ? m_AttributeValues[LFAttrStoreID].AnsiString : "");

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
	m_wndGrid.AddAttributes(m_AttributeValues);

	for (UINT a=LFAttributeCount; a<AttrCount; a++)
		m_wndGrid.AddProperty(new CProperty(&m_AttributeValues[a]), LFAttrCategoryInternal, m_AttributeVirtualNames[a-LFAttributeCount].GetBuffer());

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

	CMenu menu;
	ENSURE(menu.LoadMenu(IDM_INSPECTOR));

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, GetOwner(), NULL);
}

LRESULT CInspectorPane::OnPropertyChanged(WPARAM wParam, LPARAM lParam)
{
	SHORT Attr1 = LOWORD(wParam);
	SHORT Attr2 = LOWORD(lParam);
	SHORT Attr3 = HIWORD(lParam);
	SHORT AttrIATA = (Attr1==LFAttrLocationIATA) ? Attr1 : (Attr2==LFAttrLocationIATA) ? Attr2 : (Attr3==LFAttrLocationIATA) ? Attr3 : -1;

	LFVariantData* Value1 = (Attr1==-1) ? NULL : &m_AttributeValues[Attr1];
	LFVariantData* Value2 = (Attr2==-1) ? NULL : &m_AttributeValues[Attr2];
	LFVariantData* Value3 = (Attr3==-1) ? NULL : &m_AttributeValues[Attr3];

	// Special handling of IATA airport for internal properties
	if (AttrIATA!=-1)
	{
		LFAirport* pAirport;
		if (LFIsNullVariantData(m_AttributeValues[AttrIATA]))
		{
			m_AttributeValues[AttrIATAAirportName].UnicodeString[0] = L'\0';
			m_AttributeValues[AttrIATAAirportCountry].UnicodeString[0] = L'\0';
		}
		else
			if (LFIATAGetAirportByCode(m_AttributeValues[AttrIATA].AnsiString, &pAirport))
			{
				MultiByteToWideChar(CP_ACP, 0, pAirport->Name, -1, m_AttributeValues[AttrIATAAirportName].UnicodeString, 256);
				MultiByteToWideChar(CP_ACP, 0, LFIATAGetCountry(pAirport->CountryID)->Name, -1, m_AttributeValues[AttrIATAAirportCountry].UnicodeString, 256);
			}
			else
			{
				wcscpy_s(m_AttributeValues[AttrIATAAirportName].UnicodeString, 256, L"?");
				wcscpy_s(m_AttributeValues[AttrIATAAirportCountry].UnicodeString, 256, L"?");
			}

		m_wndGrid.UpdatePropertyState(AttrIATAAirportName, FALSE, FALSE, m_ShowInternal);
		m_wndGrid.UpdatePropertyState(AttrIATAAirportCountry, FALSE, FALSE, m_ShowInternal);
		m_wndGrid.AdjustLayout();
	}

	((CMainView*)GetParent())->UpdateItems(Value1, Value2, Value3);

	return NULL;
}


void CInspectorPane::OnToggleInternal()
{
	m_ShowInternal = !m_ShowInternal;
	SaveSettings();
	UpdateFinish();
}

void CInspectorPane::OnAlphabetic()
{
	m_SortAlphabetic = !m_SortAlphabetic;
	SaveSettings();

	m_wndGrid.SetAlphabeticMode(m_SortAlphabetic);
}

void CInspectorPane::OnExportSummary()
{
	CString Extensions((LPCSTR)IDS_TXTFILEFILTER);
	Extensions += _T(" (*.txt)|*.txt||");

	CFileDialog dlg(FALSE, _T(".txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
	{
		FILE *fStream;
		if (_tfopen_s(&fStream, dlg.GetPathName(), _T("wt,ccs=UTF-8")))
		{
			LFErrorBox(this, LFDriveNotReady);
		}
		else
		{
			CStdioFile f(fStream);
			try
			{
				f.WriteString(m_TypeName+_T("\n\n"));
				for (UINT a=0; a<AttrCount; a++)
					if (m_AttributeVisible[a])
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
		bEnable = m_Count;
		break;
	}

	pCmdUI->Enable(bEnable);
}
