
// CFileView.cpp: Implementierung der Klasse CFileView
//

#include "stdafx.h"
#include "liquidFOLDERS.h"


BOOL AttributeSortableInView(UINT Attr, UINT ViewMode)
{
	BOOL bSortable = theApp.m_Attributes[Attr].Sortable;

	switch (ViewMode)
	{
	case LFViewCalendar:
	case LFViewTimeline:
		bSortable &= (theApp.m_Attributes[Attr].Type==LFTypeTime);
		break;

	case LFViewGlobe:
		bSortable &= ((Attr==LFAttrLocationIATA) || (theApp.m_Attributes[Attr].Type==LFTypeGeoCoordinates));
		break;
	}

	return bSortable;
}


// CFileView
//

#define GetItemData(Index)     ((FVItemData*)(m_ItemData+Index*m_DataSize))
#define IsSelected(Index)      GetItemData(Index)->Selected
#define ChangedItem(Index)     { InvalidateItem(Index); GetParent()->SendMessage(WM_UPDATESELECTION); }
#define ChangedItems()         { Invalidate(); GetParent()->SendMessage(WM_UPDATESELECTION); }
#define FIRSTSENDTO            0xFF00

CFileView::CFileView(UINT DataSize, BOOL EnableScrolling, BOOL EnableHover, BOOL EnableTooltip, BOOL EnableShiftSelection, BOOL EnableLabelEdit, BOOL EnableTooltipOnVirtual)
	: CFrontstageWnd()
{
	ASSERT(DataSize>=sizeof(FVItemData));

	p_RawFiles = p_CookedFiles = NULL;
	p_Edit = NULL;
	m_ItemData = NULL;
	m_ItemDataAllocated = 0;
	m_FocusItem = m_HotItem = m_SelectionAnchor = m_EditLabel = m_Context = -1;
	m_Context = LFContextAllFiles;
	m_HeaderHeight = m_HScrollMax = m_VScrollMax = m_HScrollPos = m_VScrollPos = 0;
	m_RowHeight = LFGetApp()->m_DefaultFont.GetFontHeight();
	m_ColWidth = HorizontalScrollWidth;
	m_DataSize = DataSize;
	m_Nothing = TRUE;
	m_Hover = m_BeginDragDrop = m_ShowFocusRect = m_AllowMultiSelect = FALSE;
	m_TypingBuffer[0] = L'\0';
	m_TypingTicks = 0;

	m_EnableScrolling = EnableScrolling;
	m_EnableHover = EnableHover;
	m_EnableTooltip = EnableTooltip;
	m_EnableShiftSelection = EnableShiftSelection;
	m_EnableLabelEdit = EnableLabelEdit;
	m_EnableTooltipOnVirtual = EnableTooltipOnVirtual;

	ZeroMemory(&m_Bitmaps, sizeof(m_Bitmaps));
}

CFileView::~CFileView()
{
	DestroyEdit();

	for (UINT a=0; a<2; a++)
		DeleteObject(m_Bitmaps[a].hBitmap);

	if (m_ItemData)
		free(m_ItemData);
}

BOOL CFileView::Create(CWnd* pParentWnd, UINT nID, const CRect& rect, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data, UINT nClassStyle)
{
	CString className = AfxRegisterWndClass(nClassStyle, theApp.LoadStandardCursor(IDC_ARROW));

	if (!CFrontstageWnd::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP, rect, pParentWnd, nID))
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
		theApp.HideTooltip();
		break;
	}

	return CFrontstageWnd::PreTranslateMessage(pMsg);
}

