
// CInspectorWnd.cpp: Implementierung der Klasse CInspectorWnd
//

#include "stdafx.h"
#include "liquidFOLDERS.h"


// CIconHeader
//

CIconHeader::CIconHeader()
	: CInspectorHeader()
{
	m_Empty.Load(IDB_INSPECTOR, _T("PNG"));
	m_Multiple.Load(IDB_MULTIPLE, _T("PNG"));

	ENSURE(m_strUnused.LoadString(IDS_NOITEMSSELECTED));
	m_strDescription = m_strUnused;
	m_Status = IconEmpty;
	m_pItem = NULL;
}

INT CIconHeader::GetPreferredHeight()
{
	return 128+24;
}

void CIconHeader::DrawHeader(CDC& dc, CRect rect, BOOL Themed)
{
	const INT cx = (rect.Width()-128)/2;
	const INT cy = rect.top+4;
	CRect rectIcon(cx, cy, cx+128, cy+128);

	Graphics g(dc);

	switch (m_Status)
	{
	case IconEmpty:
	case IconMultiple:
		{
			g.DrawImage((m_Status==IconEmpty) ? m_Empty.m_pBitmap : m_Multiple.m_pBitmap, cx, cy);
			break;
		}
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

	dc.SetTextColor(m_Status==IconEmpty ? Themed ? 0xBFB0A6 : GetSysColor(COLOR_3DFACE) : Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));

	CRect rectText(rect);
	rectText.top += 128+6;
	dc.DrawText(m_strDescription, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER | DT_CENTER | DT_NOPREFIX);

	if (Themed)
	{
		SolidBrush brush1(Color(0x18, 0x00, 0x00, 0x00));
		g.FillRectangle(&brush1, 0, 0, rect.Width(), 1);

		SolidBrush brush2(Color(0x0C, 0x00, 0x00, 0x00));
		g.FillRectangle(&brush2, 0, 1, rect.Width(), 1);
	}
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
	m_strDescription = m_strUnused;

	FreeItem();
}

void CIconHeader::SetMultiple(CString Description)
{
	m_Status = IconMultiple;
	m_strDescription = Description;

	FreeItem();
}

void CIconHeader::SetCoreIcon(INT IconID, CString Description)
{
	m_Status = IconCore;
	m_IconID = IconID;
	m_strDescription = Description;

	FreeItem();
}

void CIconHeader::SetFormatIcon(CHAR* FileFormat, CString Description)
{
	ASSERT(FileFormat);

	m_Status = IconExtension;
	strcpy_s(m_FileFormat, LFExtSize, FileFormat);
	m_strDescription = Description;

	FreeItem();
}

void CIconHeader::SetPreview(LFItemDescriptor* i, CString Description)
{
	ASSERT(i);

	m_Status = IconPreview;
	m_strDescription = Description;
	strcpy_s(m_FileFormat, LFExtSize, i->CoreAttributes.FileFormat);

	FreeItem();
	m_pItem = LFAllocItemDescriptor(i);
}


// CStoreManagerGrid
//

void CStoreManagerGrid::ScrollWindow(INT /*dx*/, INT /*dy*/)
{
	Invalidate();
}


// CInspectorWnd
//

#define StatusUnused       0
#define StatusUsed         1
#define StatusMultiple     2

CInspectorWnd::CInspectorWnd()
	: CGlassPane()
{
	m_Count = 0;
	p_LastItem = NULL;

	for (UINT a=0; a<AttrCount-LFAttributeCount; a++)
		ENSURE(m_AttributeVirtualNames[a].LoadString(a+IDS_VATTR_FIRST));

	for (UINT a=0; a<AttrCount; a++)
	{
		m_AttributeValues[a].Attr = m_AttributeRangeFirst[a].Attr = m_AttributeRangeSecond[a].Attr = a;
		LFGetNullVariantData(&m_AttributeValues[a]);
		LFGetNullVariantData(&m_AttributeRangeFirst[a]);
		LFGetNullVariantData(&m_AttributeRangeSecond[a]);
	}
}

