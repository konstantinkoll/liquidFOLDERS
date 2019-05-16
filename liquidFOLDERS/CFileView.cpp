
// CFileView.cpp: Implementierung der Klasse CFileView
//

#include "stdafx.h"
#include "liquidFOLDERS.h"


// CFileView
//

#define BUTTONPADDING     8

CIcons CFileView::m_LargeColorDots;
CIcons CFileView::m_DefaultColorDots;
CString CFileView::m_WelcomeCaption;
CString CFileView::m_WelcomeMessage;

CFileView::CFileView(UINT Flags, SIZE_T szData, const CSize& szItemInflate)
	: CAbstractFileView(Flags | FRONTSTAGE_ENABLEFOCUSITEM | FRONTSTAGE_ENABLEDRAGANDDROP, szData, szItemInflate)
{
	if (m_WelcomeCaption.IsEmpty())
	{
		ENSURE(m_WelcomeCaption.LoadString(IDS_WELCOME_CAPTION));
		ENSURE(m_WelcomeMessage.LoadString(IDS_WELCOME_MESSAGE));
	}

	p_TaskIcons = NULL;
	p_InspectorPane = NULL;
	p_Filter = NULL;
	p_RawFiles = NULL;
	m_Context = LFContextAllFiles;
	m_SubfolderAttribute = -1;
	m_WelcomeCaptionHeight = m_WelcomeMessageHeight = 0;
}

BOOL CFileView::Create(CWnd* pParentWnd, UINT nID, const CRect& rect, CIcons* pTaskIcons, CInspectorPane* pInspectorPane, LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData, UINT nClassStyle)
{
	ASSERT(pTaskIcons);
	ASSERT(pInspectorPane);

	p_TaskIcons = pTaskIcons;
	p_InspectorPane = pInspectorPane;

	if (!CAbstractFileView::Create(pParentWnd, nID, rect, nClassStyle))
		return FALSE;

	UpdateViewSettings(pCookedFiles ? pCookedFiles->m_Context : LFContextAllFiles, TRUE);
	UpdateSearchResult(pFilter, pRawFiles, pCookedFiles, pPersistentData, TRUE);

	return TRUE;
}

void CFileView::UpdateViewSettings(INT Context, BOOL UpdateSearchResultPending)
{
	DestroyEdit();
	HideTooltip();

	// Set new context
	if (Context>=0)
		m_Context = Context;

	p_ContextViewSettings = &theApp.m_ContextViewSettings[m_Context];
	p_GlobalViewSettings = &theApp.m_GlobalViewSettings;

	// Copy context view settings locally
	m_ContextViewSettings = *p_ContextViewSettings;

	// Allow subclass to see both new and old global settings
	ResetDragLocation();
	SetViewSettings(UpdateSearchResultPending);

	// Finally copy global view settings locally
	m_GlobalViewSettings = *p_GlobalViewSettings;
}

void CFileView::UpdateSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData, BOOL InternalCall)
{
	if (pCookedFiles)
	{
		p_ContextViewSettings = &theApp.m_ContextViewSettings[m_Context=pCookedFiles->m_Context];
		m_ContextViewSettings.SortBy = p_ContextViewSettings->SortBy;

		if (pPersistentData)
		{
			m_HScrollPos = pPersistentData->HScrollPos;
			m_VScrollPos = pPersistentData->VScrollPos;
			m_FocusItem = pPersistentData->FocusItem;
		}
		else
		{
			m_FocusItem = -1;
		}

		m_HideFileExt = LFHideFileExt();
	}

	m_Nothing = TRUE;
	SetSearchResult(pFilter, pRawFiles, pCookedFiles, pPersistentData);

	FinishUpdate(InternalCall);
}

void CFileView::SetViewSettings(BOOL /*UpdateSearchResultPending*/)
{
}

void CFileView::SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* /*pPersistentData*/)
{
	// Filter
	p_Filter = pFilter;

	// Subfolder sort
	m_SubfolderAttribute = LFGetSubfolderAttribute(pFilter);

	// Search results
	p_RawFiles = pRawFiles;
	CAbstractFileView::SetSearchResult(pCookedFiles);
}

