
// CFileView.cpp: Implementierung der Klasse CFileView
//

#include "stdafx.h"
#include "liquidFOLDERS.h"


BOOL AttributeSortableInView(UINT Attr, UINT ViewMode)
{
	BOOL b = theApp.m_Attributes[Attr].Sortable;

	switch (ViewMode)
	{
	case LFViewCalendar:
	case LFViewTimeline:
		b &= (theApp.m_Attributes[Attr].Type==LFTypeTime);
		break;

	case LFViewGlobe:
		b &= ((Attr==LFAttrLocationIATA) || (theApp.m_Attributes[Attr].Type==LFTypeGeoCoordinates));
		break;
	}

	return b;
}

HBITMAP IconToBitmap(HICON hIcon, INT cx, INT cy)
{
	if (!hIcon)
		return NULL;

	HDC hDC = CreateCompatibleDC(NULL);

	HBITMAP hBitmap = CreateTransparentBitmap(cx, cy);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDC, hBitmap);

	if (theApp.OSVersion==OS_XP)
	{
		CRect rect(0, 0, cx, cy);
		FillRect(hDC, rect, (HBRUSH)GetSysColorBrush(COLOR_MENU));
	}

	DrawIconEx(hDC, 0, 0, hIcon, cx, cy, 0, NULL, DI_NORMAL);

	SelectObject(hDC, hOldBitmap);
	DeleteDC(hDC);

	return hBitmap;
}

void AppendSendToItem(CMenu* pMenu, UINT nID, LPCWSTR lpszNewItem, HICON hIcon, INT cx, INT cy, SendToItemData* id)
{
	UINT Index = nID % 0xFF;

	pMenu->AppendMenu(MF_STRING, nID, lpszNewItem);

	if (hIcon)
	{
		id[Index].hBitmap = IconToBitmap(hIcon, cx, cy);
		DestroyIcon(hIcon);

		MENUITEMINFO mii;
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_BITMAP;
		mii.hbmpItem = id[Index].hBitmap;
		SetMenuItemInfo(*pMenu, pMenu->GetMenuItemCount()-1, TRUE, &mii);
	}
}


// CFileView
//

#define GetItemData(Index)     ((FVItemData*)(m_ItemData+Index*m_DataSize))
#define IsSelected(Index)      GetItemData(Index)->Selected
#define ChangedItem(Index)     { InvalidateItem(Index); GetParent()->SendMessage(WM_UPDATESELECTION); }
#define ChangedItems()         { Invalidate(); GetParent()->SendMessage(WM_UPDATESELECTION); }
#define FooterMargin           8

CFileView::CFileView(UINT DataSize, BOOL EnableScrolling, BOOL EnableHover, BOOL EnableTooltip, BOOL EnableShiftSelection, BOOL EnableLabelEdit, BOOL EnableTooltipOnVirtual)
	: CWnd()
{
	ASSERT(DataSize>=sizeof(FVItemData));

	p_RawFiles = p_CookedFiles = NULL;
	p_Edit = NULL;
	m_ItemData = NULL;
	m_ItemDataAllocated = 0;
	m_FocusItem = m_HotItem = m_SelectionAnchor = m_EditLabel = m_Context = -1;
	m_Context = LFContextAllFiles;
	m_HeaderHeight = m_FontHeight[0] = m_FontHeight[1] = m_FontHeight[2] = m_FontHeight[3] = m_HScrollMax = m_VScrollMax = m_HScrollPos = m_VScrollPos = 0;
	m_DataSize = DataSize;
	m_Nothing = TRUE;
	m_Hover = m_BeginDragDrop = m_ShowFocusRect = m_AllowMultiSelect = FALSE;
	hThemeList = NULL;

	m_EnableScrolling = EnableScrolling;
	m_EnableHover = EnableHover;
	m_EnableTooltip = EnableTooltip;
	m_EnableShiftSelection = EnableShiftSelection;
	m_EnableLabelEdit = EnableLabelEdit;
	m_EnableTooltipOnVirtual = EnableTooltipOnVirtual;
}

CFileView::~CFileView()
{
	DestroyEdit();

	if (m_ItemData)
		free(m_ItemData);
}

BOOL CFileView::Create(CWnd* pParentWnd, UINT nID, CRect rect, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data, UINT nClassStyle)
{
	CString className = AfxRegisterWndClass(nClassStyle, theApp.LoadStandardCursor(IDC_ARROW));

	DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP;
	if (m_EnableScrolling)
		dwStyle |= WS_HSCROLL | WS_VSCROLL;

	if (!CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID))
		return FALSE;

	UpdateViewOptions(pCookedFiles ? pCookedFiles->m_Context : LFContextAllFiles, TRUE);
	UpdateSearchResult(pRawFiles, pCookedFiles, Data, TRUE);
	return TRUE;
}

BOOL CFileView::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (p_Edit)
			switch (pMsg->wParam)
			{
			case VK_EXECUTE:
			case VK_RETURN:
				DestroyEdit(TRUE);
				return TRUE;

			case VK_ESCAPE:
				DestroyEdit(FALSE);
				return TRUE;
			}

		break;

	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
		if (p_Edit)
			return TRUE;

		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONUP:
		m_TooltipCtrl.Deactivate();
		break;
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void CFileView::UpdateViewOptions(INT Context, BOOL Force)
{
	DestroyEdit();
	m_TooltipCtrl.Deactivate();

	if (Context>=0)
		m_Context = Context;
	p_ViewParameters = &theApp.m_Views[m_Context];

	m_BeginDragDrop = FALSE;
	m_EditLabel = -1;
	SetViewOptions(Force);

	BOOL Arrange = (m_ViewParameters.Mode!=p_ViewParameters->Mode);
	m_ViewParameters = *p_ViewParameters;

	if (Arrange)
	{
		AdjustLayout();
	}
	else
	{
		Invalidate();
	}
}