void CInspectorWnd::AdjustLayout()
{
	CRect rectClient;
	GetClientRect(rectClient);

	m_wndGrid.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CInspectorWnd::SaveSettings()
{
	CString oldBase = theApp.GetRegistryBase();
	theApp.SetRegistryBase(_T("Inspector"));
	theApp.WriteInt(_T("ShowInternal"), m_ShowInternal);
	theApp.WriteInt(_T("SortAlphabetic"), m_SortAlphabetic);
	theApp.SetRegistryBase(oldBase);
}

void CInspectorWnd::AddValue(LFItemDescriptor* i, UINT Attr, BOOL Editable)
{
	m_AttributeEditable[Attr] |= Editable;

	if (i->AttributeValues[Attr])
		switch (m_AttributeStatus[Attr])
		{
		case StatusUnused:
			LFGetAttributeVariantData(i, &m_AttributeValues[Attr]);
			if ((Editable) || (!LFIsNullVariantData(&m_AttributeValues[Attr])))
			{
				m_AttributeStatus[Attr] = StatusUsed;
				m_AttributeVisible[Attr] = TRUE;
			}
			break;
		case StatusUsed:
			if (!LFIsEqualToVariantData(i, &m_AttributeValues[Attr]))
			{
				m_AttributeStatus[Attr] = StatusMultiple;

				LFGetAttributeVariantData(i, &m_AttributeRangeFirst[Attr]);
				if (LFCompareVariantData(&m_AttributeRangeFirst[Attr], &m_AttributeValues[Attr])==-1)
				{
					m_AttributeRangeSecond[Attr] = m_AttributeValues[Attr];
				}
				else
				{
					m_AttributeRangeSecond[Attr] = m_AttributeRangeFirst[Attr];
					m_AttributeRangeFirst[Attr] = m_AttributeValues[Attr];
				}
			}
			break;
		case StatusMultiple:
			LFGetAttributeVariantData(i, &m_AttributeValues[Attr]);

			if (!LFIsNullVariantData(&m_AttributeValues[Attr]))
			{
				if (LFCompareVariantData(&m_AttributeValues[Attr], &m_AttributeRangeFirst[Attr])==-1)
					m_AttributeRangeFirst[Attr] = m_AttributeValues[Attr];

				if (LFCompareVariantData(&m_AttributeValues[Attr], &m_AttributeRangeSecond[Attr])==1)
					m_AttributeRangeSecond[Attr] = m_AttributeValues[Attr];
			}

			break;
		}
}

void CInspectorWnd::AddValueVirtual(UINT Attr, CHAR* Value)
{
	WCHAR tmpStr[256];
	MultiByteToWideChar(CP_ACP, 0, Value, -1, tmpStr, 256);

	AddValueVirtual(Attr, &tmpStr[0]);
}

void CInspectorWnd::AddValueVirtual(UINT Attr, WCHAR* Value)
{
	switch (m_AttributeStatus[Attr])
	{
	case StatusUnused:
		m_AttributeStatus[Attr] = StatusUsed;
		m_AttributeVisible[Attr] = TRUE;
		wcscpy_s(m_AttributeValues[Attr].UnicodeString, 256, Value);
		break;
	case StatusUsed:
		if (wcscmp(m_AttributeValues[Attr].UnicodeString, Value)!=0)
			m_AttributeStatus[Attr] = StatusMultiple;
		break;
	}
}

void CInspectorWnd::UpdateStart()
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
		LFGetNullVariantData(&m_AttributeValues[a]);
		LFGetNullVariantData(&m_AttributeRangeFirst[a]);
		LFGetNullVariantData(&m_AttributeRangeSecond[a]);
	}
}