void CFileView::GetPersistentData(FVPersistentData& Data, BOOL ForReload) const
{
	ZeroMemory(&Data, sizeof(Data));

	Data.FocusItem = m_FocusItem;
	Data.FocusItemSelected = !ForReload && p_CookedFiles && (m_FocusItem>=0) && (m_FocusItem<(INT)p_CookedFiles->m_ItemCount) ? IsItemSelected(m_FocusItem) : FALSE;
	Data.HScrollPos = m_HScrollPos;
	Data.VScrollPos = m_VScrollPos;
}


// Menus

void CFileView::AppendMoveToItem(CMenu& Menu, UINT FromContext, UINT ToContext) const
{
	ASSERT(IsMenu(Menu));
	ASSERT(FromContext<LFContextCount);
	ASSERT(ToContext<LFContextCount);

	CString tmpStr;
	tmpStr.Format((ToContext==LFContextAllFiles) ? IDS_CONTEXTMENU_REMOVEFROMCONTEXT : IDS_CONTEXTMENU_MOVETOCONTEXT, theApp.m_Contexts[(ToContext==LFContextAllFiles) ? FromContext : ToContext].Name);

	Menu.AppendMenu(MF_STRING, IDM_FILE_MOVETOCONTEXT+ToContext, tmpStr);
}

void CFileView::GetMoveToMenu(CMenu& Menu) const
{
	ASSERT(p_CookedFiles);

	// Source contexts
	INT UserContext = -1;
	UINT64 ContextMask = (UINT64)-1;

	// Folders have aggregated contexts
	for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		if (IsItemSelected(a))
		{
			const BYTE Context = LFGetUserContextID((*p_CookedFiles)[a]);

			UserContext = (UserContext==-1) || (UserContext==Context) ? Context : -2;
			ContextMask &= theApp.m_Contexts[Context].CtxProperties.AllowMoveToContext;
		}

	// Are there contexts left?
	if (!ContextMask || (ContextMask==(UINT64)-1))
		return;

	// Create menu
	Menu.CreatePopupMenu();

	// Contexts
	ASSERT(LFContextAllFiles==0);

	for (UINT a=1; a<LFContextCount; a++)
		if ((ContextMask>>a) & 1)
			AppendMoveToItem(Menu, m_Context, a);

	// Remove from context
	if ((UserContext>=0) && (ContextMask & (1ull<<LFContextAllFiles)))
	{
		if (Menu.GetMenuItemCount())
			Menu.AppendMenu(MF_SEPARATOR);

		AppendMoveToItem(Menu, UserContext, LFContextAllFiles);
	}
}

void CFileView::AppendSendToItem(CMenu& Menu, UINT nID, LPCWSTR lpszNewItem, HICON hIcon, INT cx, INT cy)
{
	ASSERT(IsMenu(Menu));

	const UINT Index = nID % 0xFF;

	Menu.AppendMenu(MF_STRING, nID, lpszNewItem);

	if (hIcon)
	{
		m_SendToItems[Index].hIcon = hIcon;
		m_SendToItems[Index].cx = cx;
		m_SendToItems[Index].cy = cy;

		MENUITEMINFO mii;
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_BITMAP;
		mii.hbmpItem = HBMMENU_CALLBACK;

		SetMenuItemInfo(Menu, Menu.GetMenuItemCount()-1, TRUE, &mii);
	}
}