void CFileView::UpdateSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data, BOOL InternalCall)
{
	DestroyEdit();
	m_TooltipCtrl.Deactivate();

	void* Victim = m_ItemData;
	UINT VictimAllocated = m_ItemDataAllocated;

	m_Nothing = TRUE;

	if (pCookedFiles)
	{
#ifdef DEBUG
		m_AllowMultiSelect = TRUE;
#else
		m_AllowMultiSelect = (pCookedFiles->m_Context!=LFContextStores);
#endif

		SIZE_T sz = pCookedFiles->m_ItemCount*m_DataSize;
		m_ItemData = (BYTE*)malloc(sz);
		m_ItemDataAllocated = pCookedFiles->m_ItemCount;
		ZeroMemory(m_ItemData, sz);

		if (VictimAllocated)
		{
			INT RetainSelection = Data ? Data->FocusItem : -1;

			for (UINT a=0; a<min(VictimAllocated, pCookedFiles->m_ItemCount); a++)
			{
				FVItemData* d = GetItemData(a);

				BYTE* v = (BYTE*)Victim;
				v += ((BYTE*)d)-((BYTE*)m_ItemData);

				if ((m_AllowMultiSelect) || ((INT)a==RetainSelection))
					d->Selected = ((FVItemData*)v)->Selected;
			}
		}

		m_Context = pCookedFiles->m_Context;
		p_ViewParameters = &theApp.m_Views[m_Context];
		m_ViewParameters.SortBy = p_ViewParameters->SortBy;

		m_FocusItem = Data ? min(Data->FocusItem, (INT)pCookedFiles->m_ItemCount-1) : pCookedFiles->m_ItemCount ? 0 : -1;
		m_HScrollPos = Data ? Data->HScrollPos : 0;
		m_VScrollPos = Data ? Data->VScrollPos : 0;
		m_HotItem = -1;
		m_HideFileExt = LFHideFileExt();
	}
	else
	{
		m_ItemData = NULL;
		m_ItemDataAllocated = 0;

		m_FocusItem = m_HotItem = -1;
		m_HScrollPos = m_VScrollPos = 0;

		m_ShowFocusRect = FALSE;
	}

	m_BeginDragDrop = FALSE;
	m_EditLabel = -1;
	SetSearchResult(pRawFiles, pCookedFiles, Data);

	free(Victim);

	if (p_CookedFiles)
	{
		BOOL NeedNewFocusItem = (m_FocusItem>=0) ? !GetItemData(m_FocusItem)->Valid : TRUE;

		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			FVItemData* d = GetItemData(a);
			d->Selected &= d->Valid;
			m_Nothing &= !d->Valid;

			if (NeedNewFocusItem && d->Valid)
			{
				m_FocusItem = a;
				NeedNewFocusItem = FALSE;
			}
		}

		AdjustLayout();

		if (!InternalCall)
			EnsureVisible(m_FocusItem);
	}
	else
	{
		Invalidate();
	}

	SetCursor(theApp.LoadStandardCursor(pCookedFiles ? IDC_ARROW : IDC_WAIT));
}

void CFileView::SetViewOptions(BOOL /*Force*/)
{
}

void CFileView::SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* /*Data*/)
{
	p_RawFiles = pRawFiles;
	p_CookedFiles = pCookedFiles;
}

void CFileView::AdjustLayout()
{
	AdjustScrollbars();
	Invalidate();
}

INT CFileView::GetFocusItem()
{
	if (p_CookedFiles)
	{
		FVItemData* d = GetItemData(m_FocusItem);
		return d->Valid ? m_FocusItem : -1;
	}

	return -1;
}

INT CFileView::GetSelectedItem()
{
	if (p_CookedFiles)
		if (p_CookedFiles->m_ItemCount)
		{
			FVItemData* d = GetItemData(m_FocusItem);
			return (d->Selected && d->Valid) ? m_FocusItem : -1;
		}

	return -1;
}

INT CFileView::GetNextSelectedItem(INT Index)
{
	if (p_CookedFiles)
	{
		ASSERT(Index>=-1);

		while (++Index<(INT)p_CookedFiles->m_ItemCount)
		{
			FVItemData* d = GetItemData(Index);
			if (d->Selected && d->Valid)
				return Index;
		}
	}

	return -1;
}

void CFileView::SelectItem(INT Index, BOOL Select, BOOL InternalCall)
{
	if (p_CookedFiles)
	{
		ASSERT(Index<(INT)p_CookedFiles->m_ItemCount);

		FVItemData* d = GetItemData(Index);
		if (d->Valid)
		{
			d->Selected = Select;
			if (!InternalCall)
				ChangedItem(Index);
		}
	}
}

