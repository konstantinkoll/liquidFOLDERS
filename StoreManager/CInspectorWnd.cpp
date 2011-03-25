
#include "stdafx.h"
#include "CInspectorWnd.h"
#include "Resource.h"
#include "StoreManager.h"
#include "LFCore.h"


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

	switch (m_Status)
	{
	case IconEmpty:
	case IconMultiple:
		{
			Graphics g(dc);
			g.SetCompositingMode(CompositingModeSourceOver);
			g.DrawImage((m_Status==IconEmpty) ? m_Empty.m_pBitmap : m_Multiple.m_pBitmap, cx, cy);
			break;
		}
	case IconCore:
		theApp.m_CoreImageListJumbo.DrawEx(&dc, m_IconID, CPoint(cx, cy), CSize(128, 128), CLR_NONE, 0xFFFFFF, ILD_TRANSPARENT);
		break;
	case IconExtension:
		theApp.m_FileFormats.DrawJumboIcon(dc, rectIcon, m_FileFormat);
		break;
	}

	dc.SetTextColor(m_Status==IconEmpty ? GetSysColor(COLOR_3DSHADOW) : Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));

	CRect rectText(rect);
	rectText.top += 128+6;
	dc.DrawText(m_strDescription, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER | DT_CENTER);
}

void CIconHeader::SetEmpty()
{
	m_Status = IconEmpty;
	m_strDescription = m_strUnused;
}

void CIconHeader::SetMultiple(CString Description)
{
	m_Status = IconMultiple;
	m_strDescription = Description;
}

void CIconHeader::SetCoreIcon(INT IconID, CString Description)
{
	m_Status = IconCore;
	m_IconID = IconID;
	m_strDescription = Description;
}

void CIconHeader::SetFormatIcon(CHAR* FileFormat, CString Description)
{
	ASSERT(FileFormat);

	m_Status = IconExtension;
	strcpy_s(m_FileFormat, LFExtSize, FileFormat);
	m_strDescription = Description;
}


// CInspectorWnd
//

#define StatusUnused            0
#define StatusUsed              1
#define StatusMultiple          2

CInspectorWnd::CInspectorWnd()
	: CGlasPane()
{
	Count = 0;

	for (UINT a=0; a<AttrCount-LFAttributeCount; a++)
		AttributeVirtualNames[a].LoadString(a+IDS_VATTR_FIRST);

	for (UINT a=0; a<AttrCount; a++)
	{
		AttributeValues[a].Attr = a;
		LFGetNullVariantData(&AttributeValues[a]);
	}
}