void CFileView::GetSendToMenu(CMenu& Menu)
{
	// Create menu
	Menu.CreatePopupMenu();

	MENUINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_STYLE;
	mi.dwStyle = MNS_CHECKORBMP;
	SetMenuInfo(Menu, &mi);

	const INT cx = GetSystemMetrics(SM_CXSMICON);
	const INT cy = GetSystemMetrics(SM_CYSMICON);

	BOOL Added = FALSE;

	// Stores
	UINT nID = FIRSTSENDTO;

	ABSOLUTESTOREID StoreID;
	if (LFGetDefaultStore(StoreID)==LFOk)
	{
		AppendSendToItem(Menu, nID, CString((LPCSTR)IDS_DEFAULTSTORE), (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_APPLICATION), IMAGE_ICON, cx, cy, LR_SHARED), cx, cy);
		Added = TRUE;

		const UINT Index = (nID++) & 0xFF;

		m_SendToItems[Index].IsStore = TRUE;
		DEFAULTSTOREID(m_SendToItems[Index].StoreID);
	}

	if (LFGetStoreCount())
	{
		AppendSendToItem(Menu, nID, CString((LPCSTR)IDS_CONTEXTMENU_CHOOSESTORE), NULL, cx, cy);
		Added = TRUE;

		const UINT Index = (nID++) & 0xFF;

		m_SendToItems[Index].IsStore = TRUE;
		strcpy_s(m_SendToItems[Index].StoreID, LFKeySize, CHOOSESTOREID);
	}

	// SendTo shortcuts
	nID = FIRSTSENDTO+0x40;
	if (Added)
	{
		Menu.AppendMenu(MF_SEPARATOR);
		Added = FALSE;
	}

	WCHAR Path[MAX_PATH];
	if (SHGetSpecialFolderPath(NULL, Path, CSIDL_SENDTO, FALSE))
	{
		WCHAR Mask[MAX_PATH];
		wcscpy_s(Mask, MAX_PATH, Path);
		wcscat_s(Mask, MAX_PATH, L"\\*.*");

		WIN32_FIND_DATA ffd;
		HANDLE hFind = FindFirstFile(Mask, &ffd);

		if (hFind!=INVALID_HANDLE_VALUE)
			do
			{
				if ((ffd.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_DIRECTORY))==0)
				{
					WCHAR Name[MAX_PATH];
					wcscpy_s(Name, MAX_PATH, ffd.cFileName);

					WCHAR* pLastExt = wcsrchr(Name, L'.');
					if (pLastExt)
						if (*pLastExt!=L'\0')
						{
							CString Ext(pLastExt);
							Ext.MakeUpper();
							if ((Ext==_T(".DESKLINK")) || (Ext==_T(".LFSENDTO")))
								continue;

							*pLastExt = L'\0';
						}

					WCHAR Dst[MAX_PATH];
					wcscpy_s(Dst, MAX_PATH, Path);
					wcscat_s(Dst, MAX_PATH, L"\\");
					wcscat_s(Dst, MAX_PATH, ffd.cFileName);

					SHFILEINFO ShellFileInfo;
					if (SHGetFileInfo(Dst, 0, &ShellFileInfo, sizeof(SHFILEINFO), SHGFI_ATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON))
					{
						AppendSendToItem(Menu, nID, Name, theApp.m_SystemImageListSmall.ExtractIcon(ShellFileInfo.iIcon), cx, cy);
						Added = TRUE;

						const UINT Index = (nID++) & 0xFF;

						m_SendToItems[Index].IsStore = FALSE;
						wcscpy_s(m_SendToItems[Index].Path, MAX_PATH, Dst);
					}
				}
			}
			while (FindNextFile(hFind, &ffd));

		FindClose(hFind);
	}

	// Volumes
	DWORD VolumesOnSystem = LFGetLogicalVolumes(LFGLV_EXTERNAL | LFGLV_NETWORK | LFGLV_FLOPPIES);
	DWORD VolumesWOFloppies = LFGetLogicalVolumes(LFGLV_EXTERNAL | LFGLV_NETWORK);
	const BOOL HideVolumesWithNoMedia = LFHideVolumesWithNoMedia();

	for (CHAR cVolume='A'; cVolume<='Z'; cVolume++, VolumesOnSystem>>=1, VolumesWOFloppies>>=1)
	{
		if ((VolumesOnSystem & 1)==0)
			continue;

		BOOL CheckEmpty = HideVolumesWithNoMedia && ((VolumesWOFloppies & 1)!=0);

		WCHAR szDriveRoot[] = L" :\\";
		szDriveRoot[0] = cVolume;
		SHFILEINFO ShellFileInfo;
		if (SHGetFileInfo(szDriveRoot, 0, &ShellFileInfo, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON | (CheckEmpty ? SHGFI_ATTRIBUTES : 0)))
		{
			if ((!ShellFileInfo.dwAttributes) && CheckEmpty)
				continue;

			if (Added)
			{
				Menu.AppendMenu(MF_SEPARATOR);
				Added = FALSE;
			}

			AppendSendToItem(Menu, nID, ShellFileInfo.szDisplayName, ShellFileInfo.hIcon, cx, cy);

			const UINT Index = (nID++) & 0xFF;

			m_SendToItems[Index].IsStore = FALSE;
			wcscpy_s(m_SendToItems[Index].Path, MAX_PATH, szDriveRoot);
		}
	}
}