void CFileView::EnsureVisible(INT Index)
{
	if (!m_EnableScrolling)
		return;

	CRect rect;
	GetClientRect(&rect);

	RECT rectItem = GetItemRect(Index);

	SCROLLINFO si;
	INT nInc;

	// Vertikal
	nInc = 0;
	if (rectItem.bottom>rect.Height())
		nInc = rectItem.bottom-rect.Height();
	if (rectItem.top<nInc+(INT)m_HeaderHeight)
		nInc = rectItem.top-(INT)m_HeaderHeight;

	nInc = max(-m_VScrollPos, min(nInc, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindow(0, -nInc);

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = m_VScrollPos;
		SetScrollInfo(SB_VERT, &si);
	}

	// Horizontal
	if (m_ViewParameters.Mode!=LFViewDetails)
	{
		nInc = 0;
		if (rectItem.right>rect.Width())
			nInc = rectItem.right-rect.Width();
		if (rectItem.left<nInc)
			nInc = rectItem.left;

		nInc = max(-m_HScrollPos, min(nInc, m_HScrollMax-m_HScrollPos));
		if (nInc)
		{
			m_HScrollPos += nInc;
			ScrollWindow(-nInc, 0);
	
			ZeroMemory(&si, sizeof(si));
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_POS;
			si.nPos = m_HScrollPos;
			SetScrollInfo(SB_HORZ, &si);
		}
	}
}

BOOL CFileView::MultiSelectAllowed()
{
	return m_AllowMultiSelect;
}

void CFileView::SetFocusItem(INT FocusItem, BOOL ShiftSelect)
{
	if (!m_AllowMultiSelect)
		ShiftSelect = FALSE;

	if (ShiftSelect && m_EnableShiftSelection)
	{
		if (m_SelectionAnchor==-1)
			m_SelectionAnchor = m_FocusItem;

		for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
			SelectItem(a, ((a>=FocusItem) && (a<=m_SelectionAnchor)) || ((a>=m_SelectionAnchor) && (a<=FocusItem)), TRUE);
	}
	else
	{
		m_SelectionAnchor = -1;

		for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
			SelectItem(a, a==FocusItem, TRUE);
	}

	m_FocusItem = FocusItem;
	m_EditLabel = -1;

	ChangedItems();
	EnsureVisible(m_FocusItem);
}

RECT CFileView::GetItemRect(INT Index)
{
	RECT rect = { 0, 0, 0, 0 };

	if (p_CookedFiles)
		if ((Index>=0) && (Index<(INT)p_CookedFiles->m_ItemCount))
		{
			rect = GetItemData(Index)->Rect;
			OffsetRect(&rect, -m_HScrollPos, -m_VScrollPos+(INT)m_HeaderHeight);
		}

	return rect;
}

RECT CFileView::GetLabelRect(INT Index)
{
	return GetItemRect(Index);
}

INT CFileView::ItemAtPosition(CPoint point)
{
	if (p_CookedFiles)
	{
		point.Offset(m_HScrollPos, m_VScrollPos-m_HeaderHeight);

		for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
			if (PtInRect(&GetItemData(a)->Rect, point))
				return a;
	}

	return -1;
}

void CFileView::InvalidateItem(INT Index)
{
	if (p_CookedFiles)
		if ((Index>=0) && (Index<(INT)p_CookedFiles->m_ItemCount))
		{
			RECT rect = GetItemRect(Index);
			InflateRect(&rect, GetItemData(Index)->RectInflate, 0);
			InvalidateRect(&rect);
		}
}

CMenu* CFileView::GetViewContextMenu()
{
	return NULL;
}

CMenu* CFileView::GetSendToMenu()
{
	CMenu* pMenu = new CMenu();
	pMenu->CreatePopupMenu();

	MENUINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_STYLE;
	mi.dwStyle = MNS_CHECKORBMP;
	SetMenuInfo(*pMenu, &mi);

	INT cx = GetSystemMetrics(SM_CXSMICON);
	INT cy = GetSystemMetrics(SM_CYSMICON);

	BOOL Added = FALSE;

	// Stores
	UINT nID = 0xFF00;
	if (LFDefaultStoreAvailable())
	{
		CString tmpStr((LPCSTR)IDS_DEFAULTSTORE);
		AppendSendToItem(pMenu, nID, tmpStr, (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_APPLICATION), IMAGE_ICON, cx, cy, LR_SHARED), cx, cy, m_SendToItems);
		Added = TRUE;

		INT Index = (nID++) & 0xFF;
		m_SendToItems[Index].IsStore = TRUE;
		strcpy_s(m_SendToItems[Index].StoreID, LFKeySize, "");
	}

	if (LFGetStoreCount())
	{
		CString tmpStr((LPCSTR)IDS_CONTEXTMENU_CHOOSESTORE);
		AppendSendToItem(pMenu, nID, tmpStr, NULL, cx, cy, m_SendToItems);
		Added = TRUE;

		INT Index = (nID++) & 0xFF;
		m_SendToItems[Index].IsStore = TRUE;
		strcpy_s(m_SendToItems[Index].StoreID, LFKeySize, "CHOOSE");
	}


	// SendTo shortcuts
	nID = 0xFF40;
	if (Added)
	{
		pMenu->AppendMenu(MF_SEPARATOR);
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

					WCHAR* LastExt = wcsrchr(Name, L'.');
					if (LastExt)
						if (*LastExt!=L'\0')
						{
							CString Ext(LastExt);
							Ext.MakeUpper();
							if ((Ext==_T(".DESKLINK")) || (Ext==_T(".LFSENDTO")))
								continue;

							*LastExt = L'\0';
						}

					WCHAR Dst[MAX_PATH];
					wcscpy_s(Dst, MAX_PATH, Path);
					wcscat_s(Dst, MAX_PATH, L"\\");
					wcscat_s(Dst, MAX_PATH, ffd.cFileName);

					SHFILEINFO sfi;
					if (SHGetFileInfo(Dst, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON))
					{
						AppendSendToItem(pMenu, nID, Name, theApp.m_SystemImageListSmall.ExtractIcon(sfi.iIcon), cx, cy, m_SendToItems);
						Added = TRUE;

						INT Index = (nID++) & 0xFF;
						m_SendToItems[Index].IsStore = FALSE;
						wcscpy_s(m_SendToItems[Index].Path, MAX_PATH, Dst);
					}
				}
			}
			while (FindNextFile(hFind, &ffd));

		FindClose(hFind);
	}

	// Volumes
	DWORD VolumesOnSystem = LFGetLogicalVolumes(LFGLV_EXTERNAL | LFGLV_NETWORK | LFGLV_INCLUDEFLOPPIES);
	DWORD VolumesWOFloppies = LFGetLogicalVolumes(LFGLV_EXTERNAL | LFGLV_NETWORK);
	BOOL HideVolumesWithNoMedia = LFHideVolumesWithNoMedia();

	for (CHAR cVolume='A'; cVolume<='Z'; cVolume++, VolumesOnSystem>>=1, VolumesWOFloppies>>=1)
	{
		if ((VolumesOnSystem & 1)==0)
			continue;

		BOOL CheckEmpty = HideVolumesWithNoMedia && ((VolumesWOFloppies & 1)!=0);

		WCHAR szDriveRoot[] = L" :\\";
		szDriveRoot[0] = cVolume;
		SHFILEINFO sfi;
		if (SHGetFileInfo(szDriveRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON | (CheckEmpty ? SHGFI_ATTRIBUTES : 0)))
		{
			if ((!sfi.dwAttributes) && CheckEmpty)
				continue;

			if (Added)
			{
				pMenu->AppendMenu(MF_SEPARATOR);
				Added = FALSE;
			}

			AppendSendToItem(pMenu, nID, sfi.szDisplayName, sfi.hIcon, cx, cy, m_SendToItems);

			INT Index = (nID++) & 0xFF;
			m_SendToItems[Index].IsStore = FALSE;
			wcscpy_s(m_SendToItems[Index].Path, MAX_PATH, szDriveRoot);
		}
	}

	return pMenu;
}

