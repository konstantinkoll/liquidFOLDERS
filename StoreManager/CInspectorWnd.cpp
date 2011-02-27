
#include "stdafx.h"
#include "CInspectorWnd.h"
#include "Resource.h"
#include "StoreManager.h"
#include "LFCore.h"


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

	INT heightIcn = m_ShowIcon ? m_wndIconCtrl.GetPreferredHeight() : 0;

	m_wndIconCtrl.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), heightIcn, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top+heightIcn, rectClient.Width(), rectClient.Height()-heightIcn, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CInspectorWnd::SaveSettings()
{
	CString oldBase = theApp.GetRegistryBase();
	theApp.SetRegistryBase(_T("Inspector"));
	theApp.WriteInt(_T("ShowIcon"), m_ShowIcon);
	theApp.WriteInt(_T("Alphabetic"), m_Alphabetic);
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

		if (a<LFAttributeCount)
			if (theApp.m_Attributes[a]->Type==LFTypeUnicodeArray)
				((CAttributePropertyTags*)pAttributes[a])->SetStore(StoreID ? *StoreID!='\0' ? StoreID : NULL : NULL);
	}
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
	case LFTypeDrive:
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
		case LFTypeDrive:
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
		m_wndIconCtrl.SetEmpty();
		break;
	case StatusMultiple:
		m_wndIconCtrl.SetMultiple(TypeName);
		break;
	default:
		if (TypeStatus==StatusMultiple)
		{
			m_wndIconCtrl.SetMultiple(TypeName);
		}
		else
			if (TypeID==LFTypeFile)
			{
				if (AttributeStatus[LFAttrFileFormat]==StatusMultiple)
				{
					m_wndIconCtrl.SetMultiple(TypeName);
				}
				else
				{
					m_wndIconCtrl.SetFormatIcon(AttributeValues[LFAttrFileFormat].AnsiString, TypeName);
				}
			}
			else
			{
				m_wndIconCtrl.SetCoreIcon(IconID-1, TypeName);
			}
	}

	m_wndPropList.SetRedraw(FALSE);

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

	// Attribut-Kategorien
	#ifndef _DEBUG
	BOOL CategoryVisible[LFAttrCategoryCount];
	ZeroMemory(CategoryVisible, sizeof(CategoryVisible));
	for (UINT a=0; a<AttrCount; a++)
		CategoryVisible[AttributeCategory[a]] |= AttributeVisible[a];
	for (UINT a=0; a<LFAttrCategoryCount; a++)
		pGroups[a]->Show(CategoryVisible[a]);
	#endif

	// Attribute
	for (UINT a=0; a<AttrCount; a++)
	{
		#ifndef _DEBUG
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
		#endif
	}

	m_wndPropList.SetRedraw(TRUE);
	m_wndPropList.Invalidate();
}


BEGIN_MESSAGE_MAP(CInspectorWnd, CGlasPane)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_PAINT()
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
	ON_COMMAND(ID_INSPECTOR_TOGGLEICON, OnToggleIcon)
	ON_COMMAND(ID_INSPECTOR_TREEVIEW, OnTreeView)
	ON_COMMAND(ID_INSPECTOR_ALPHABETIC, OnAlphabetic)
	ON_COMMAND(ID_INSPECTOR_EXPANDALL, OnExpandAll)
	ON_COMMAND(ID_INSPECTOR_EXPORT, OnExport)
	ON_UPDATE_COMMAND_UI_RANGE(ID_INSPECTOR_TOGGLEICON, ID_INSPECTOR_EXPORT, OnUpdateCommands)
END_MESSAGE_MAP()