void CFileView::UpdateViewOptions(INT Context, BOOL Force)
{
	DestroyEdit();
	theApp.HideTooltip();

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
	theApp.HideTooltip();

	void* pVictim = m_ItemData;
	SIZE_T VictimAllocated = m_ItemDataAllocated;

	m_Nothing = TRUE;

	if (pCookedFiles)
	{
		m_AllowMultiSelect = (pCookedFiles->m_Context!=LFContextStores);

		SIZE_T Size = pCookedFiles->m_ItemCount*m_DataSize;
		m_ItemData = (BYTE*)malloc(Size);
		m_ItemDataAllocated = pCookedFiles->m_ItemCount;
		ZeroMemory(m_ItemData, Size);

		if (VictimAllocated)
		{
			INT RetainSelection = Data ? Data->FocusItem : -1;

			for (UINT a=0; a<min(VictimAllocated, pCookedFiles->m_ItemCount); a++)
			{
				FVItemData* pData = GetItemData(a);

				BYTE* pVictimData = (BYTE*)pVictim+(((BYTE*)pData)-((BYTE*)m_ItemData));

				if ((m_AllowMultiSelect) || ((INT)a==RetainSelection))
					pData->Selected = ((FVItemData*)pVictimData)->Selected;
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
	SetSearchResult(pRawFiles, pCookedFiles, Data);

	free(pVictim);

	if (p_CookedFiles)
	{
		BOOL NeedNewFocusItem = (m_FocusItem>=0) ? !GetItemData(m_FocusItem)->Valid : TRUE;

		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			FVItemData* pData = GetItemData(a);
			pData->Selected &= pData->Valid;
			m_Nothing &= !pData->Valid;

			if (NeedNewFocusItem && pData->Valid)
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

INT CFileView::GetFocusItem() const
{
	if (p_CookedFiles)
	{
		FVItemData* pData = GetItemData(m_FocusItem);
		return pData->Valid ? m_FocusItem : -1;
	}

	return -1;
}

INT CFileView::GetSelectedItem() const
{
	if (p_CookedFiles)
		if (p_CookedFiles->m_ItemCount)
		{
			FVItemData* pData = GetItemData(m_FocusItem);
			return (pData->Selected && pData->Valid) ? m_FocusItem : -1;
		}

	return -1;
}

INT CFileView::GetNextSelectedItem(INT Index) const
{
	if (p_CookedFiles)
	{
		ASSERT(Index>=-1);

		while (++Index<(INT)p_CookedFiles->m_ItemCount)
		{
			FVItemData* pData = GetItemData(Index);
			if (pData->Selected && pData->Valid)
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

		FVItemData* pData = GetItemData(Index);
		if (pData->Valid)
		{
			pData->Selected = Select;

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
	GetClientRect(rect);

	RECT rectItem = GetItemRect(Index);

	// Vertikal
	INT nInc = 0;

	if (rectItem.bottom>rect.Height())
		nInc = rectItem.bottom-rect.Height();

	if (rectItem.top<nInc+(INT)m_HeaderHeight)
		nInc = rectItem.top-(INT)m_HeaderHeight;

	nInc = max(-m_VScrollPos, min(nInc, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindow(0, -nInc);
		SetScrollPos(SB_VERT, m_VScrollPos);
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
			SetScrollPos(SB_HORZ, m_HScrollPos);
		}
	}
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

RECT CFileView::GetItemRect(INT Index) const
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

RECT CFileView::GetLabelRect(INT Index) const
{
	return GetItemRect(Index);
}

INT CFileView::ItemAtPosition(CPoint point) const
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
			InflateRect(&rect, GetItemData(Index)->RectInflate, GetItemData(Index)->RectInflate);
			InvalidateRect(&rect);
		}
}

CMenu* CFileView::GetViewContextMenu()
{
	return NULL;
}

void CFileView::AppendSendToItem(CMenu* pMenu, UINT nID, LPCWSTR lpszNewItem, HICON hIcon, INT cx, INT cy)
{
	const UINT Index = nID % 0xFF;

	pMenu->AppendMenu(MF_STRING, nID, lpszNewItem);

	if (hIcon)
	{
		m_SendToItems[Index].hIcon = hIcon;
		m_SendToItems[Index].cx = cx;
		m_SendToItems[Index].cy = cy;

		MENUITEMINFO mii;
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_BITMAP;
		mii.hbmpItem = HBMMENU_CALLBACK;

		SetMenuItemInfo(*pMenu, pMenu->GetMenuItemCount()-1, TRUE, &mii);
	}
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
	UINT nID = FIRSTSENDTO;
	if (LFGetDefaultStore()==LFOk)
	{
		CString tmpStr((LPCSTR)IDS_DEFAULTSTORE);
		AppendSendToItem(pMenu, nID, tmpStr, (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_APPLICATION), IMAGE_ICON, cx, cy, LR_SHARED), cx, cy);
		Added = TRUE;

		const UINT Index = (nID++) & 0xFF;
		m_SendToItems[Index].IsStore = TRUE;
		strcpy_s(m_SendToItems[Index].StoreID, LFKeySize, "");
	}

	if (LFGetStoreCount())
	{
		CString tmpStr((LPCSTR)IDS_CONTEXTMENU_CHOOSESTORE);
		AppendSendToItem(pMenu, nID, tmpStr, NULL, cx, cy);
		Added = TRUE;

		const UINT Index = (nID++) & 0xFF;
		m_SendToItems[Index].IsStore = TRUE;
		strcpy_s(m_SendToItems[Index].StoreID, LFKeySize, "CHOOSE");
	}

	// SendTo shortcuts
	nID = FIRSTSENDTO+0x40;
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
						AppendSendToItem(pMenu, nID, Name, theApp.m_SystemImageListSmall.ExtractIcon(sfi.iIcon), cx, cy);
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

			AppendSendToItem(pMenu, nID, sfi.szDisplayName, sfi.hIcon, cx, cy);

			const UINT Index = (nID++) & 0xFF;
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

	LFItemDescriptor* Item = (*p_CookedFiles)[Index];
	switch (Item->Type & LFTypeMask)
	{
	case LFTypeStore:
		pMenu->LoadMenu(IDM_STORE);
		pMenu->GetSubMenu(0)->InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);

		break;

	case LFTypeFolder:
		if ((Item->FirstAggregate!=-1) && (Item->LastAggregate!=-1))
			pMenu->LoadMenu(IDM_FOLDER);

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

void CFileView::GetPersistentData(FVPersistentData& Data) const
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
		LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];
		if (((pItemDescriptor->Type & LFTypeMask)==LFTypeStore) ||
			((pItemDescriptor->Type & (LFTypeNotMounted | LFTypeMask))==LFTypeFile))
		{
			m_EditLabel = Index;
			InvalidateItem(Index);
			EnsureVisible(Index);

			const INT FontHeight = theApp.m_DefaultFont.GetFontHeight();

			CRect rect(GetLabelRect(Index));
			if (rect.Height()>FontHeight+4)
			{
				rect.top += (rect.Height()-FontHeight-4)/2;
				rect.bottom = rect.top+FontHeight+4;
			}

			p_Edit = new CEdit();
			p_Edit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ES_AUTOHSCROLL, rect, this, 2);
			p_Edit->SetWindowText(pItemDescriptor->CoreAttributes.FileName);
			p_Edit->SetFont(&theApp.m_DefaultFont);
			p_Edit->SetFocus();
			p_Edit->SetSel(0, -1);
		}
	}
}

void CFileView::DrawItemBackground(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed, BOOL Cached)
{
	if (Cached && Themed && IsSelected(Index))
	{
		CDC MemDC;
		MemDC.CreateCompatibleDC(&dc);

		HBITMAP hOldBitmap;

		const INT Width = rectItem->right-rectItem->left;
		const INT Height = rectItem->bottom-rectItem->top;

		if ((m_Bitmaps[BM_SELECTED].Width!=Width) || (m_Bitmaps[BM_SELECTED].Height!=Height))
		{
			DeleteObject(m_Bitmaps[BM_SELECTED].hBitmap);

			m_Bitmaps[BM_SELECTED].Width = Width;
			m_Bitmaps[BM_SELECTED].Height = Height;
			m_Bitmaps[BM_SELECTED].hBitmap = CreateCompatibleBitmap(dc, Width, Height);

			hOldBitmap = (HBITMAP)MemDC.SelectObject(m_Bitmaps[BM_SELECTED].hBitmap);

			MemDC.FillSolidRect(0, 0, Width, Height, 0xFFFFFF);
			DrawListItemBackground(MemDC, CRect(0, 0, Width, Height), Themed, GetFocus()==this, m_HotItem==Index, m_FocusItem==Index, IsSelected(Index));
		}
		else
		{
			hOldBitmap = (HBITMAP)MemDC.SelectObject(m_Bitmaps[BM_SELECTED].hBitmap);
		}

		dc.BitBlt(rectItem->left, rectItem->top, Width, Height, &MemDC, 0, 0, SRCCOPY);

		MemDC.SelectObject(hOldBitmap);

		dc.SetTextColor(((*p_CookedFiles)[Index]->CoreAttributes.Flags & LFFlagMissing) ? 0x0000FF : 0xFFFFFF);
	}
	else
	{
		DrawListItemBackground(dc, rectItem, Themed, GetFocus()==this,
			m_HotItem==Index, m_FocusItem==Index, IsSelected(Index),
			((*p_CookedFiles)[Index]->CoreAttributes.Flags & LFFlagMissing) ? 0x0000FF : (COLORREF)-1,
			m_ShowFocusRect);
	}
}

void CFileView::DrawItemForeground(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed, BOOL Cached)
{
	if (((m_HotItem!=Index) && !IsSelected(Index)) || !Themed)
		return;

	if (Cached)
	{
		CDC MemDC;
		MemDC.CreateCompatibleDC(&dc);

		HBITMAP hOldBitmap;

		const INT Width = rectItem->right-rectItem->left;
		const INT Height = rectItem->bottom-rectItem->top;

		if ((m_Bitmaps[BM_REFLECTION].Width!=Width) || (m_Bitmaps[BM_REFLECTION].Height!=Height))
		{
			DeleteObject(m_Bitmaps[BM_REFLECTION].hBitmap);

			m_Bitmaps[BM_REFLECTION].Width = Width;
			m_Bitmaps[BM_REFLECTION].Height = Height;
			m_Bitmaps[BM_REFLECTION].hBitmap = CreateTransparentBitmap(Width, Height);

			hOldBitmap = (HBITMAP)MemDC.SelectObject(m_Bitmaps[BM_REFLECTION].hBitmap);

			DrawListItemForeground(MemDC, CRect(0, 0, Width, Height), Themed, GetFocus()==this, m_HotItem==Index, m_FocusItem==Index, IsSelected(Index));
		}
		else
		{
			hOldBitmap = (HBITMAP)MemDC.SelectObject(m_Bitmaps[BM_REFLECTION].hBitmap);
		}

		AlphaBlend(dc, rectItem->left, rectItem->top, Width, Height, MemDC, 0, 0, Width, Height, BF);

		MemDC.SelectObject(hOldBitmap);
	}
	else
	{
		DrawListItemForeground(dc, rectItem, Themed, GetFocus()==this, m_HotItem==Index, m_FocusItem==Index, IsSelected(Index));
	}
}

void CFileView::ResetScrollbars()
{
	if (m_EnableScrolling)
	{
		ScrollWindow(m_HScrollPos, m_VScrollPos);

		SetScrollPos(SB_VERT, m_VScrollPos=0);
		SetScrollPos(SB_HORZ, m_HScrollPos=0);
	}
}

void CFileView::AdjustScrollbars()
{
	if (!m_EnableScrolling)
		return;

	// Dimensions
	CRect rect;
	GetWindowRect(rect);

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

	// Set vertical bars
	m_VScrollMax = max(0, m_ScrollHeight-rect.Height()+(INT)m_HeaderHeight);
	m_VScrollPos = min(m_VScrollPos, m_VScrollMax);

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rect.Height()-m_HeaderHeight;
	si.nMax = m_ScrollHeight-1;
	si.nPos = m_VScrollPos;
	SetScrollInfo(SB_VERT, &si);

	// Set horizontal bars
	m_HScrollMax = max(0, m_ScrollWidth-rect.Width());
	m_HScrollPos = min(m_HScrollPos, m_HScrollMax);

	si.nPage = rect.Width();
	si.nMax = m_ScrollWidth-1;
	si.nPos = m_HScrollPos;
	SetScrollInfo(SB_HORZ, &si);
}

CString CFileView::GetLabel(LFItemDescriptor* pItemDescriptor) const
{
	CString Label = pItemDescriptor->CoreAttributes.FileName;

	if ((pItemDescriptor->Type & LFTypeMask)==LFTypeFile)
		if (((!m_HideFileExt) || (pItemDescriptor->CoreAttributes.FileName[0]==L'\0')) && (pItemDescriptor->CoreAttributes.FileFormat[0]!='\0') && (strcmp(pItemDescriptor->CoreAttributes.FileFormat, "filter")!=0))
		{
			Label += _T(".");
			Label += pItemDescriptor->CoreAttributes.FileFormat;
		}

	return Label;
}

BOOL CFileView::BeginDragDrop()
{
	m_BeginDragDrop = FALSE;

	GetParent()->SendMessage(WM_BEGINDRAGDROP);

	return TRUE;
}

CString CFileView::GetHint(LFItemDescriptor* pItemDescriptor, WCHAR* FormatName) const
{
	WCHAR tmpStr[256];
	CString Hint;

	switch (pItemDescriptor->Type & LFTypeMask)
	{
	case LFTypeStore:
		GetHintForStore(Hint, pItemDescriptor);
		break;

	case LFTypeFolder:
		AppendAttribute(Hint, pItemDescriptor, LFAttrComments);
		AppendAttribute(Hint, pItemDescriptor, LFAttrDescription);

		if ((pItemDescriptor->Type & LFTypeSourceMask)>LFTypeSourceInternal)
			AppendAttribute(Hint, LFAttrComments, theApp.m_SourceNames[pItemDescriptor->Type & LFTypeSourceMask][1]);

		break;

	case LFTypeFile:
		AppendAttribute(Hint, pItemDescriptor, LFAttrComments);
		AppendAttribute(Hint, LFAttrFileFormat, FormatName);

		if ((pItemDescriptor->Type & LFTypeSourceMask)>LFTypeSourceInternal)
		{
			tmpStr[0] = L' ';
			wcscpy_s(&tmpStr[1], 255, theApp.m_SourceNames[pItemDescriptor->Type & LFTypeSourceMask][1]);
			tmpStr[1] = (WCHAR)towlower(tmpStr[1]);

			Hint += tmpStr;
		}

		AppendAttribute(Hint, pItemDescriptor, LFAttrArtist);
		AppendAttribute(Hint, pItemDescriptor, LFAttrTitle);
		AppendAttribute(Hint, pItemDescriptor, LFAttrAlbum);
		AppendAttribute(Hint, pItemDescriptor, LFAttrRecordingTime);
		AppendAttribute(Hint, pItemDescriptor, LFAttrRoll);
		AppendAttribute(Hint, pItemDescriptor, LFAttrDuration);
		AppendAttribute(Hint, pItemDescriptor, LFAttrHashtags);
		AppendAttribute(Hint, pItemDescriptor, LFAttrPages);

		if (pItemDescriptor->AttributeValues[LFAttrDimension])
		{
			LFAttributeToString(pItemDescriptor, LFAttrDimension, tmpStr, 256);

			WCHAR Resolution[256];
			swprintf_s(Resolution, 256, L"%s (%u×%u)", tmpStr, (UINT)*((UINT*)pItemDescriptor->AttributeValues[LFAttrWidth]), (UINT)*((UINT*)pItemDescriptor->AttributeValues[LFAttrHeight]));

			AppendAttribute(Hint, LFAttrDimension, Resolution);
		}

		AppendAttribute(Hint, pItemDescriptor, LFAttrEquipment);
		AppendAttribute(Hint, pItemDescriptor, LFAttrBitrate);
		AppendAttribute(Hint, pItemDescriptor, LFAttrCreationTime);
		AppendAttribute(Hint, pItemDescriptor, LFAttrFileTime);
		AppendAttribute(Hint, pItemDescriptor, LFAttrFileSize);

		break;
	}

	return Hint;
}

void CFileView::DestroyEdit(BOOL Accept)
{
	if (p_Edit)
	{
		INT Item = m_EditLabel;

		CString Name;
		p_Edit->GetWindowText(Name);

		// Destroying the edit control will release its focus, in turn causing DestroyEdit()
		// to be called. To prevent an infinite recursion, we set p_Edit to NULL first.
		CEdit* pVictim = p_Edit;
		p_Edit = NULL;

		pVictim->DestroyWindow();
		delete pVictim;

		if ((Accept) && (!Name.IsEmpty()) && (Item!=-1))
			GetParent()->SendMessage(WM_RENAMEITEM, (WPARAM)Item, (LPARAM)(LPCWSTR)Name);
	}

	m_EditLabel = -1;
	m_TypingBuffer[0] = L'\0';
}

void CFileView::ScrollWindow(INT dx, INT dy, LPCRECT /*lpRect*/, LPCRECT /*lpClipRect*/)
{
	ASSERT(m_EnableScrolling);

	CRect rect;
	GetClientRect(rect);

	rect.top = m_HeaderHeight;

	if (IsCtrlThemed() && (dy!=0))
	{
		rect.bottom -= BACKSTAGERADIUS;

		ScrollWindowEx(dx, dy, rect, rect, NULL, NULL, SW_INVALIDATE);
		RedrawWindow(CRect(rect.left, rect.bottom, rect.right, rect.bottom+BACKSTAGERADIUS), NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else
	{
		ScrollWindowEx(dx, dy, rect, rect, NULL, NULL, SW_INVALIDATE);
	}
}


BEGIN_MESSAGE_MAP(CFileView, CFrontstageWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEHWHEEL()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SETCURSOR()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
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
	if (CFrontstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	if (m_EnableScrolling)
		ResetScrollbars();

	return 0;
}

BOOL CFileView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CFileView::OnSize(UINT nType, INT cx, INT cy)
{
	CFrontstageWnd::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CFileView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CRect rect;
	GetClientRect(rect);

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
		SetScrollPos(SB_VERT, m_VScrollPos);

		if (p_Edit)
		{
			CRect rect;
			p_Edit->GetWindowRect(rect);
			ScreenToClient(rect);

			rect.OffsetRect(0, -nInc);
			p_Edit->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	CFrontstageWnd::OnVScroll(nSBCode, nPos, pScrollBar);
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
		nInc = -m_ColWidth;
		break;

	case SB_PAGEDOWN:
	case SB_LINEDOWN:
		nInc = m_ColWidth;
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
		SetScrollPos(SB_HORZ, m_HScrollPos);

		UpdateWindow();
	}

	CFrontstageWnd::OnHScroll(nSBCode, nPos, pScrollBar);
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
			tme.dwHoverTime = HOVERTIME;
			tme.hwndTrack = m_hWnd;
			TrackMouseEvent(&tme);
		}
		else
			if ((LFGetApp()->IsTooltipVisible()) && (Index!=m_HotItem))
				theApp.HideTooltip();

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
	InvalidateItem(m_HotItem);
	theApp.HideTooltip();

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
				theApp.HideTooltip();
				EditLabel(m_EditLabel);
			}
			else
				if (!LFGetApp()->IsTooltipVisible() && m_EnableTooltip)
				{
					FormatData fd;
					HICON hIcon = NULL;
					HBITMAP hBitmap = NULL;

					LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[m_HotItem];

					switch (pItemDescriptor->Type & LFTypeMask)
					{
					case LFTypeFile:
						if ((theApp.m_Views[m_Context].Mode!=LFViewContent) && (theApp.m_Views[m_Context].Mode!=LFViewPreview) && (theApp.m_Views[m_Context].Mode!=LFViewTimeline))
						{
							CDC* pDC = GetDC();
							hBitmap = theApp.m_ThumbnailCache.GetThumbnailBitmap(pItemDescriptor, pDC);
							ReleaseDC(pDC);
						}

						theApp.m_FileFormats.Lookup(pItemDescriptor->CoreAttributes.FileFormat, fd);

						break;

					case LFTypeFolder:
						if (!m_EnableTooltipOnVirtual)
							goto Leave;

					default:
						fd.FormatName[0] = L'\0';
						fd.SysIconIndex = -1;
					}

					if (!hIcon && !hBitmap)
						hIcon = (fd.SysIconIndex>=0) ? theApp.m_SystemImageListExtraLarge.ExtractIcon(fd.SysIconIndex) : theApp.m_CoreImageListExtraLarge.ExtractIcon(pItemDescriptor->IconID-1);

					theApp.ShowTooltip(this, point, GetLabel(pItemDescriptor), GetHint(pItemDescriptor, fd.FormatName), hIcon, hBitmap);
				}
	}
	else
	{
Leave:
		theApp.HideTooltip();
	}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = HOVERTIME;
	tme.hwndTrack = m_hWnd;
	TrackMouseEvent(&tme);
}

BOOL CFileView::OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(rect);

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
		SetScrollPos(SB_VERT, m_VScrollPos);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}

	return TRUE;
}

void CFileView::OnMouseHWheel(UINT nFlags, SHORT zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(rect);

	if (!rect.PtInRect(pt))
		return;

	INT nInc = max(-m_HScrollPos, min(zDelta*64/WHEEL_DELTA, m_HScrollMax-m_HScrollPos));
	if (nInc)
	{
		m_HScrollPos += nInc;
		ScrollWindow(-nInc, 0);
		SetScrollPos(SB_HORZ, m_HScrollPos);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}
}

void CFileView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((GetKeyState(VK_CONTROL)>=0) && (nChar>=32) && (m_EditLabel==-1))
	{
		if (p_CookedFiles)
			if (p_CookedFiles->m_ItemCount>1)
			{
				// Reset typing buffer?
				DWORD Ticks = GetTickCount();
				if (Ticks-m_TypingTicks>=1000)
					m_TypingBuffer[0] = L'\0';

				m_TypingTicks = Ticks;

				// Concatenate typing buffer
				WCHAR TypingBuffer[256];
				wcscpy_s(TypingBuffer, 256, m_TypingBuffer);

				WCHAR Letter[2] = { (WCHAR)nChar, L'\0' };
				wcscat_s(TypingBuffer, 256, Letter);

				INT FocusItem = m_FocusItem;

				for (UINT a=0; a<p_CookedFiles->m_ItemCount-1; a++, FocusItem++)
					if (_wcsnicmp(TypingBuffer, (*p_CookedFiles)[FocusItem]->CoreAttributes.FileName, wcslen(TypingBuffer))==0)
					{
						wcscpy_s(m_TypingBuffer, 256, TypingBuffer);
						SetFocusItem(FocusItem, FALSE);

						return;
					}
			}

		LFGetApp()->PlayDefaultSound();
	}

	CFrontstageWnd::OnChar(nChar, nRepCnt, nFlags);
}