CMenu* CFileView::GetItemContextMenu(INT Index)
{
	UINT InsertPos = 0;

	CMenu* pMenu = new CMenu();

	LFItemDescriptor* Item = p_CookedFiles->m_Items[Index];
	switch (Item->Type & LFTypeMask)
	{
	case LFTypeVolume:
		pMenu->LoadMenu(IDM_VOLUME);
		pMenu->GetSubMenu(0)->InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);
		break;

	case LFTypeStore:
		pMenu->LoadMenu(IDM_STORE);
		pMenu->GetSubMenu(0)->InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);
		break;

	case LFTypeFile:
		if ((m_Context==LFContextArchive) || (m_Context==LFContextTrash))
		{
			pMenu->LoadMenu(IDM_FILE_AWAY);
			InsertPos = 2;
		}
		else
		{
			pMenu->LoadMenu(IDM_FILE);
		}

		break;

	case LFTypeFolder:
		if ((Item->FirstAggregate!=-1) && (Item->LastAggregate!=-1))
			pMenu->LoadMenu(IDM_FOLDER);
	}

	if (!IsMenu(*pMenu))
	{
		pMenu->CreatePopupMenu();
		pMenu->AppendMenu(MF_POPUP, (UINT_PTR)CreateMenu(), _T("POPUP"));
	}

	CMenu* pPopup = pMenu->GetSubMenu(0);
	ASSERT_VALID(pPopup);

	CString tmpStr;

	if (m_Context!=LFContextTrash)
		if (((Item->Type & LFTypeMask)==LFTypeFile) || (((Item->Type & LFTypeMask)==LFTypeFolder) && (Item->FirstAggregate!=-1) && (Item->LastAggregate!=-1)))
		{
			ENSURE(tmpStr.LoadString(m_Context==LFContextClipboard ? IDS_CONTEXTMENU_REMOVE : IDS_CONTEXTMENU_REMEMBER));
			pPopup->InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, m_Context==LFContextClipboard ? IDM_FILE_REMOVE : IDM_FILE_REMEMBER, tmpStr);

			CMenu* pSendPopup = GetSendToMenu();

			ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_SENDTO));
			pPopup->InsertMenu(InsertPos, MF_POPUP | MF_BYPOSITION, (UINT_PTR)pSendPopup->m_hMenu, tmpStr);
			pPopup->InsertMenu(InsertPos, MF_SEPARATOR | MF_BYPOSITION);

			pSendPopup->Detach();
			delete pSendPopup;
		}

	switch (Item->Type & LFTypeMask)
	{
	case LFTypeStore:
		ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_OPENFILEDROP));
		pPopup->InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, IDM_ITEM_OPENFILEDROP, tmpStr);

		ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_OPENNEWWINDOW));
		pPopup->InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, IDM_ITEM_OPENNEWWINDOW, tmpStr);

		break;

	case LFTypeFile:
		if (Item->CoreAttributes.URL[0]!='\0')
		{
			ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_OPENBROWSER));
			pPopup->InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, IDM_FILE_OPENBROWSER, tmpStr);
		}

		if (Item->CoreAttributes.ContextID==LFContextFilters)
		{
			ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_EDIT));
			pPopup->InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, IDM_FILE_EDIT, tmpStr);
		}
		else
		{
			ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_OPENWITH));
			pPopup->InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, IDM_FILE_OPENWITH, tmpStr);
		}

		break;
	}

	ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_OPEN));
	pPopup->InsertMenu(InsertPos, MF_STRING | MF_BYPOSITION, IDM_ITEM_OPEN, tmpStr);
	pPopup->SetDefaultItem(InsertPos, TRUE);

	return pMenu;
}

void CFileView::GetPersistentData(FVPersistentData& Data)
{
	ZeroMemory(&Data, sizeof(Data));

	Data.FocusItem = m_FocusItem;
	Data.HScrollPos = m_HScrollPos;
	Data.VScrollPos = m_VScrollPos;
}

void CFileView::EditLabel(INT Index)
{
	m_EditLabel = -1;

	if ((m_EnableLabelEdit) && (p_CookedFiles) && (m_Context!=LFContextArchive) && (m_Context!=LFContextTrash))
	{
		LFItemDescriptor* Item = p_CookedFiles->m_Items[Index];
		if (((Item->Type & (LFTypeNotMounted | LFTypeMask))==LFTypeVolume) ||
			((Item->Type & LFTypeMask)==LFTypeStore) ||
			((Item->Type & (LFTypeNotMounted | LFTypeMask))==LFTypeFile))
		{
			m_EditLabel = Index;
			InvalidateItem(Index);
			EnsureVisible(Index);

			CRect rect(GetLabelRect(Index));
			if (rect.Height()>m_FontHeight[0]+4)
			{
				rect.top += (rect.Height()-m_FontHeight[0]-4)/2;
				rect.bottom = rect.top+m_FontHeight[0]+4;
			}

			p_Edit = new CEdit();
			p_Edit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ES_AUTOHSCROLL, rect, this, 2);
			p_Edit->SetWindowText(Item->CoreAttributes.FileName);
			p_Edit->SetFont(&theApp.m_DefaultFont);
			p_Edit->SetFocus();
			p_Edit->SetSel(0, -1);
		}
	}
}

BOOL CFileView::IsEditing()
{
	return (p_Edit!=NULL);
}