INT CInspectorWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlasPane::OnCreate(lpCreateStruct)==-1)
		return -1;

	CString oldBase = theApp.GetRegistryBase();
	theApp.SetRegistryBase(_T("Inspector"));
	m_ShowIcon = theApp.GetInt(_T("ShowIcon"), TRUE);
	m_Alphabetic = theApp.GetInt(_T("Alphabetic"), FALSE);
	theApp.SetRegistryBase(oldBase);

	if (!m_wndIconCtrl.Create(this, 1))
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();
	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
		return -1;

	m_wndPropList.SetGroupNameFullWidth(TRUE, FALSE);
	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.MarkModifiedProperties(TRUE);
	m_wndPropList.SetAlphabeticMode(m_Alphabetic);
	m_wndPropList.SetFont(CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT)), FALSE);

	for (UINT a=0; a<LFAttrCategoryCount; a++)
		pGroups[a] = new CMFCPropertyGridProperty(theApp.m_AttrCategories[a]);

	for (UINT a=0; a<AttrCount; a++)
	{
		if (a>=LFAttributeCount)
		{
			pAttributes[a] = new CMFCPropertyGridProperty(AttributeVirtualNames[a-LFAttributeCount], _T(""));
			pAttributes[a]->Enable(FALSE);
			pAttributes[a]->AllowEdit(FALSE);
			AttributeCategory[a] = LFAttrCategoryInternal;
		}
		else
		{
			switch (theApp.m_Attributes[a]->Type)
			{
			case LFTypeUnicodeArray:
				pAttributes[a] = new CAttributePropertyTags(&AttributeValues[a]);
				break;
			case LFTypeAnsiString:
				pAttributes[a] = (a==LFAttrLocationIATA) ? new CAttributePropertyIATA(&AttributeValues[a], (CAttributeProperty**)&pAttributes[LFAttrLocationName], (CAttributeProperty**)&pAttributes[LFAttrLocationGPS]) : new CAttributeProperty(&AttributeValues[a]);
				break;
			case LFTypeRating:
				pAttributes[a] = new CAttributePropertyRating(&AttributeValues[a]);
				break;
			case LFTypeGeoCoordinates:
				pAttributes[a] = new CAttributePropertyGPS(&AttributeValues[a]);
				break;
			case LFTypeTime:
				pAttributes[a] = new CAttributePropertyTime(&AttributeValues[a]);
				break;
			default:
				pAttributes[a] = new CAttributeProperty(&AttributeValues[a]);
			}
			AttributeCategory[a] = theApp.m_Attributes[a]->Category;
		}

		pGroups[AttributeCategory[a]]->AddSubItem(pAttributes[a]);
	}

	for (UINT a=0; a<LFAttrCategoryCount; a++)
		m_wndPropList.AddProperty(pGroups[a]);

	pGroups[LFAttrCategoryInternal]->Expand(FALSE);

	return 0;
}

void CInspectorWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	m_wndPropList.SetFocus();
}

void CInspectorWnd::OnPaint()
{
	m_wndPropList.SetCustomColors(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_WINDOWTEXT),
		afxGlobalData.clrBarFace, afxGlobalData.clrBarText,
		afxGlobalData.clrBarFace, GetSysColor(COLOR_WINDOWTEXT), afxGlobalData.clrBarFace);

	CGlasPane::OnPaint();
}

LRESULT CInspectorWnd::OnPropertyChanged(WPARAM /*wparam*/, LPARAM lparam)
{
	CAttributeProperty* pProp = (CAttributeProperty*)lparam;

	LFVariantData* value1 = pProp->p_Data;
	LFVariantData* value2 = NULL;
	LFVariantData* value3 = NULL;

	if ((pProp->p_DependentProp1) && (pProp->m_UseDependencies & 1))
		value2 = (*(pProp->p_DependentProp1))->p_Data;

	if ((pProp->p_DependentProp2) && (pProp->m_UseDependencies & 2))
		value3 = (*(pProp->p_DependentProp2))->p_Data;

	((CMainWnd*)GetTopLevelParent())->UpdateSelectedItems(value1, value2, value3);
	pProp->m_UseDependencies = 0;

	return 0;
}

void CInspectorWnd::OnToggleIcon()
{
	m_ShowIcon = !m_ShowIcon;
	AdjustLayout();
	SaveSettings();
}

void CInspectorWnd::OnTreeView()
{
	m_Alphabetic = FALSE;
	SaveSettings();

	m_wndPropList.SetAlphabeticMode(m_Alphabetic);
}

void CInspectorWnd::OnAlphabetic()
{
	m_Alphabetic = TRUE;
	SaveSettings();

	m_wndPropList.SetAlphabeticMode(m_Alphabetic);
}

void CInspectorWnd::OnExpandAll()
{
	m_wndPropList.ExpandAll();
}

void CInspectorWnd::OnExport()
{
	CString tmpStr;
	tmpStr.LoadString(IDS_TXTFILEFILTER);
	tmpStr += _T(" (*.txt)|*.txt||");

	CFileDialog dlg(FALSE, _T(".txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, tmpStr, this);
	if (dlg.DoModal()==IDOK)
	{
		CStdioFile f;
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
		}
	}
}

void CInspectorWnd::OnUpdateCommands(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_INSPECTOR_TOGGLEICON:
		pCmdUI->SetCheck(m_ShowIcon);
		pCmdUI->Enable(TRUE);
		break;
	case ID_INSPECTOR_TREEVIEW:
		pCmdUI->SetCheck(!m_Alphabetic);
		pCmdUI->Enable(TRUE);
		break;
	case ID_INSPECTOR_ALPHABETIC:
		pCmdUI->SetCheck(m_Alphabetic);
		pCmdUI->Enable(TRUE);
		break;
	case ID_INSPECTOR_EXPANDALL:
		pCmdUI->Enable((!m_Alphabetic) && (Count));
		break;
	case ID_INSPECTOR_EXPORT:
		pCmdUI->Enable(Count);
	}
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