BOOL CFileView::GetContextMenu(CMenu& Menu, INT Index)
{
	// Background context menu
	if (Index==-1)
	{
		// Check contexts with their own menu, overriding view menus
		if (!IsMenu(Menu))
			switch (m_Context)
			{
			case LFContextStores:
				Menu.LoadMenu(IDM_STORES);
				break;

			case LFContextFonts:
				Menu.LoadMenu(IDM_FONTS);
				break;

			case LFContextNew:
				Menu.LoadMenu(IDM_NEW);
				break;

			case LFContextTrash:
				Menu.LoadMenu(IDM_TRASH);
				break;

			case LFContextFilters:
				Menu.LoadMenu(IDM_FILTERS);
				break;
			}

		// Insert "Select all" command
		if (IsSelectionEnabled())
		{
			if (IsMenu(Menu))
			{
				Menu.InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);
			}
			else
			{
				Menu.CreatePopupMenu();
			}

			Menu.InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_ITEMVIEW_SELECTALL, CString((LPCSTR)IDS_CONTEXTMENU_SELECTALL));
		}

		return FALSE;
	}

	// Item context menu
	ASSERT(p_InspectorPane);
	const UINT FileCount = p_InspectorPane->GetFileCount();
	const BOOL MultipleFiles = (FileCount>1);

	UINT InsertPos = 0;
	const LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];

	switch (LFGetItemType(pItemDescriptor))
	{
	case LFTypeStore:
		Menu.LoadMenu(IDM_STORE);

		Menu.InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);
		Menu.InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_STORE_OPENFILEDROP, CString((LPCSTR)IDS_CONTEXTMENU_OPENFILEDROP));
		Menu.InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_STORE_OPENNEWWINDOW, CString((LPCSTR)IDS_CONTEXTMENU_OPENNEWWINDOW));

		break;

	case LFTypeFolder:
		if ((pItemDescriptor->AggregateFirst==-1) || (pItemDescriptor->AggregateLast==-1))
			break;

	case LFTypeFile:
		if ((m_Context==LFContextArchive) || (m_Context==LFContextTrash))
		{
			Menu.LoadMenu(MultipleFiles ? IDM_FILE_AWAY_MULTIPLE : IDM_FILE_AWAY_SINGLE);
			InsertPos = 2;
		}
		else
		{
			Menu.LoadMenu(MultipleFiles ? IDM_FILE_MULTIPLE : IDM_FILE_SINGLE);
		}

		break;
	}

	if (!IsMenu(Menu))
		Menu.CreatePopupMenu();

	// Append file menu
	if (m_Context!=LFContextTrash)
		if (LFIsFile(pItemDescriptor) || LFIsAggregatedFolder(pItemDescriptor))
		{
			if (m_Context!=LFContextArchive)
				Menu.InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION,
					m_Context==LFContextTasks ? IDM_FILE_TASKDONE : IDM_FILE_MAKETASK,
					CString((LPCSTR)(m_Context==LFContextTasks ? MultipleFiles ? IDS_CONTEXTMENU_TASKDONE_MULTIPLE : IDS_CONTEXTMENU_TASKDONE_SINGLE : MultipleFiles ? IDS_CONTEXTMENU_MAKETASK_MULTIPLE : IDS_CONTEXTMENU_MAKETASK_SINGLE)));

			Menu.InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION,
				m_Context==LFContextClipboard ? IDM_FILE_REMOVEFROMCLIPBOARD : IDM_FILE_REMEMBER,
				CString((LPCSTR)(m_Context==LFContextClipboard ? MultipleFiles ? IDS_CONTEXTMENU_REMOVEFROMCLIPBOARD_MULTIPLE : IDS_CONTEXTMENU_REMOVEFROMCLIPBOARD_SINGLE : MultipleFiles ? IDS_CONTEXTMENU_REMEMBER_MULTIPLE : IDS_CONTEXTMENU_REMEMBER_SINGLE)));

			Menu.InsertMenu(InsertPos, MF_SEPARATOR | MF_BYPOSITION);

			if (m_Context!=LFContextArchive)
			{
				// "Move to" popup menu
				CMenu PopupMenu;
				GetMoveToMenu(PopupMenu);

				if (IsMenu(PopupMenu))
				{
					Menu.InsertMenu(InsertPos, MF_POPUP | MF_BYPOSITION, (UINT_PTR)(HMENU)PopupMenu, CString((LPCSTR)(MultipleFiles ? IDS_CONTEXTMENU_MOVETO_MULTIPLE : IDS_CONTEXTMENU_MOVETO_SINGLE)));

					PopupMenu.Detach();
				}
			}

			// "Send to" popup menu
			CMenu PopupMenu;
			GetSendToMenu(PopupMenu);
			ASSERT(IsMenu(PopupMenu));

			Menu.InsertMenu(InsertPos, MF_POPUP | MF_BYPOSITION, (UINT_PTR)(HMENU)PopupMenu, CString((LPCSTR)(MultipleFiles ? IDS_CONTEXTMENU_SENDTO_MULTIPLE : IDS_CONTEXTMENU_SENDTO_SINGLE)));
			Menu.InsertMenu(InsertPos, MF_SEPARATOR | MF_BYPOSITION);

			PopupMenu.Detach();
		}

	if (LFIsFile(pItemDescriptor))
		if (LFIsFilterFile(pItemDescriptor))
		{
			// "Edit" command
			Menu.InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, IDM_FILE_EDIT, CString((LPCSTR)IDS_CONTEXTMENU_EDIT));
		}
		else
		{
			// "Show in Explorer" command
			Menu.InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, IDM_FILE_SHOWEXPLORER, CString((LPCSTR)IDS_CONTEXTMENU_SHOWEXPLORER));

			// "Open with" command
			Menu.InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, IDM_FILE_OPENWITH, CString((LPCSTR)IDS_CONTEXTMENU_OPENWITH));
		}

	// "Open" command
	Menu.InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, IDM_ITEM_OPEN, CString((LPCSTR)IDS_CONTEXTMENU_OPEN));
	Menu.SetDefaultItem(InsertPos, TRUE);

	return FALSE;
}