void CFileView::DrawItemBackground(CDC& dc, LPRECT rectItem, INT Index, BOOL Themed)
{
	BOOL Hot = (m_HotItem==Index);
	BOOL Selected = IsSelected(Index);

	if (hThemeList)
	{
		dc.SetTextColor((p_CookedFiles->m_Items[Index]->CoreAttributes.Flags & LFFlagMissing) ? 0x0000FF : 0x000000);

		if (Hot | Selected)
		{
			const INT StateIDs[4] = { LISS_NORMAL, LISS_HOT, GetFocus()!=this ? LISS_SELECTEDNOTFOCUS : LISS_SELECTED, LISS_HOTSELECTED };
			UINT State = 0;
			if (Hot)
				State |= 1;
			if (Selected)
				State |= 2;
			theApp.zDrawThemeBackground(hThemeList, dc, LVP_LISTITEM, StateIDs[State], rectItem, rectItem);
		}

		if ((GetFocus()==this) && (m_FocusItem==Index))
			switch (theApp.OSVersion)
			{
			case OS_Vista:
				if (m_ShowFocusRect)
				{
					CRect rect(rectItem);
					rect.DeflateRect(1, 1);

					dc.SetBkColor(0xFFFFFF);
					dc.DrawFocusRect(rect);
				}

				break;

			case OS_Seven:
			case OS_Eight:
				if (!Selected)
				{
					CRect rect(rectItem);
					rect.bottom--;
					rect.right--;

					Graphics g(dc);
					g.SetSmoothingMode(SmoothingModeAntiAlias);

					GraphicsPath path;
					CreateRoundRectangle(rect, 2, path);

					Pen pen(Color(0xFF, 0x7D, 0xA2, 0xCE));
					g.DrawPath(&pen, &path);
				}

				break;
			}
	}
	else
	{
		if (Selected)
		{
			dc.FillSolidRect(rectItem, GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHT : COLOR_3DFACE));
			dc.SetTextColor(GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));
			dc.SetBkColor(0x000000);
		}
		else
		{
			dc.SetTextColor(Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));
			dc.SetBkColor(0xFFFFFF);
		}

		if ((Index==m_FocusItem) && (GetFocus()==this))
			dc.DrawFocusRect(rectItem);

		if (!Selected)
			if (p_CookedFiles->m_Items[Index]->CoreAttributes.Flags & LFFlagMissing)
			{
				dc.SetTextColor(0x0000FF);
			}
	}
}

void CFileView::ResetScrollbars()
{
	if (m_EnableScrolling)
	{
		ScrollWindow(0, m_VScrollPos);
		ScrollWindow(m_HScrollPos, 0);

		m_VScrollPos = m_HScrollPos = 0;

		SetScrollPos(SB_VERT, m_VScrollPos, TRUE);
		SetScrollPos(SB_HORZ, m_HScrollPos, TRUE);
	}
}

void CFileView::AdjustScrollbars()
{
	if (!m_EnableScrolling)
		return;

	CRect rect;
	GetWindowRect(&rect);

	BOOL HScroll = FALSE;
	if (m_ScrollWidth>rect.Width())
	{
		rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
		HScroll = TRUE;
	}
	if (m_ScrollHeight>rect.Height()-(INT)m_HeaderHeight)
		rect.right -= GetSystemMetrics(SM_CXVSCROLL);
	if ((m_ScrollWidth>rect.Width()) && (!HScroll))
		rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);

	INT OldVScrollPos = m_VScrollPos;
	m_VScrollMax = max(0, m_ScrollHeight-rect.Height()+(INT)m_HeaderHeight);
	m_VScrollPos = min(m_VScrollPos, m_VScrollMax);

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rect.Height()-m_HeaderHeight;
	si.nMin = 0;
	si.nMax = m_ScrollHeight-1;
	si.nPos = m_VScrollPos;
	SetScrollInfo(SB_VERT, &si);

	INT OldHScrollPos = m_HScrollPos;
	m_HScrollMax = max(0, m_ScrollWidth-rect.Width());
	m_HScrollPos = min(m_HScrollPos, m_HScrollMax);

	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rect.Width();
	si.nMin = 0;
	si.nMax = m_ScrollWidth-1;
	si.nPos = m_HScrollPos;
	SetScrollInfo(SB_HORZ, &si);

	if ((OldVScrollPos!=m_VScrollPos) || (OldHScrollPos!=m_HScrollPos))
		Invalidate();
}

CString CFileView::GetLabel(LFItemDescriptor* i)
{
	CString Label = i->CoreAttributes.FileName;

	switch (i->Type & LFTypeMask)
	{
	case LFTypeVolume:
		Label += _T(" (");
		Label += i->CoreAttributes.FileID[0];
		Label += _T(":)");

		break;

	case LFTypeFile:
		if (((!m_HideFileExt) || (i->CoreAttributes.FileName[0]==L'\0')) && (i->CoreAttributes.FileFormat[0]!='\0') && (strcmp(i->CoreAttributes.FileFormat, "filter")!=0))
		{
			Label += _T(".");
			Label += i->CoreAttributes.FileFormat;
		}

		break;
	}

	return Label;
}

BOOL CFileView::BeginDragDrop()
{
	m_BeginDragDrop = FALSE;

	GetParent()->SendMessage(WM_BEGINDRAGDROP);

	return TRUE;
}

CString CFileView::GetHint(LFItemDescriptor* i, WCHAR* FormatName)
{
	WCHAR tmpStr[256];
	CString Hint;

	switch (i->Type & LFTypeMask)
	{
	case LFTypeVolume:
		AppendTooltipAttribute(i, LFAttrDescription, Hint);
		break;

	case LFTypeStore:
		GetHintForStore(i, Hint);
		break;

	case LFTypeFile:
		AppendTooltipAttribute(i, LFAttrComments, Hint);
		AppendTooltipString(LFAttrFileFormat, Hint, FormatName);

		if ((i->Type & LFTypeSourceMask)>LFTypeSourceInternal)
		{
			tmpStr[0] = L' ';
			wcscpy_s(&tmpStr[1], 255, theApp.m_SourceNames[i->Type & LFTypeSourceMask][1]);
			tmpStr[1] = (WCHAR)towlower(tmpStr[1]);

			Hint += tmpStr;
		}

		AppendTooltipAttribute(i, LFAttrArtist, Hint);
		AppendTooltipAttribute(i, LFAttrTitle, Hint);
		AppendTooltipAttribute(i, LFAttrAlbum, Hint);
		AppendTooltipAttribute(i, LFAttrRecordingTime, Hint);
		AppendTooltipAttribute(i, LFAttrRoll, Hint);
		AppendTooltipAttribute(i, LFAttrDuration, Hint);
		AppendTooltipAttribute(i, LFAttrHashtags, Hint);
		AppendTooltipAttribute(i, LFAttrPages, Hint);
		AppendTooltipAttribute(i, LFAttrWidth, Hint);
		AppendTooltipAttribute(i, LFAttrHeight, Hint);
		AppendTooltipAttribute(i, LFAttrEquipment, Hint);
		AppendTooltipAttribute(i, LFAttrBitrate, Hint);
		AppendTooltipAttribute(i, LFAttrCreationTime, Hint);
		AppendTooltipAttribute(i, LFAttrFileTime, Hint);
		AppendTooltipAttribute(i, LFAttrFileSize, Hint);

		break;

	case LFTypeFolder:
		AppendTooltipAttribute(i, LFAttrComments, Hint);
		AppendTooltipAttribute(i, LFAttrDescription, Hint);

		if ((i->Type & LFTypeSourceMask)>LFTypeSourceInternal)
			AppendTooltipString(LFAttrComments, Hint, theApp.m_SourceNames[i->Type & LFTypeSourceMask][1]);

		break;
	}

	return Hint;
}