void CInspectorWnd::AdjustLayout()
{
	CRect rectClient;
	GetClientRect(rectClient);

	m_wndInspectorGrid.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CInspectorWnd::SaveSettings()
{
	CString oldBase = theApp.GetRegistryBase();
	theApp.SetRegistryBase(_T("Inspector"));
	theApp.WriteInt(_T("ShowPreview"), m_ShowPreview);
	theApp.WriteInt(_T("SortAlphabetic"), m_SortAlphabetic);
	theApp.SetRegistryBase(oldBase);
}

void CInspectorWnd::UpdateStart(CHAR* StoreID)
{
	Count = 0;

	// Icon
	IconStatus = StatusUnused;

	// Typ
	TypeStatus = StatusUnused;

	// Properties
	ZeroMemory(AttributeVisible, sizeof(AttributeVisible));
	ZeroMemory(AttributeStatus, sizeof(AttributeStatus));
	ZeroMemory(AttributeEditable, sizeof(AttributeEditable));

	for (UINT a=0; a<AttrCount; a++)
	{
		AttributeValues[a].Attr = a;
		LFGetNullVariantData(&AttributeValues[a]);
	}

/*		if (a<LFAttributeCount)
			if (theApp.m_Attributes[a]->Type==LFTypeUnicodeArray)
				((CAttributePropertyTags*)pAttributes[a])->SetStore(StoreID ? *StoreID!='\0' ? StoreID : NULL : NULL);
	}*/
}

void CInspectorWnd::UpdateAdd(LFItemDescriptor* i, LFSearchResult* raw)
{
	Count++;

	// Icon
	if (IconStatus<StatusMultiple)
	{
		UINT _IconID = i->IconID;
		if (IconID==IDI_STORE_Default)
			IconID=IDI_STORE_Internal;

		switch (IconStatus)
		{
		case StatusUnused:
			IconStatus = StatusUsed;
			IconID = _IconID;
			break;
		case StatusUsed:
			if (IconID!=_IconID)
				IconStatus = StatusMultiple;
			break;
		}
	}

	// Typ
	if (TypeStatus<StatusMultiple)
		switch (TypeStatus)
		{
		case StatusUnused:
			TypeStatus = StatusUsed;
			TypeID = (i->Type & LFTypeMask);
			break;
		case StatusUsed:
			if (TypeID!=(i->Type & LFTypeMask))
				TypeStatus = StatusMultiple;
			break;
		}

	// Attribute
	switch (i->Type & LFTypeMask)
	{
	case LFTypeVolume:
		AddValue(i, LFAttrFileName, FALSE);
		AddValue(i, LFAttrDescription);
		AddValueVirtual(AttrDriveLetter, i->CoreAttributes.FileID);
		break;
	case LFTypeVirtual:
		AddValue(i, LFAttrFileName, FALSE);
		AddValue(i, LFAttrFileID);
		AddValue(i, LFAttrStoreID);
		AddValue(i, LFAttrDescription);
		if ((i->FirstAggregate!=-1) && (i->LastAggregate!=-1))
		{
			AddValue(i, LFAttrFileCount);
			for (INT a=i->FirstAggregate; a<=i->LastAggregate; a++)
				for (UINT b=0; b<LFAttributeCount; b++)
					if ((raw->m_Items[a]->AttributeValues[b]) && (b!=LFAttrFileName) && (b!=LFAttrDescription) && (b!=LFAttrDeleteTime) && (b!=LFAttrFileCount))
						AddValue(raw->m_Items[a], b);
		}
		else
		{
			AddValue(i, LFAttrComment, FALSE);
			for (UINT a=LFAttrDescription+1; a<LFAttributeCount; a++)
				if (i->AttributeValues[a])
					AddValue(i, a, FALSE);
		}
		break;
	case LFTypeFile:
		for (UINT a=0; a<LFAttributeCount; a++)
			if ((i->AttributeValues[a]) && (a!=LFAttrDescription) && (a!=LFAttrDeleteTime))
				AddValue(i, a);
		if (i->CoreAttributes.Flags & LFFlagTrash)
			AddValue(i, LFAttrDeleteTime);
		break;
	case LFTypeStore:
		for (UINT a=0; a<=LFAttrFileTime; a++)
			AddValue(i, a);

		LFStoreDescriptor s;
		LFGetStoreSettings(i->CoreAttributes.FileID, &s);

		OLECHAR szGUID[MAX_PATH];
		StringFromGUID2(s.guid, szGUID, MAX_PATH);
		AddValueVirtual(AttrGUID, szGUID);

		WCHAR tmpStr[256];
		LFTimeToString(s.MaintenanceTime, tmpStr, 256);
		AddValueVirtual(AttrMaintenanceTime, tmpStr);

		LFUINTToString(s.IndexVersion, tmpStr, 256);
		AddValueVirtual(AttrIndexVersion, tmpStr);

		if (s.StoreMode!=LFStoreModeInternal)
			AddValueVirtual(AttrLastSeen, s.LastSeen);

		AddValueVirtual(AttrPathData, s.DatPath);
		AddValueVirtual(AttrPathIdxMain, s.IdxPathMain);
		AddValueVirtual(AttrPathIdxAux, s.IdxPathAux);
		break;
	}
}

void CInspectorWnd::UpdateFinish()
{
	// Icon & Typ
	UINT SID = 0;
	if (TypeStatus==StatusMultiple)
	{
		SID = IDS_MULTIPLETYPESSELECTED;
	}
	else
	{
		switch (TypeID)
		{
		case LFTypeVolume:
			SID = IDS_DRIVES_SINGULAR;
			break;
		case LFTypeStore:
			SID = IDS_STORES_SINGULAR;
			break;
		case LFTypeFile:
			SID = IDS_FILES_SINGULAR;
			break;
		case LFTypeVirtual:
			SID = IDS_FOLDERS_SINGULAR;
			break;
		}

		if ((SID) && (Count!=1))
			SID++;
	}

	if (SID)
	{
		CString tmpStr;
		ENSURE(tmpStr.LoadString(SID));
		TypeName.Format(tmpStr, Count);
	}

	switch (IconStatus)
	{
	case StatusUnused:
		m_IconHeader.SetEmpty();
		break;
	case StatusMultiple:
		m_IconHeader.SetMultiple(TypeName);
		break;
	default:
		if (TypeStatus==StatusMultiple)
		{
			m_IconHeader.SetMultiple(TypeName);
		}
		else
			if (TypeID==LFTypeFile)
			{
				if (AttributeStatus[LFAttrFileFormat]==StatusMultiple)
				{
					m_IconHeader.SetMultiple(TypeName);
				}
				else
				{
					m_IconHeader.SetFormatIcon(AttributeValues[LFAttrFileFormat].AnsiString, TypeName);
				}
			}
			else
			{
				m_IconHeader.SetCoreIcon(IconID-1, TypeName);
			}
	}

	// Flughafen-Name
	if ((AttributeStatus[LFAttrLocationIATA]==StatusUsed) && (AttributeValues[LFAttrLocationIATA].AnsiString[0]!='\0'))
	{
		LFAirport* airport;
		if (LFIATAGetAirportByCode(AttributeValues[LFAttrLocationIATA].AnsiString, &airport))
		{
			AddValueVirtual(AttrIATAAirportName, airport->Name);
			AddValueVirtual(AttrIATAAirportCountry, LFIATAGetCountry(airport->CountryID)->Name);
		}
		else
		{
			AddValueVirtual(AttrIATAAirportName, "?");
			AddValueVirtual(AttrIATAAirportCountry, "?");
		}
	}
	else
	{
		AttributeStatus[AttrIATAAirportName] = AttributeStatus[AttrIATAAirportCountry] = AttributeStatus[LFAttrLocationIATA];
		AttributeVisible[AttrIATAAirportName] = AttributeVisible[AttrIATAAirportCountry] = AttributeVisible[LFAttrLocationIATA];
	}

	// Attribute
	for (UINT a=0; a<AttrCount; a++)
		m_wndInspectorGrid.UpdatePropertyState(a, AttributeStatus[a]==StatusMultiple, a<LFAttributeCount ? !theApp.m_Attributes[a]->ReadOnly : FALSE, AttributeVisible[a]);
/*		#ifndef _DEBUG
		if (AttributeVisible[a])
		{
		#endif
			BOOL editable = FALSE;
			BOOL enabled = FALSE;
			if (a<LFAttributeCount)
			{
				#ifndef _DEBUG
				enabled = (!theApp.m_Attributes[a]->ReadOnly) && AttributeEditable[a];
				#else
				enabled = TRUE;
				#endif
				editable = enabled && ((CAttributeProperty*)pAttributes[a])->IsEditable();
			}
			pAttributes[a]->Enable(enabled);
			pAttributes[a]->AllowEdit(editable);

			pAttributes[a]->ResetOriginalValue();
			if (a<LFAttributeCount)
			{
				WCHAR tmpStr[256];
				LFVariantDataToString(&AttributeValues[a], tmpStr, 256);
				((CAttributeProperty*)pAttributes[a])->SetValue(tmpStr, AttributeStatus[a]==StatusMultiple);
			}
			else
			{
				pAttributes[a]->SetValue(AttributeStatus[a]==StatusMultiple ? _T("...") : AttributeValues[a].UnicodeString);
			}
		#ifndef _DEBUG
		}
		pAttributes[a]->Show(AttributeVisible[a]);
		#endif*/

	m_wndInspectorGrid.AdjustLayout();
}


BEGIN_MESSAGE_MAP(CInspectorWnd, CGlasPane)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)

	ON_COMMAND(IDM_INSPECTOR_SHOWPREVIEW, OnTogglePreview)
	ON_COMMAND(IDM_INSPECTOR_SORTALPHABETIC, OnAlphabetic)
	ON_COMMAND(IDM_INSPECTOR_EXPORTSUMMARY, OnExport)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_INSPECTOR_SHOWPREVIEW, IDM_INSPECTOR_EXPORTMETADATA, OnUpdateCommands)