void CInspectorWnd::UpdateAdd(LFItemDescriptor* i, LFSearchResult* pRawFiles)
{
	p_LastItem = i;

	// Icon
	if (m_IconStatus<StatusMultiple)
		switch (m_IconStatus)
		{
		case StatusUnused:
			m_IconStatus = StatusUsed;
			m_IconID = i->IconID;
			break;
		case StatusUsed:
			if (m_IconID!=i->IconID)
				m_IconStatus = StatusMultiple;
			break;
		}

	// Typ
	m_TypeID = (i->Type & LFTypeMask);

	// Attribute
	switch (m_TypeID)
	{
	case LFTypeVolume:
		m_Count++;
		AddValue(i, LFAttrFileName, TRUE);
		AddValue(i, LFAttrDescription);
		AddValueVirtual(AttrDriveLetter, i->CoreAttributes.FileID);
		AddValueVirtual(AttrSource, theApp.m_SourceNames[i->Type & LFTypeSourceMask][1]);
		break;
	case LFTypeStore:
		m_Count++;

		for (UINT a=0; a<=LFAttrFileTime; a++)
			AddValue(i, a, !theApp.m_Attributes[a].ReadOnly);

		AddValue(i, LFAttrFileCount);
		AddValue(i, LFAttrFileSize);

		LFStoreDescriptor s;
		LFGetStoreSettings(i->StoreID, &s);

		AddValueVirtual(AttrSource, theApp.m_SourceNames[s.Source][0]);

		WCHAR tmpStr[256];
		LFTimeToString(s.MaintenanceTime, tmpStr, 256);
		AddValueVirtual(AttrMaintenanceTime, tmpStr);

		LFTimeToString(s.SynchronizeTime, tmpStr, 256);
		if (tmpStr[0]!=L'\0')
			AddValueVirtual(AttrSynchronizeTime, tmpStr);

		if ((s.Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal)
			AddValueVirtual(AttrLastSeen, s.LastSeen);
		break;
	case LFTypeFile:
		m_Count++;

		for (UINT a=0; a<LFAttributeCount; a++)
			if (a!=LFAttrDescription)
				AddValue(i, a, !theApp.m_Attributes[a].ReadOnly);
		AddValueVirtual(AttrSource, theApp.m_SourceNames[i->Type & LFTypeSourceMask][0]);
		break;
	case LFTypeFolder:
		m_Count += i->AggregateCount;

		if ((i->FirstAggregate!=-1) && (i->LastAggregate!=-1))
		{
			for (INT a=i->FirstAggregate; a<=i->LastAggregate; a++)
			{
				for (UINT b=0; b<LFAttributeCount; b++)
					AddValue(pRawFiles->m_Items[a], b, !theApp.m_Attributes[b].ReadOnly);
				AddValueVirtual(AttrSource, theApp.m_SourceNames[pRawFiles->m_Items[a]->Type & LFTypeSourceMask][0]);
			}
		}
		else
		{
			for (UINT a=LFAttrFileName; a<LFAttributeCount; a++)
				AddValue(i, a);
		}
		break;
	}
}

void CInspectorWnd::UpdateFinish()
{
	// Icon & Typ
	UINT SID = 0;
	switch (m_TypeID)
	{
	case LFTypeVolume:
		SID = IDS_DRIVES_SINGULAR;
		break;
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
		case LFTypeVolume:
			if (m_AttributeStatus[AttrDriveLetter]==StatusMultiple)
			{
				m_IconHeader.SetMultiple(m_TypeName);
			}
			else
			{
				CHAR Path[4];
				strcpy_s(Path, 4, " :\\");
				Path[0] = m_AttributeValues[AttrDriveLetter].AnsiString[0];
				m_IconHeader.SetFormatIcon(Path, m_TypeName);
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
	m_wndGrid.SetStore(m_AttributeStatus[LFAttrStoreID]==StatusUsed ? m_AttributeValues[LFAttrStoreID].AnsiString : NULL);

	m_wndGrid.AdjustLayout();
}


BEGIN_MESSAGE_MAP(CInspectorWnd, CGlassPane)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE(WM_PROPERTYCHANGED, OnPropertyChanged)

	ON_COMMAND(IDM_INSPECTOR_SHOWINTERNAL, OnToggleInternal)
	ON_COMMAND(IDM_INSPECTOR_SORTALPHABETIC, OnAlphabetic)
	ON_COMMAND(IDM_INSPECTOR_EXPORTSUMMARY, OnExportSummary)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_INSPECTOR_SHOWINTERNAL, IDM_INSPECTOR_EXPORTMETADATA, OnUpdateCommands)
END_MESSAGE_MAP()

INT CInspectorWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlassPane::OnCreate(lpCreateStruct)==-1)
		return -1;

	CString oldBase = theApp.GetRegistryBase();
	theApp.SetRegistryBase(_T("Inspector"));
	m_ShowInternal = theApp.GetInt(_T("ShowInternal"), FALSE);
	m_SortAlphabetic = theApp.GetInt(_T("SortAlphabetic"), FALSE);
	theApp.SetRegistryBase(oldBase);

	if (!m_wndGrid.Create(this, 1, &m_IconHeader))
		return -1;

	m_wndGrid.SetAlphabeticMode(m_SortAlphabetic);
	m_wndGrid.AddAttributes(m_AttributeValues);
	for (UINT a=LFAttributeCount; a<AttrCount; a++)
		m_wndGrid.AddProperty(new CProperty(&m_AttributeValues[a]), LFAttrCategoryInternal, m_AttributeVirtualNames[a-LFAttributeCount].GetBuffer());

	return 0;
}

void CInspectorWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	m_wndGrid.SetFocus();
}

void CInspectorWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
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

LRESULT CInspectorWnd::OnPropertyChanged(WPARAM wparam, LPARAM lparam)
{
	SHORT Attr1 = LOWORD(wparam);
	SHORT Attr2 = LOWORD(lparam);
	SHORT Attr3 = HIWORD(lparam);
	SHORT AttrIATA = (Attr1==LFAttrLocationIATA) ? Attr1 : (Attr2==LFAttrLocationIATA) ? Attr2 : (Attr3==LFAttrLocationIATA) ? Attr3 : -1;

	LFVariantData* Value1 = (Attr1==-1) ? NULL : &m_AttributeValues[Attr1];
	LFVariantData* Value2 = (Attr2==-1) ? NULL : &m_AttributeValues[Attr2];
	LFVariantData* Value3 = (Attr3==-1) ? NULL : &m_AttributeValues[Attr3];

	// Special handling of IATA airport for internal properties
	if (AttrIATA!=-1)
	{
		LFAirport* pAirport;
		if (LFIsNullVariantData(&m_AttributeValues[AttrIATA]))
		{
			wcscpy_s(m_AttributeValues[AttrIATAAirportName].UnicodeString, 256, L"");
			wcscpy_s(m_AttributeValues[AttrIATAAirportCountry].UnicodeString, 256, L"");
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


void CInspectorWnd::OnToggleInternal()
{
	m_ShowInternal = !m_ShowInternal;
	SaveSettings();
	UpdateFinish();
}

void CInspectorWnd::OnAlphabetic()
{
	m_SortAlphabetic = !m_SortAlphabetic;
	SaveSettings();

	m_wndGrid.SetAlphabeticMode(m_SortAlphabetic);
}

void CInspectorWnd::OnExportSummary()
{
	CString Extensions((LPCSTR)IDS_TXTFILEFILTER);
	Extensions += _T(" (*.txt)|*.txt||");

	CFileDialog dlg(FALSE, _T(".txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
	{
		FILE *fStream;
		if (_tfopen_s(&fStream, dlg.GetPathName(), _T("wt,ccs=UTF-8")))
		{
			LFErrorBox(LFDriveNotReady, GetSafeHwnd());
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
				LFErrorBox(LFDriveNotReady, GetSafeHwnd());
			}
			f.Close();
		}
	}
}

void CInspectorWnd::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;

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
		b = m_Count;
		break;
	}

	pCmdUI->Enable(b);
}