// Scrolling

void CFileView::AdjustScrollbars()
{
	CAbstractFileView::AdjustScrollbars();

	// Command button
	if (CAbstractFileView::DrawNothing() && (m_Context==LFContextStores))
	{
		CRect rectClient;
		GetClientRect(rectClient);

		// Message
		CDC dc;
		dc.CreateCompatibleDC(NULL);

		CFont* pOldFont = dc.SelectObject(&theApp.m_CaptionFont);

		CRect rectText(rectClient);
		rectText.DeflateRect(BACKSTAGEBORDER, 0);

		dc.DrawText(m_WelcomeCaption, rectText, DT_TOP | DT_CENTER | DT_WORDBREAK | DT_HIDEPREFIX | DT_CALCRECT);

		m_WelcomeCaptionHeight = rectText.Height()+BACKSTAGEBORDER/2;

		rectText = rectClient;
		rectText.DeflateRect(BACKSTAGEBORDER, 0);

		dc.SelectObject(&theApp.m_LargeFont);
		dc.DrawText(m_WelcomeMessage, rectText, DT_TOP | DT_CENTER | DT_WORDBREAK | DT_HIDEPREFIX | DT_CALCRECT);
		dc.SelectObject(pOldFont);

		m_WelcomeMessageHeight = rectText.Height()+BACKSTAGEBORDER;

		// Button
		CString tmpStr;
		m_wndCommandButton.GetWindowText(tmpStr);

		const INT ButtonWidth = min(rectClient.Width()-2*BACKSTAGEBORDER, max(theApp.m_DefaultFont.GetTextExtent(tmpStr).cx, p_TaskIcons->GetIconSize())+2*BUTTONPADDING);
		const INT ButtonHeight = m_DefaultFontHeight+p_TaskIcons->GetIconSize()+3*BUTTONPADDING-2;

		m_wndCommandButton.SetWindowPos(NULL, (rectClient.Width()-ButtonWidth)/2, 2*BACKSTAGEBORDER+m_WelcomeCaptionHeight+m_WelcomeMessageHeight, ButtonWidth, ButtonHeight, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		m_wndCommandButton.ShowWindow(SW_HIDE);
	}
}


// Item handling

void CFileView::ShowTooltip(const CPoint& point)
{
	ASSERT(m_HoverItem>=0);

	if (IsEditing())
		return;

	LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[m_HoverItem];
	HBITMAP hBitmap = (m_Flags & FF_ENABLETOOLTIPICONS) ? theApp.m_IconFactory.GetJumboIconBitmap(pItemDescriptor, theApp.ShowRepresentativeThumbnail(m_ContextViewSettings.SortBy, m_Context) ? p_RawFiles : NULL) : NULL;

	theApp.ShowTooltip(this, point, GetItemLabel(pItemDescriptor),
		theApp.GetHintForItem(pItemDescriptor, theApp.m_IconFactory.GetTypeName(pItemDescriptor->CoreAttributes.FileFormat)),
		NULL, hBitmap);
}


// Item selection

BOOL CFileView::IsItemSelected(INT Index) const
{
	assert(p_CookedFiles);
	assert(Index>=0);
	assert(Index<m_ItemCount);

	return IsSelectionEnabled() ? IsItemSelected((*p_CookedFiles)[Index]) : CAbstractFileView::IsItemSelected(Index);
}

void CFileView::SelectItem(INT Index, BOOL Select)
{
	ASSERT(Index>=0);
	ASSERT(Index<m_ItemCount);

	if (!Select || GetItemData(Index)->Valid)
	{
		LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];
		SelectItem(pItemDescriptor, Select);

		if (LFIsAggregatedFolder(pItemDescriptor))
			for (INT a=pItemDescriptor->AggregateFirst; a<=pItemDescriptor->AggregateLast; a++)
				SelectItem((*p_RawFiles)[a], Select);
	}
}