END_MESSAGE_MAP()

INT CInspectorWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlasPane::OnCreate(lpCreateStruct)==-1)
		return -1;

	CString oldBase = theApp.GetRegistryBase();
	theApp.SetRegistryBase(_T("Inspector"));
	m_ShowPreview = theApp.GetInt(_T("ShowPreview"), TRUE);
	m_SortAlphabetic = theApp.GetInt(_T("SortAlphabetic"), FALSE);
	theApp.SetRegistryBase(oldBase);

	if (!m_wndInspectorGrid.Create(this, 1, &m_IconHeader))
		return -1;

	m_wndInspectorGrid.SetAlphabeticMode(m_SortAlphabetic);
	m_wndInspectorGrid.AddAttributes(AttributeValues);
	for (UINT a=LFAttributeCount; a<AttrCount; a++)
		m_wndInspectorGrid.AddProperty(new CInspectorProperty(&AttributeValues[a]), LFAttrCategoryInternal, AttributeVirtualNames[a-LFAttributeCount].GetBuffer());

	return 0;
}

void CInspectorWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	m_wndInspectorGrid.SetFocus();
}

void CInspectorWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if ((point.x==-1) && (point.y==-1))
	{
		point.x = point.y = 0;
		ClientToScreen(&point);
	}

	CMenu menu;
	ENSURE(menu.LoadMenu(IDM_INSPECTOR));

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, GetOwner(), NULL);
}