void CFileView::DestroyEdit(BOOL Accept)
{
	if (p_Edit)
	{
		INT Item = m_EditLabel;

		CEdit* victim = p_Edit;
		p_Edit = NULL;

		CString Name;
		victim->GetWindowText(Name);
		victim->DestroyWindow();
		delete victim;

		if ((Accept) && (!Name.IsEmpty()) && (Item!=-1))
			GetParent()->SendMessage(WM_RENAMEITEM, (WPARAM)Item, (LPARAM)Name.GetBuffer());
	}

	m_EditLabel = -1;
}

void CFileView::ScrollWindow(INT dx, INT dy)
{
	ASSERT(m_EnableScrolling);

	CWnd::ScrollWindow(dx, dy);
}


BEGIN_MESSAGE_MAP(CFileView, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_THEMECHANGED()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEHWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SETCURSOR()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE_VOID(WM_SELECTALL, OnSelectAll)
	ON_MESSAGE_VOID(WM_SELECTNONE, OnSelectNone)
	ON_COMMAND(IDM_SELECTALL, OnSelectAll)
	ON_COMMAND(IDM_SELECTNONE, OnSelectNone)
	ON_COMMAND(IDM_SELECTINVERT, OnSelectInvert)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_SELECTALL, IDM_SELECTINVERT, OnUpdateCommands)
	ON_EN_KILLFOCUS(2, OnDestroyEdit)
END_MESSAGE_MAP()

INT CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	m_TooltipCtrl.Create(this);

	if (theApp.m_ThemeLibLoaded)
		if (theApp.OSVersion>=OS_Vista)
		{
			theApp.zSetWindowTheme(GetSafeHwnd(), L"EXPLORER", NULL);
			hThemeList = theApp.zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
		}

	if (m_EnableScrolling)
		ResetScrollbars();

	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&theApp.m_DefaultFont);
	m_FontHeight[0] = m_RowHeight = dc->GetTextExtent(_T("Wy")).cy;
	dc->SelectObject(&theApp.m_LargeFont);
	m_FontHeight[1] = dc->GetTextExtent(_T("Wy")).cy;
	dc->SelectStockObject(DEFAULT_GUI_FONT);
	m_FontHeight[2] = dc->GetTextExtent(_T("Wy")).cy;
	dc->SelectObject(&theApp.m_SmallFont);
	m_FontHeight[3] = dc->GetTextExtent(_T("Wy")).cy;
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	return 0;
}

void CFileView::OnDestroy()
{
	if (hThemeList)
		theApp.zCloseThemeData(hThemeList);

	CWnd::OnDestroy();
}

LRESULT CFileView::OnThemeChanged()
{
	if (theApp.m_ThemeLibLoaded)
	{
		if (hThemeList)
			theApp.zCloseThemeData(hThemeList);

		if (theApp.OSVersion>=OS_Vista)
			hThemeList = theApp.zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
	}

	return TRUE;
}

void CFileView::OnSysColorChange()
{
	Invalidate();
}

BOOL CFileView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CFileView::OnSize(UINT nType, INT cx, INT cy)
{
	SetRedraw(FALSE);
	CWnd::OnSize(nType, cx, cy);
	SetRedraw(TRUE);

	AdjustLayout();
}