// Selected item commands

void CFileView::FireSelectedItem() const
{
	ASSERT(IsFocusItemEnabled());
	ASSERT(GetSelectedItem()>=0);

	GetOwner()->SendMessage(WM_COMMAND, IDM_ITEM_OPEN);
}

void CFileView::DeleteSelectedItem() const
{
	ASSERT(IsFocusItemEnabled());
	ASSERT(GetSelectedItem()>=0);

	GetOwner()->PostMessage(WM_COMMAND, IDM_FILE_DELETE);
}


// Draw support

CString CFileView::GetItemLabel(const LFItemDescriptor* pItemDescriptor, BOOL AllowExtension) const
{
	CString strLabel(pItemDescriptor->CoreAttributes.FileName);

	// Remove annotation
	LFVariantData VData1;
	LFGetAttributeVariantDataEx(pItemDescriptor, LFAttrApplication, VData1);

	if (!LFIsNullVariantData(VData1))
	{
		const INT Length = strLabel.GetLength();

		// Remove app name from file name
		if (strLabel.GetAt(Length-1)==L')')
		{
			const UINT Pos = strLabel.ReverseFind(L'(');

			if (Pos!=-1)
			{
				LFVariantData VData2;
				LFInitVariantData(VData2, LFAttrApplication);
				LFVariantDataFromString(VData2, strLabel.Mid(Pos+1, Length-Pos-2));

				if (LFCompareVariantData(VData1, VData2)==0)
					strLabel = strLabel.Left(Pos).TrimRight();
			}
		}
	}

	// Extension
	if (AllowExtension && LFIsFile(pItemDescriptor))
		if ((!m_HideFileExt || (pItemDescriptor->CoreAttributes.FileName[0]==L'\0')) && (pItemDescriptor->CoreAttributes.FileFormat[0]!='\0') && (_stricmp(pItemDescriptor->CoreAttributes.FileFormat, "filter")!=0))
		{
			strLabel += _T(".");
			strLabel += pItemDescriptor->CoreAttributes.FileFormat;
		}

	return strLabel;
}