LRESULT CInspectorWnd::OnPropertyChanged(WPARAM /*wparam*/, LPARAM lparam)
{
/*	CAttributeProperty* pProp = (CAttributeProperty*)lparam;

	LFVariantData* value1 = pProp->p_Data;
	LFVariantData* value2 = NULL;
	LFVariantData* value3 = NULL;

	if ((pProp->p_DependentProp1) && (pProp->m_UseDependencies & 1))
		value2 = (*(pProp->p_DependentProp1))->p_Data;

	if ((pProp->p_DependentProp2) && (pProp->m_UseDependencies & 2))
		value3 = (*(pProp->p_DependentProp2))->p_Data;

	((CMainWnd*)GetTopLevelParent())->UpdateSelectedItems(value1, value2, value3);
	pProp->m_UseDependencies = 0;
*/
	return 0;
}


void CInspectorWnd::OnTogglePreview()
{
	m_ShowPreview = !m_ShowPreview;
	SaveSettings();

	m_wndInspectorGrid.ShowHeader(m_ShowPreview);
}

void CInspectorWnd::OnAlphabetic()
{
	m_SortAlphabetic = !m_SortAlphabetic;
	SaveSettings();

	m_wndInspectorGrid.SetAlphabeticMode(m_SortAlphabetic);
}

void CInspectorWnd::OnExport()
{
	CString tmpStr;
	tmpStr.LoadString(IDS_TXTFILEFILTER);
	tmpStr += _T(" (*.txt)|*.txt||");

	CFileDialog dlg(FALSE, _T(".txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, tmpStr, this);
	if (dlg.DoModal()==IDOK)
	{
/*		CStdioFile f;
		if (!f.Open(dlg.GetFileName(), CFile::modeCreate | CFile::modeWrite))
		{
			LFErrorBox(LFDriveNotReady);
		}
		else
		{
			try
			{
				f.WriteString(TypeName+_T("\n\n"));
				for (UINT a=0; a<AttrCount; a++)
					if (pAttributes[a]->IsVisible())
					{
						CString tmpStr1 = pAttributes[a]->GetName();
						CString tmpStr2 = pAttributes[a]->GetValue();
						f.WriteString(tmpStr1+_T(": ")+tmpStr2+_T("\n"));
					}
			}
			catch(CFileException ex)
			{
				LFErrorBox(LFDriveNotReady);
			}
			f.Close();
		}*/
	}
}

void CInspectorWnd::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_INSPECTOR_SHOWPREVIEW:
		pCmdUI->SetCheck(m_ShowPreview);
		break;
	case IDM_INSPECTOR_SORTALPHABETIC:
		pCmdUI->SetCheck(m_SortAlphabetic);
		break;
	case IDM_INSPECTOR_EXPORTSUMMARY:
	case IDM_INSPECTOR_EXPORTMETADATA:
		b = Count;
		break;
	}

	pCmdUI->Enable(b);
}




void CInspectorWnd::AddValue(LFItemDescriptor* i, UINT Attr, BOOL Editable)
{
	AttributeEditable[Attr] |= Editable;

	if (i->AttributeValues[Attr])
	{
		switch (AttributeStatus[Attr])
		{
		case StatusUnused:
			LFGetAttributeVariantData(i, &AttributeValues[Attr]);
			if ((Editable) || (!LFIsNullVariantData(&AttributeValues[Attr])))
			{
				AttributeStatus[Attr] = StatusUsed;
				AttributeVisible[Attr] = TRUE;
			}
			break;
		case StatusUsed:
			if (!LFIsEqualToVariantData(i, &AttributeValues[Attr]))
				AttributeStatus[Attr] = StatusMultiple;
		}
	}
	else
		if (Editable)
			switch (AttributeStatus[Attr])
			{
			case StatusUnused:
				AttributeStatus[Attr] = StatusUsed;
				AttributeVisible[Attr] = TRUE;
				break;
			case StatusUsed:
				if (!AttributeValues[Attr].IsNull)
					AttributeStatus[Attr] = StatusMultiple;
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
	switch (AttributeStatus[Attr])
	{
	case StatusUnused:
		AttributeStatus[Attr] = StatusUsed;
		AttributeVisible[Attr] = TRUE;
		wcscpy_s(AttributeValues[Attr].UnicodeString, 256, Value);
		break;
	case StatusUsed:
		if (wcscmp(AttributeValues[Attr].UnicodeString, Value)!=0)
			AttributeStatus[Attr] = StatusMultiple;
		break;
	}
}