void CFileView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CRect rect;
	GetClientRect(&rect);

	SCROLLINFO si;

	INT nInc = 0;
	switch (nSBCode)
	{
	case SB_TOP:
		nInc = -m_VScrollPos;
		break;

	case SB_BOTTOM:
		nInc = m_VScrollMax-m_VScrollPos;
		break;

	case SB_LINEUP:
		nInc = -m_RowHeight;
		break;

	case SB_LINEDOWN:
		nInc = m_RowHeight;
		break;

	case SB_PAGEUP:
		nInc = min(-1, -(rect.Height()-(INT)m_HeaderHeight));
		break;

	case SB_PAGEDOWN:
		nInc = max(1, rect.Height()-(INT)m_HeaderHeight);
		break;

	case SB_THUMBTRACK:
		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(SB_VERT, &si);

		nInc = si.nTrackPos-m_VScrollPos;
		break;
	}

	nInc = max(-m_VScrollPos, min(nInc, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindow(0, -nInc);

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = m_VScrollPos;
		SetScrollInfo(SB_VERT, &si);

		if (p_Edit)
		{
			CRect rect;
			p_Edit->GetWindowRect(&rect);
			ScreenToClient(&rect);

			rect.OffsetRect(0, -nInc);
			p_Edit->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CFileView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO si;

	INT nInc = 0;
	switch (nSBCode)
	{
	case SB_TOP:
		nInc = -m_HScrollPos;
		break;

	case SB_BOTTOM:
		nInc = m_HScrollMax-m_HScrollPos;
		break;

	case SB_PAGEUP:
	case SB_LINEUP:
		nInc = -64;
		break;

	case SB_PAGEDOWN:
	case SB_LINEDOWN:
		nInc = 64;
		break;

	case SB_THUMBTRACK:
		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(SB_HORZ, &si);

		nInc = si.nTrackPos-m_HScrollPos;
		break;
	}

	nInc = max(-m_HScrollPos, min(nInc, m_HScrollMax-m_HScrollPos));
	if (nInc)
	{
		m_HScrollPos += nInc;
		ScrollWindow(-nInc, 0);

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = m_HScrollPos;
		SetScrollInfo(SB_HORZ, &si);

		UpdateWindow();
	}

	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CFileView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_BeginDragDrop && (nFlags & MK_LBUTTON))
		if ((abs(point.x-m_DragPos.x)>=GetSystemMetrics(SM_CXDRAG)) || (abs(point.y-m_DragPos.y)>=GetSystemMetrics(SM_CYDRAG)))
			if (BeginDragDrop())
				return;

	if (m_EnableHover || m_EnableTooltip)
	{
		INT Index = ItemAtPosition(point);

		if (!m_Hover)
		{
			m_Hover = TRUE;

			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE | TME_HOVER;
			tme.dwHoverTime = LFHOVERTIME;
			tme.hwndTrack = m_hWnd;
			TrackMouseEvent(&tme);
		}
		else
			if ((m_TooltipCtrl.IsWindowVisible()) && (Index!=m_HotItem))
				m_TooltipCtrl.Deactivate();

		if (m_HotItem!=Index)
		{
			if (m_EnableHover)
				InvalidateItem(m_HotItem);
			m_HotItem = Index;
			if (m_EnableHover)
				InvalidateItem(m_HotItem);
		}
	}
}

void CFileView::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();
	InvalidateItem(m_HotItem);

	m_Hover = FALSE;
	m_HotItem = -1;
}

void CFileView::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if ((m_HotItem!=-1) && (!IsEditing()))
			if (m_HotItem==m_EditLabel)
			{
				m_TooltipCtrl.Deactivate();
				EditLabel(m_EditLabel);
			}
			else
				if (!m_TooltipCtrl.IsWindowVisible() && m_EnableTooltip)
				{
					FormatData fd;
					CHAR Path[4];
					HICON hIcon = NULL;

					LFItemDescriptor* i = p_CookedFiles->m_Items[m_HotItem];
					switch (i->Type & LFTypeMask)
					{
					case LFTypeFile:
						if ((theApp.m_Views[m_Context].Mode!=LFViewContent) && (theApp.m_Views[m_Context].Mode!=LFViewPreview) && (theApp.m_Views[m_Context].Mode!=LFViewTimeline))
						{
							CDC* pDC = GetWindowDC();
							hIcon = theApp.m_ThumbnailCache.GetThumbnailIcon(i, pDC);
							ReleaseDC(pDC);
						}

						theApp.m_FileFormats.Lookup(i->CoreAttributes.FileFormat, fd);

						break;

					case LFTypeVolume:
						strcpy_s(Path, 4, " :\\");
						Path[0] = i->CoreAttributes.FileID[0];
						theApp.m_FileFormats.Lookup(Path, fd);

						break;

					case LFTypeFolder:
						if (!m_EnableTooltipOnVirtual)
							goto Leave;

					default:
						fd.FormatName[0] = L'\0';
						fd.SysIconIndex = -1;
					}

					if (!hIcon)
						hIcon = (fd.SysIconIndex>=0) ? theApp.m_SystemImageListExtraLarge.ExtractIcon(fd.SysIconIndex) : theApp.m_CoreImageListExtraLarge.ExtractIcon(i->IconID-1);

					ClientToScreen(&point);
					m_TooltipCtrl.Track(point, hIcon, GetLabel(i), GetHint(i, fd.FormatName));
				}
	}
	else
	{
Leave:
		m_TooltipCtrl.Deactivate();
	}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = LFHOVERTIME;
	tme.hwndTrack = m_hWnd;
	TrackMouseEvent(&tme);
}

BOOL CFileView::OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(&rect);
	if (!rect.PtInRect(pt))
		return FALSE;

	INT nScrollLines;
	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nScrollLines, 0);
	if (nScrollLines<1)
		nScrollLines = 1;

	INT nInc = max(-m_VScrollPos, min(-zDelta*(INT)m_RowHeight*nScrollLines/WHEEL_DELTA, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindow(0, -nInc);
		SetScrollPos(SB_VERT, m_VScrollPos, TRUE);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}

	return TRUE;
}

void CFileView::OnMouseHWheel(UINT nFlags, SHORT zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(&rect);
	if (!rect.PtInRect(pt))
		return;

	INT nInc = max(-m_HScrollPos, min(zDelta*64/WHEEL_DELTA, m_HScrollMax-m_HScrollPos));
	if (nInc)
	{
		m_HScrollPos += nInc;
		ScrollWindow(-nInc, 0);
		SetScrollPos(SB_HORZ, m_HScrollPos, TRUE);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}
}