void CFileView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
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
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0) && (m_FocusItem!=-1))
			if (IsSelected(m_FocusItem))
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
		CFrontstageWnd::OnKeyDown(nChar, nRepCnt, nFlags);
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

	if (GetFocus()!=this)
		SetFocus();
}

void CFileView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (!(nFlags & MK_CONTROL) || (!m_AllowMultiSelect))
		if (ItemAtPosition(point)==-1)
			OnSelectNone();

	if (GetFocus()!=this)
		SetFocus();

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

	CFrontstageWnd::OnRButtonUp(nFlags, point);
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

void CFileView::OnMeasureItem(INT nIDCtl, LPMEASUREITEMSTRUCT lpmis)
{
	if ((lpmis==NULL) || (lpmis->CtlType!=ODT_MENU))
	{
		CFrontstageWnd::OnMeasureItem(nIDCtl, lpmis);

		return;
	}

	if ((lpmis->itemID>=FIRSTSENDTO) && (lpmis->itemID<=FIRSTSENDTO+0xFF))
	{
		lpmis->itemWidth = GetSystemMetrics(SM_CXSMICON)*5/4;
		lpmis->itemHeight = GetSystemMetrics(SM_CYSMICON);
	}
}

void CFileView::OnDrawItem(INT nIDCtl, LPDRAWITEMSTRUCT lpdis)
{
	if ((lpdis==NULL) || (lpdis->CtlType!=ODT_MENU))
	{
		CFrontstageWnd::OnDrawItem(nIDCtl, lpdis);

		return;
	}

	if ((lpdis->itemID>=FIRSTSENDTO) && (lpdis->itemID<=FIRSTSENDTO+0xFF))
	{
		const UINT Index = lpdis->itemID % 0xFF;

		DrawIconEx(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, m_SendToItems[Index].hIcon, m_SendToItems[Index].cx, m_SendToItems[Index].cy, 0, NULL, DI_NORMAL);
	}
}

void CFileView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	INT Index = -1;
	if ((point.x<0) || (point.y<0))
	{
		Index = GetSelectedItem();

		if (Index!=-1)
		{
			FVItemData* pData = GetItemData(Index);

			point.x = pData->Rect.left-m_HScrollPos;
			point.y = pData->Rect.bottom-m_VScrollPos+(INT)m_HeaderHeight+1;
			ClientToScreen(&point);
		}
	}
	else
	{
		CPoint pt(point);
		ScreenToClient(&pt);

		Index = ItemAtPosition(pt);
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
			DestroyIcon(m_SendToItems[a].hIcon);

		if (idCmd)
			if (idCmd<FIRSTSENDTO)
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
	BOOL bEnable = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_SELECTALL:
	case IDM_SELECTINVERT:
		bEnable &= m_AllowMultiSelect;

	case IDM_SELECTNONE:
		bEnable &= p_CookedFiles ? (p_CookedFiles->m_ItemCount!=NULL) : FALSE;
		break;
	}

	pCmdUI->Enable(bEnable);
}

void CFileView::OnDestroyEdit()
{
	DestroyEdit(TRUE);
}