UINT CFileView::GetColorDotCount(const LFItemDescriptor* pItemDescriptor)
{
	ASSERT(pItemDescriptor);

	switch (LFGetItemType(pItemDescriptor))
	{
	case LFTypeFile:
		return pItemDescriptor->CoreAttributes.Color ? 1 : 0;

	case LFTypeStore:
	case LFTypeFolder:
		UINT Count = 0;

		for (UINT a=1; a<LFColorCount; a++)
			if (pItemDescriptor->AggregateColorSet & (1 << a))
				Count++;

		return Count;
	}

	return 0;
}

INT CFileView::GetColorDotWidth(const LFItemDescriptor* pItemDescriptor, const CIcons& Icons) const
{
	const UINT Count = GetColorDotCount(pItemDescriptor);
	const INT Size = Icons.GetIconSize();

	return Count*(4*Size/3)-((Count>1) ? (Count-1)*(5*Size/6) : 0);
}

void CFileView::DrawColorDots(CDC& dc, CRect& rect, const LFItemDescriptor* pItemDescriptor, INT FontHeight, CIcons& Icons) const
{
	ASSERT(pItemDescriptor);

	BOOL First = TRUE;

	switch (LFGetItemType(pItemDescriptor))
	{
	case LFTypeFile:
		ASSERT(pItemDescriptor->AggregateColorSet==(1 << pItemDescriptor->CoreAttributes.Color));

		if (pItemDescriptor->CoreAttributes.Color)
			DrawColorDot(dc, rect, pItemDescriptor->CoreAttributes.Color, First, Icons, FontHeight);

		break;

	case LFTypeStore:
	case LFTypeFolder:
		for (BYTE a=1; a<LFColorCount; a++)
			if (pItemDescriptor->AggregateColorSet & (1 << a))
				DrawColorDot(dc, rect, a, First, Icons, FontHeight);

		break;
	}
}

void CFileView::DrawJumboIcon(CDC& dc, Graphics& g, CPoint pt, LFItemDescriptor* pItemDescriptor, INT ThumbnailYOffset) const
{
	theApp.m_IconFactory.DrawJumboIcon(dc, g, pt, pItemDescriptor, theApp.ShowRepresentativeThumbnail(m_ContextViewSettings.SortBy, m_Context) ? p_RawFiles : NULL, TRUE, ThumbnailYOffset);
}


// Drawing

void CFileView::DrawNothing(CDC& dc, CRect rect, BOOL Themed) const
{
	if (m_Context==LFContextStores)
	{
		rect.top += 2*BACKSTAGEBORDER;
		rect.left += BACKSTAGEBORDER;
		rect.right -= BACKSTAGEBORDER;

		CFont* pOldFont = dc.SelectObject(&theApp.m_CaptionFont);

		dc.SetTextColor(Themed ? 0x404040 : GetSysColor(COLOR_WINDOWTEXT));
		dc.DrawText(m_WelcomeCaption, rect, DT_TOP | DT_CENTER | DT_WORDBREAK | DT_HIDEPREFIX);

		rect.top += m_WelcomeCaptionHeight;

		dc.SelectObject(&theApp.m_LargeFont);

		dc.SetTextColor(Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));
		dc.DrawText(m_WelcomeMessage, rect, DT_TOP | DT_CENTER | DT_WORDBREAK | DT_HIDEPREFIX);

		dc.SelectObject(pOldFont);
	}
	else
	{
		CFrontstageScroller::DrawNothing(dc, rect, Themed);
	}
}


BEGIN_MESSAGE_MAP(CFileView, CAbstractFileView)
	ON_WM_CREATE()
	ON_WM_KEYDOWN()

	ON_NOTIFY(REQUEST_DRAWBUTTONFOREGROUND, 2, OnDrawButtonForeground)
	ON_BN_CLICKED(2, OnButtonClicked)

	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()