void CFileView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((!m_ShowFocusRect) && (theApp.OSVersion==OS_Vista))
	{
		m_ShowFocusRect = TRUE;
		InvalidateItem(m_FocusItem);
	}

	switch(nChar)
	{
	case 'A':
		if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
			OnSelectAll();

		break;

	case 'I':
		if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
			OnSelectInvert();

		break;

	case 'N':
		if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
			OnSelectNone();

		break;

	case VK_SPACE:
		if (m_FocusItem!=-1)
			SelectItem(m_FocusItem, (GetKeyState(VK_CONTROL)>=0) ? TRUE : !IsSelected(m_FocusItem));

		break;

	case VK_F2:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			EditLabel(m_FocusItem);

		break;

	case VK_F5:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			GetOwner()->PostMessage(WM_COMMAND, ID_NAV_RELOAD);

		break;

	case VK_BACK:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			GetOwner()->PostMessage(WM_COMMAND, ID_NAV_BACK);

		break;

	case VK_EXECUTE:
	case VK_RETURN:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			GetOwner()->PostMessage(WM_COMMAND, IDM_ITEM_OPEN);

		break;

	case VK_DELETE:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			GetOwner()->PostMessage(WM_COMMAND, IDM_FILE_DELETE);

		break;

	default:
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

void CFileView::OnLButtonDown(UINT nFlags, CPoint point)
{
	DestroyEdit();

	INT Index = ItemAtPosition(point);
	if (Index!=-1)
	{
		if (!m_BeginDragDrop)
		{
			m_BeginDragDrop = TRUE;
			m_DragPos = point;
		}

		if ((nFlags & MK_CONTROL) && (m_AllowMultiSelect))
		{
			InvalidateItem(m_FocusItem);
			m_FocusItem = Index;
			SelectItem(Index, !IsSelected(Index));
		}
		else
			if ((m_FocusItem==Index) && (IsSelected(Index)))
			{
				m_EditLabel = Index;
			}
			else
			{
				SetFocusItem(Index, nFlags & MK_SHIFT);
			}
	}
	else
	{
		if (GetFocus()!=this)
			SetFocus();
	}
}

void CFileView::OnLButtonUp(UINT nFlags, CPoint point)
{
	INT Index = ItemAtPosition(point);
	if (Index!=-1)
	{
		if (GetFocus()!=this)
			SetFocus();
	}
	else
	{
		if (!(nFlags & MK_CONTROL) || (!m_AllowMultiSelect))
			OnSelectNone();
	}

	m_BeginDragDrop = FALSE;
}

void CFileView::OnLButtonDblClk(UINT /*nFlags*/, CPoint point)
{
	DestroyEdit();

	INT Index = ItemAtPosition(point);
	if (Index!=-1)
		GetOwner()->SendMessage(WM_COMMAND, IDM_ITEM_OPEN);
}

void CFileView::OnRButtonDown(UINT nFlags, CPoint point)
{
	INT Index = ItemAtPosition(point);
	if (Index!=-1)
	{
		if (!(nFlags & (MK_SHIFT | MK_CONTROL)) || (!m_AllowMultiSelect))
			if (!IsSelected(Index))
			{
				m_FocusItem = Index;

				for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
					SelectItem(a, a==Index, TRUE);

				ChangedItems();
			}
			else
				if (m_FocusItem!=Index)
				{
					InvalidateItem(m_FocusItem);
					m_FocusItem = Index;
					InvalidateItem(m_FocusItem);
				}
	}
	else
	{
		if (GetFocus()!=this)
			SetFocus();
	}

	m_BeginDragDrop = FALSE;
}

void CFileView::OnRButtonUp(UINT nFlags, CPoint point)
{
	INT Index = ItemAtPosition(point);
	if (Index!=-1)
	{
		if (GetFocus()!=this)
			SetFocus();

		if (!IsSelected(Index))
		{
			m_FocusItem = Index;

			for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
				SelectItem(a, a==Index, TRUE);

			ChangedItems();
		}
	}
	else
	{
		if (!(nFlags & MK_CONTROL) || (!m_AllowMultiSelect))
			OnSelectNone();
	}

	GetParent()->UpdateWindow();
	CWnd::OnRButtonUp(nFlags, point);
}

void CFileView::OnSetFocus(CWnd* /*pOldWnd*/)
{
	Invalidate();
}

void CFileView::OnKillFocus(CWnd* /*pNewWnd*/)
{
	Invalidate();
}

BOOL CFileView::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*Message*/)
{
	SetCursor(theApp.LoadStandardCursor(p_CookedFiles ? IDC_ARROW : IDC_WAIT));
	return TRUE;
}

void CFileView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	INT Index = -1;
	if ((point.x<0) || (point.y<0))
	{
		Index = GetSelectedItem();

		if (Index!=-1)
		{
			FVItemData* d = GetItemData(Index);
			point.x = d->Rect.left-m_HScrollPos;
			point.y = d->Rect.bottom-m_VScrollPos+(INT)m_HeaderHeight+1;
			ClientToScreen(&point);
		}
	}
	else
	{
		CPoint ptClient(point);
		ScreenToClient(&ptClient);

		Index = ItemAtPosition(ptClient);
	}

	if (Index==-1)
	{
		GetParent()->SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(point.x, point.y));
	}
	else
	{
		ZeroMemory(&m_SendToItems, sizeof(m_SendToItems));

		UINT idCmd = 0;

		CMenu* pMenu = GetItemContextMenu(Index);
		if (pMenu)
		{
			CMenu* pPopup = pMenu->GetSubMenu(0);
			ASSERT_VALID(pPopup);

			idCmd = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, GetOwner(), NULL);
			delete pMenu;
		}

		for (UINT a=0; a<256; a++)
			if (m_SendToItems[a].hBitmap)
				DeleteObject(m_SendToItems[a].hBitmap);

		if (idCmd)
			if (idCmd<0xFF00)
			{
				GetOwner()->SendMessage(WM_COMMAND, (WPARAM)idCmd);
			}
			else
			{
				GetParent()->SendMessage(WM_SENDTO, (WPARAM)&m_SendToItems[idCmd % 0xFF]);
			}
	}
}

void CFileView::OnSelectAll()
{
	if (p_CookedFiles && m_AllowMultiSelect)
	{
		for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
			SelectItem(a, TRUE, TRUE);

		ChangedItems();
		RedrawWindow();
	}
}

void CFileView::OnSelectNone()
{
	if (p_CookedFiles)
	{
		for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
			SelectItem(a, FALSE, TRUE);

		ChangedItems();
		RedrawWindow();
	}
}

void CFileView::OnSelectInvert()
{
	if (p_CookedFiles && m_AllowMultiSelect)
	{
		for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
			SelectItem(a, !IsSelected(a), TRUE);

		ChangedItems();
		RedrawWindow();
	}
}

void CFileView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_SELECTALL:
	case IDM_SELECTINVERT:
		b &= m_AllowMultiSelect;

	case IDM_SELECTNONE:
		b &= p_CookedFiles ? (p_CookedFiles->m_ItemCount!=NULL) : FALSE;
		break;
	}

	pCmdUI->Enable(b);
}

void CFileView::OnDestroyEdit()
{
	DestroyEdit(TRUE);
}