INT CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CAbstractFileView::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Color dots
	theApp.LoadColorDots(m_LargeColorDots, m_LargeFontHeight);
	theApp.LoadColorDots(m_DefaultColorDots, m_DefaultFontHeight);

	// Command button
	CString tmpStr((LPCSTR)IDM_STORES_ADD);

	const INT Pos = tmpStr.Find(L'\n');
	if (Pos!=-1)
		tmpStr.Delete(0, Pos+1);

	m_wndCommandButton.Create(tmpStr, this, 2, TRUE);

	return 0;
}

void CFileView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	const BOOL Control = (GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0);
	const BOOL Plain = (GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0);

	switch (nChar)
	{
	case 'C':
		if (Control)
			GetOwner()->PostMessage(WM_COMMAND, IDM_FILE_COPY);

		break;

	case 'K':
		if (Control)
			GetOwner()->PostMessage(WM_COMMAND, IDM_FILE_MAKETASK);

		break;

	case 'R':
		if (Control)
			GetOwner()->PostMessage(WM_COMMAND, IDM_FILE_REMEMBER);

		break;

	case VK_F5:
		if (Plain)
			GetTopLevelParent()->PostMessage(WM_COMMAND, ID_NAV_RELOAD);

		break;

	case VK_BACK:
		if (Plain)
			GetTopLevelParent()->PostMessage(WM_COMMAND, ID_NAV_BACK);

		break;

	default:
		CAbstractFileView::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}


// Command button

void CFileView::OnDrawButtonForeground(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_DRAWBUTTONFOREGROUND* pDrawButtonForeground = (NM_DRAWBUTTONFOREGROUND*)pNMHDR;
	LPDRAWITEMSTRUCT lpDrawItemStruct = pDrawButtonForeground->lpDrawItemStruct;

	// State
	const BOOL Disabled = (lpDrawItemStruct->itemState & ODS_DISABLED);

	// Content
	CRect rect(lpDrawItemStruct->rcItem);

	CFont* pOldFont = pDrawButtonForeground->pDC->SelectObject(&theApp.m_DefaultFont);

	// Icon
	p_TaskIcons->Draw(*pDrawButtonForeground->pDC, rect.left+(rect.Width()-p_TaskIcons->GetIconSize())/2, rect.top+BUTTONPADDING, 0, pDrawButtonForeground->Themed && pDrawButtonForeground->Hover, Disabled);

	rect.DeflateRect(BUTTONPADDING, BUTTONPADDING);
	rect.top += p_TaskIcons->GetIconSize()+BUTTONPADDING-2;

	// Text
	WCHAR Caption[256];
	::GetWindowText(pDrawButtonForeground->hdr.hwndFrom, Caption, 256);

	pDrawButtonForeground->pDC->DrawText(Caption, -1, rect, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_HIDEPREFIX | DT_TOP);

	pDrawButtonForeground->pDC->SelectObject(pOldFont);

	*pResult = TRUE;
}

void CFileView::OnButtonClicked()
{
	GetOwner()->PostMessage(WM_COMMAND, IDM_STORES_ADD);
}


// Context menu

void CFileView::OnMeasureItem(INT nIDCtl, LPMEASUREITEMSTRUCT lpmis)
{
	if (!lpmis || (lpmis->CtlType!=ODT_MENU))
	{
		CAbstractFileView::OnMeasureItem(nIDCtl, lpmis);

		return;
	}

	if ((lpmis->itemID>=FIRSTSENDTO) && (lpmis->itemID<=LASTSENDTO))
	{
		lpmis->itemWidth = GetSystemMetrics(SM_CXSMICON)*5/4;
		lpmis->itemHeight = GetSystemMetrics(SM_CYSMICON);
	}
}

void CFileView::OnDrawItem(INT nIDCtl, LPDRAWITEMSTRUCT lpdis)
{
	if (!lpdis || (lpdis->CtlType!=ODT_MENU))
	{
		CAbstractFileView::OnDrawItem(nIDCtl, lpdis);

		return;
	}

	if ((lpdis->itemID>=FIRSTSENDTO) && (lpdis->itemID<=LASTSENDTO))
	{
		const UINT Index = lpdis->itemID % 0xFF;

		DrawIconEx(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, m_SendToItems[Index].hIcon, m_SendToItems[Index].cx, m_SendToItems[Index].cy, 0, NULL, DI_NORMAL);
	}
}
